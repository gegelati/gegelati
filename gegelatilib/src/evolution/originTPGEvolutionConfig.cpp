#include "evolution/originTPGEvolutionConfig.h"

#include <stdexcept>
#include <map>
#include <queue>

#include "evaluation/predictionProblem.h"
#include "evolution/steps/assignFitnessStep.h"
#include "evolution/steps/evaluateStep.h"
#include "evolution/steps/initPopulationStep.h"
#include "evolution/steps/reproduceStep.h"
#include "evolution/steps/selectStep.h"
#include "evolution/steps/tpgSteps.h"
#include "metrics/inputMetric.h"
#include "mutation/pointMutator.h"

void Evolution::OriginTPGEvolutionConfig::updateEmptyComponents()
{
    if(this->nodeTransfer == nullptr) {
        this->nodeTransfer = std::make_shared<Mutation::NodeTransferMutation>(0.1, 0.7, 0.7);
    }
    if(this->memberMutation == nullptr) {
        this->memberMutation = std::make_shared<Mutation::PointMutator>(0.2, 0.5, 0.5);
    }
    // The vertices are created and mutated by the node transfer.
    this->mutation = this->nodeTransfer;
    BasicEvolutionConfig::updateEmptyComponents();

    if(std::dynamic_pointer_cast<const Representations::TPG>(this->representation) == nullptr) {
        throw std::runtime_error("Evolution::OriginTPGEvolutionConfig: the representation must be a TPG");
    }
    if(this->memberRepresentation == nullptr) {
        throw std::runtime_error("Evolution::OriginTPGEvolutionConfig: the representation of the members must be set");
    }
    if(!Dimensions::DimensionFlow::acceptsRequirements(this->representation->getDimensionFlow().getInputDimensions(),
                                                       this->memberRepresentation->getDimensionFlow().getInputDimensions())) {
        throw std::runtime_error("Evolution::OriginTPGEvolutionConfig: input requirements not compatible between the TPG and the members");
    }
    if(!this->memberRepresentation->getDimensionFlow().getOutputDimension().isCompatibleWith(Dimensions::Requirement(Data::DataType::type<double>()))) {
        throw std::runtime_error("Evolution::OriginTPGEvolutionConfig: the members must output a double");
    }

    // The archive samples the inputs measured by the training evaluations (ignored if already in the template).
    if(this->archiveSize > 0) {
        this->trainingMetricTemplate->addRequiredMetric(std::make_unique<Metrics::InputMetric>());
    }
}

std::shared_ptr<Evolution::Module> Evolution::OriginTPGEvolutionConfig::makeValidationModule(const std::string& name, size_t period) const
{
    std::shared_ptr<Module> validation = BasicEvolutionConfig::makeValidationModule(name, period);

    // Outputs of the members, then of the shared vertices, are cached before the validation evaluation (prediction problems only).
    const std::shared_ptr<Evaluation::Problem> validated = (this->validationProblem != nullptr) ? this->validationProblem : this->problem;
    if(std::dynamic_pointer_cast<Evaluation::PredictionProblem>(validated) != nullptr) {
        validation->insertBefore(this->names.validationEvaluationStep, std::make_shared<CacheMemberOutputsStep>(
            std::vector<std::string>{this->names.population}, this->tpgNames.validationMemberOutputCache, validated->getName(),
            this->nbValidationIterations, Evaluation::Mode::VALIDATION, this->nbThreads, this->tpgNames.validationCacheMemberOutputsStep));
        validation->insertBefore(this->names.validationEvaluationStep, std::make_shared<CacheVertexOutputsStep>(
            std::vector<std::string>{this->names.population}, this->tpgNames.validationVertexOutputCache, validated->getName(),
            this->nbValidationIterations, Evaluation::Mode::VALIDATION, this->nbThreads, this->tpgNames.validationCacheVertexOutputsStep));
    }
    return validation;
}

