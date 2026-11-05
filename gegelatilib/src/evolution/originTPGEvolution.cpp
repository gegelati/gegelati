#include "evolution/originTPGEvolution.h"

#include <stdexcept>
#include <map>
#include <queue>
#include "dimensions/dataValueGenerator.h"
#include "dimensions/individualArchiveValueGenerator.h"
#include "dimensions/individualValueGenerator.h"
#include "graphBased/nodeGenerator.h"

#include "evaluation/predictionProblem.h"


#include "evolution/steps/evaluateStep.h"
#include "evolution/steps/initPopulationStep.h"
#include "evolution/steps/lambdaStep.h"
#include "evolution/steps/tpgSteps.h"




#include "metrics/inputMetric.h"
#include "mutation/pointMutator.h"

namespace {
    // Mutator of the vertices, the default one if none is set.
    std::shared_ptr<Mutation::NodeTransferMutation> usedTransfer(const Evolution::OriginTPGEvolution& evolution)
    {
        return (evolution.nodeTransfer != nullptr) ? evolution.nodeTransfer : std::make_shared<Mutation::NodeTransferMutation>(0.1, 0.7, 0.7);
    }

    // Mutator of the members, the default one if none is set.
    std::shared_ptr<const Mutation::Mutator> usedMemberMutator(const Evolution::OriginTPGEvolution& evolution)
    {
        return (evolution.memberMutation != nullptr) ? evolution.memberMutation : std::make_shared<Mutation::PointMutator>(0.2, 0.5, 0.5);
    }
}

std::shared_ptr<const Representations::TPG> Evolution::OriginTPGEvolution::checkTPGSettings() const
{
    this->checkSettings();
    const auto tpgRepresentation = std::dynamic_pointer_cast<const Representations::TPG>(this->representation);
    if(tpgRepresentation == nullptr) {
        throw std::runtime_error("Evolution::OriginTPGEvolution: the representation must be a TPG");
    }
    if(this->memberRepresentation == nullptr) {
        throw std::runtime_error("Evolution::OriginTPGEvolution: the representation of the members must be set");
    }
    if(!Dimensions::DimensionFlow::acceptsRequirements(tpgRepresentation->getDimensionFlow().getInputDimensions(),
                                                       this->memberRepresentation->getDimensionFlow().getInputDimensions())) {
        throw std::runtime_error("Evolution::OriginTPGEvolution: input requirements not compatible between the TPG and the members");
    }
    if(!this->memberRepresentation->getDimensionFlow().getOutputDimension().isCompatibleWith(Dimensions::Requirement(Data::DataType::type<double>()))) {
        throw std::runtime_error("Evolution::OriginTPGEvolution: the members must output a double");
    }
    return tpgRepresentation;
}