std::shared_ptr<Evolution::Module> Evolution::OriginTPGEvolutionConfig::buildModule()
{
    // Step 1: build the basic evolution (it updates the empty components), its steps are then changed for the TPG.
    std::shared_ptr<Module> evolutionModule = BasicEvolutionConfig::buildModule();
    Module& initModule = *evolutionModule->getInitialization();
    const bool withArchive = this->archiveSize > 0;
    const std::string archiveName = withArchive ? this->tpgNames.archive : "";

    // Step 2: store the components of the members, read by the generator of the vertices.
    evolutionModule->getData().set(this->memberRepresentation->getName(), this->memberRepresentation);
    evolutionModule->getData().set(this->memberMutation->getName(), this->memberMutation);

    // Step 3: change the initialization.

    // The archive is created first, then the generator of the vertices, pointing to actions only, before the vertices.
    if(withArchive) {
        initModule.insertBefore(this->names.initPopulationStep, std::make_shared<CreateArchiveStep>(
            this->tpgNames.archive, this->representation->getDimensionFlow().getInputDimensions(), this->tpgNames.createArchiveStep));
    }
    initModule.insertBefore(this->names.initPopulationStep, std::make_shared<UpdateGeneratorStep>(
        this->names.generator, this->representation->getName(), this->memberRepresentation->getName(),
        this->memberMutation->getName(), archiveName, "", this->tpgNames.updateGeneratorStep));

    // After the fitness: protection of the aggregated vertices, then update of the archive.
    std::string position = this->names.trainingFitnessStep;
    if(this->protectAggregated) {
        initModule.insertAfter(position, std::make_shared<ProtectAggregatedStep>(
            this->names.population, this->tpgNames.protectedVertices, this->tpgNames.aggregators, this->tpgNames.protectAggregatedStep));
        position = this->tpgNames.protectAggregatedStep;
    }
    if(withArchive) {
        initModule.insertAfter(position, std::make_shared<UpdateArchiveStep>(
            this->names.population, this->names.trainingMetrics, this->tpgNames.archive, this->archiveSize, this->isArchiveFixed, this->tpgNames.updateArchiveStep));
    }

    // Step 4: change the generation.

    // Mutation of the offspring, with a generator whose destinations can be the current vertices, and nodes transferred from the population.
    evolutionModule->insertBefore(this->names.mutationStep, std::make_shared<UpdateGeneratorStep>(
        this->names.generator, this->representation->getName(), this->memberRepresentation->getName(),
        this->memberMutation->getName(), archiveName, this->names.population, this->tpgNames.updateGeneratorStep));
    evolutionModule->replace(this->names.mutationStep, std::make_shared<MutateVerticesStep>(
        this->names.offspring, this->names.population, this->names.generator,
        this->nodeTransfer->getName(), this->nbThreads, this->names.mutationStep));

    // Outputs of the members, then of the shared vertices, are cached before the evaluation (prediction problems only).
    if(std::dynamic_pointer_cast<Evaluation::PredictionProblem>(this->problem) != nullptr) {
        evolutionModule->insertBefore(this->names.trainingEvaluationStep, std::make_shared<CacheMemberOutputsStep>(
            std::vector<std::string>{this->names.population, this->names.offspring}, this->tpgNames.memberOutputCache, this->problem->getName(),
            this->nbTrainingIterations, Evaluation::Mode::TRAINING, this->nbThreads, this->tpgNames.cacheMemberOutputsStep));
        evolutionModule->insertBefore(this->names.trainingEvaluationStep, std::make_shared<CacheVertexOutputsStep>(
            std::vector<std::string>{this->names.population, this->names.offspring}, this->tpgNames.vertexOutputCache, this->problem->getName(),
            this->nbTrainingIterations, Evaluation::Mode::TRAINING, this->nbThreads, this->tpgNames.cacheVertexOutputsStep));
    }

    // After the surviving selection: protection of the aggregated vertices, then update of the archive.
    position = this->names.survivorSelectionStep;
    if(this->protectAggregated) {
        evolutionModule->insertAfter(position, std::make_shared<ProtectAggregatedStep>(
            this->names.population, this->tpgNames.protectedVertices, this->tpgNames.aggregators, this->tpgNames.protectAggregatedStep));
        position = this->tpgNames.protectAggregatedStep;
    }
    if(withArchive) {
        evolutionModule->insertAfter(position, std::make_shared<UpdateArchiveStep>(
            this->names.population, this->names.trainingMetrics, this->tpgNames.archive, this->archiveSize, this->isArchiveFixed, this->tpgNames.updateArchiveStep));
    }
    return evolutionModule;
}

double Evolution::OriginTPGEvolutionConfig::getNbVertices(const EvolutionData& state, const std::string& roots)
{
    const std::vector<std::shared_ptr<Individual>>& list = *state.get<std::vector<std::shared_ptr<Individual>>>(roots);
    return static_cast<double>(getAllVertices(std::vector<std::shared_ptr<const Individual>>(list.begin(), list.end())).size());
}

double Evolution::OriginTPGEvolutionConfig::getNbMembers(const EvolutionData& state, const std::string& roots)
{
    const std::vector<std::shared_ptr<Individual>>& list = *state.get<std::vector<std::shared_ptr<Individual>>>(roots);
    return static_cast<double>(getMembersOf(getAllVertices(std::vector<std::shared_ptr<const Individual>>(list.begin(), list.end()))).size());
}

std::shared_ptr<Evolution::LogModule> Evolution::OriginTPGEvolutionConfig::buildLogModule() const
{
    std::shared_ptr<LogModule> log = BasicEvolutionConfig::buildLogModule();

    // The number of vertices and of members of the graphs of the population, after its size.
    const std::string roots = this->names.population;
    log->insertAfter("Pop_Size", std::make_shared<LogColumnStep>("NbVertex",
        [roots](const EvolutionData& state) { return getNbVertices(state, roots); }, 0, std::set<std::string>{roots}));
    log->insertAfter("NbVertex", std::make_shared<LogColumnStep>("NbMember",
        [roots](const EvolutionData& state) { return getNbMembers(state, roots); }, 0, std::set<std::string>{roots}));
    return log;
}

// Individuals referenced by the value at a given index of the nodes of the first node group.
std::vector<std::shared_ptr<const Individual>> Evolution::getReferencedBy(const Individual& vertex, size_t idxValue)
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