std::shared_ptr<Evolution::Module> Evolution::OriginTPGEvolution::build() const
{
    const auto tpgRepresentation = this->checkTPGSettings();

    // Step 1: from the settings, create what the modules need.
    const std::shared_ptr<Evaluation::Evaluator> evaluator = this->makeEvaluator();
    const std::shared_ptr<Mutation::NodeTransferMutation> transfer = usedTransfer(*this);
    const std::shared_ptr<const Mutation::Mutator> memberMutator = usedMemberMutator(*this);

    // The archive samples the inputs measured by the training evaluations. A fixed archive only samples during the initialization.
    const bool withArchive = this->archiveSize > 0;
    const auto [initMetrics, fitnessMetricHash] = this->makeMetrics(*this->problem);
    const auto [generationMetrics, generationMetricHash] = this->makeMetrics(*this->problem);
    if(withArchive) {
        initMetrics->addRequiredMetric(std::make_unique<Metrics::InputMetric>());
        if(!this->isArchiveFixed) {
            generationMetrics->addRequiredMetric(std::make_unique<Metrics::InputMetric>());
        }
    }

    // The generator of the vertices is a data of the evolution, built again by a step: the members are created with the member mutator and
    // must answer differently from existing ones on the archive if there is one, and nodes may point to the current vertices.
    const std::set<std::string> archiveName = withArchive ? std::set<std::string>{this->tpgNames.archive} : std::set<std::string>{};
    auto makeGeneratorStep = [&](bool fromVertices) {
        std::set<std::string> required = archiveName;
        if(fromVertices) {
            required.insert(this->names.population);
        }
        return std::make_shared<LambdaStep>(this->tpgNames.updateGeneratorStep,
            [archiveName = this->tpgNames.archive, populationName = this->names.population, generatorName = this->tpgNames.generator, tpgRepresentation,
             memberRepresentation = this->memberRepresentation, memberMutator, fromVertices](EvolutionData& data) {
                // The member of a node is created with the member mutator, and must answer differently from existing ones on the archive if there is one.
                GraphBased::NodeGenerator bidNodes;
                if(data.has(archiveName)) {
                    bidNodes.addGenerator(Dimensions::IndividualValueArchiveGenerator(*data.get<Evaluation::DataSet>(archiveName), memberRepresentation, *memberMutator));
                } else {
                    bidNodes.addGenerator(Dimensions::IndividualValueGenerator(memberRepresentation, *memberMutator));
                }

                // The destination is an action or, sometimes, one of the current vertices.
                const std::shared_ptr<std::vector<std::shared_ptr<Individual>>> population =
                    fromVertices ? data.get<std::vector<std::shared_ptr<Individual>>>(populationName) : nullptr;
                const std::vector<std::shared_ptr<const Individual>> vertices =
                    fromVertices ? getAllVertices(std::vector<std::shared_ptr<const Individual>>(population->begin(), population->end())) : std::vector<std::shared_ptr<const Individual>>{};
                Dimensions::NumericUniformGenerator<size_t> actionGenerator(0, tpgRepresentation->getNbActions() - 1);
                if(vertices.empty()) {
                    bidNodes.addGenerator(actionGenerator);
                } else {
                    Dimensions::MultiGenerator destination;
                    destination.addGenerator(actionGenerator, 0.995);
                    destination.addGenerator(Dimensions::ListUniformGenerator<std::shared_ptr<const Individual>>(vertices), 0.005);
                    bidNodes.addGenerator(destination);
                }

                // New vertices start with the minimum number of nodes.
                const std::pair<size_t, size_t> range = tpgRepresentation->getGenotypeConstraint().getRangeAt(0);
                data.set(generatorName, std::make_shared<GraphBased::GenotypeGenerator>(bidNodes, range, std::make_pair(range.first, range.first)));
            },
            required, std::set<std::string>{this->tpgNames.generator});
    };

    // Step 2: build the modules. The initialization creates the archive and the vertices, then evaluates them...
    auto init = std::make_shared<Module>(this->names.initialization);
    if(withArchive) {
        init->add(std::make_shared<CreateArchiveStep>(this->tpgNames.archive, tpgRepresentation->getDimensionFlow().getInputDimensions(), this->tpgNames.createArchiveStep));
    }
    init->add(makeGeneratorStep(false));
    init->add(std::make_shared<InitPopulationStep>(this->names.population, this->representation, this->sizePopulation, transfer, this->tpgNames.generator, this->names.initPopulationStep));
    init->add(std::make_shared<EvaluateStep>(std::vector<std::string>{this->names.population}, this->names.trainingMetrics, this->problem,
                                             evaluator, initMetrics, this->nbTrainingIterations, Evaluation::Mode::TRAINING, this->names.initialEvaluationStep));
    this->addFitnessSteps(*init, fitnessMetricHash);
    if(this->protectAggregated) {
        init->add(std::make_shared<ProtectAggregatedStep>(this->names.population, this->tpgNames.protectedVertices, this->tpgNames.aggregators, this->tpgNames.protectAggregatedStep));
    }
    if(withArchive) {
        init->add(std::make_shared<UpdateArchiveStep>(this->names.population, this->names.trainingMetrics, this->tpgNames.archive, this->archiveSize, this->isArchiveFixed, this->tpgNames.updateArchiveStep));
    }

    // ... then a generation, where nodes transferred by the mutation of the offspring are the ones of the current population.
    auto generation = std::make_shared<Module>(this->names.evolution);
    this->addReproductionSteps(*generation);
    generation->add(makeGeneratorStep(true));
    generation->add(std::make_shared<MutateVerticesStep>(this->names.offspring, this->names.population, this->tpgNames.generator, transfer, this->nbThreads, this->names.mutationStep));
    if(const auto predictionProblem = std::dynamic_pointer_cast<Evaluation::PredictionProblem>(this->problem)) {
        generation->add(std::make_shared<CacheMemberOutputsStep>(std::vector<std::string>{this->names.population, this->names.offspring},
                                                                 this->tpgNames.memberOutputCache, predictionProblem, evaluator, this->nbTrainingIterations,
                                                                 this->tpgNames.cacheMemberOutputsStep));
    }
    generation->add(std::make_shared<EvaluateStep>(std::vector<std::string>{this->names.population, this->names.offspring},
                                                   this->names.trainingMetrics, this->problem, evaluator, generationMetrics, this->nbTrainingIterations,
                                                   Evaluation::Mode::TRAINING, this->names.trainingEvaluationStep));
    this->addFitnessSteps(*generation, generationMetricHash);
    this->addSurvivalStep(*generation);
    if(this->protectAggregated) {
        generation->add(std::make_shared<ProtectAggregatedStep>(this->names.population, this->tpgNames.protectedVertices, this->tpgNames.aggregators, this->tpgNames.protectAggregatedStep));
    }
    if(withArchive) {
        generation->add(std::make_shared<UpdateArchiveStep>(this->names.population, this->names.trainingMetrics, this->tpgNames.archive, this->archiveSize, this->isArchiveFixed, this->tpgNames.updateArchiveStep));
    }
    this->addValidationSteps(*generation, evaluator);
    this->addLogStep(*generation);

    generation->setInitialization(init);
    return generation;
}

std::vector<Evolution::LogStep::Column> Evolution::OriginTPGEvolution::logColumns() const
{
    std::vector<LogStep::Column> columns = BasicEvolution::logColumns();
    const std::string roots(this->names.population);
    const auto rootsOf = [roots](const EvolutionData& state) {
        const auto list = state.get<std::vector<std::shared_ptr<Individual>>>(roots);
        return std::vector<std::shared_ptr<const Individual>>(list->begin(), list->end());
    };
    columns.insert(columns.begin() + 1, {"NbVertex", [rootsOf](const EvolutionData& state) { return static_cast<double>(getAllVertices(rootsOf(state)).size()); }, 0, {roots}});
    columns.insert(columns.begin() + 2, {"NbMember", [rootsOf](const EvolutionData& state) { return static_cast<double>(getMembersOf(getAllVertices(rootsOf(state))).size()); }, 0, {roots}});
    return columns;
}

namespace {
    // Index of the member in a node, and of the destination in a node.
    constexpr size_t memberValueIndex = 0;
    constexpr size_t destinationValueIndex = 1;

    // Individuals referenced by the value at a given index of the nodes of the first node group.
    std::vector<std::shared_ptr<const Individual>> getReferencedBy(const Individual& vertex, size_t idxValue)
    {
        std::map<uint64_t, std::shared_ptr<const Individual>> referenced;
        const std::vector<std::reference_wrapper<const GraphBased::GPNode>> nodes = vertex.getGenotype().getNodeGroup(0).getNodes();
        for(const GraphBased::GPNode& node: nodes) {
            if(node.getValue(idxValue).getElementType() == typeid(std::shared_ptr<const Individual>)) {
                const std::shared_ptr<const Individual>& individual = node.getValue(idxValue).getScalar<std::shared_ptr<const Individual>>();
                referenced.emplace(individual->getIndividualID(), individual);
            }
        }

        std::vector<std::shared_ptr<const Individual>> result;
        for(const auto& [id, individual]: referenced) {
            result.push_back(individual);
        }
        return result;
    }
}

std::vector<std::shared_ptr<const Individual>> Evolution::getSubVertices(const Individual& vertex)
{
    return getReferencedBy(vertex, destinationValueIndex);
}

std::vector<std::shared_ptr<const Individual>> Evolution::getAllVertices(const std::vector<std::shared_ptr<const Individual>>& roots)
{
    std::map<uint64_t, std::shared_ptr<const Individual>> vertices;
    std::queue<std::shared_ptr<const Individual>> toControl;
    for(const std::shared_ptr<const Individual>& root: roots) {
        if(vertices.emplace(root->getIndividualID(), root).second) {
            toControl.push(root);
        }
    }

    while(!toControl.empty()) {
        const std::shared_ptr<const Individual> current = toControl.front();
        toControl.pop();
        for(const std::shared_ptr<const Individual>& subVertex: getSubVertices(*current)) {
            if(vertices.emplace(subVertex->getIndividualID(), subVertex).second) {
                toControl.push(subVertex);
            }
        }
    }

    std::vector<std::shared_ptr<const Individual>> result;
    for(const auto& [id, vertex]: vertices) {
        result.push_back(vertex);
    }
    return result;
}

std::vector<std::shared_ptr<const Individual>> Evolution::getMembersOf(const std::vector<std::shared_ptr<const Individual>>& vertices)
{
    std::map<uint64_t, std::shared_ptr<const Individual>> members;
    for(const std::shared_ptr<const Individual>& vertex: vertices) {
        for(const std::shared_ptr<const Individual>& member: getReferencedBy(*vertex, memberValueIndex)) {
            members.emplace(member->getIndividualID(), member);
        }
    }

    std::vector<std::shared_ptr<const Individual>> result;
    for(const auto& [id, member]: members) {
        result.push_back(member);
    }
    return result;
}
