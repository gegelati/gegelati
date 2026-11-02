#include "evolution/originTPGEvolution.h"

#include <stdexcept>

#include "evaluation/predictionProblem.h"
#include "evolution/steps/evaluateStep.h"
#include "metrics/inputMetric.h"
#include "mutation/pointMutator.h"

void Evolution::OriginTPGEvolution::addTo(Module& module) const
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

    const std::shared_ptr<Evaluation::Evaluator> evaluator = this->makeEvaluator();
    const bool withArchive = this->archiveSize > 0;
    const std::shared_ptr<Mutation::NodeTransferMutation> transfer =
        (this->nodeTransfer != nullptr) ? this->nodeTransfer : std::make_shared<Mutation::NodeTransferMutation>(0.1, 0.7, 0.7);
    const std::shared_ptr<const Mutation::Mutator> memberMutator =
        (this->memberMutation != nullptr) ? this->memberMutation : std::make_shared<Mutation::PointMutator>(0.2, 0.5, 0.5);

    // The archive samples the inputs measured during the training evaluations, a fixed archive only samples them once.
    const auto [initMetrics, fitnessMetricHash] = this->makeMetrics(*this->problem);
    const std::shared_ptr<Metrics::MetricMapTemplate> generationMetrics = this->makeMetrics(*this->problem).first;
    if(withArchive) {
        initMetrics->addRequiredMetric(std::make_unique<Metrics::InputMetric>());
        if(!this->isArchiveFixed) {
            generationMetrics->addRequiredMetric(std::make_unique<Metrics::InputMetric>());
        }
    }

    const std::shared_ptr<const Population> population = module.addPopulation(this->representation, this->sizePopulation, this->populationName);
    const std::string roots = population->individuals();
    const std::string offspring = population->data(this->offspringName);
    const std::string trainingMetrics = population->data(this->trainingMetricsName);
    const std::string archive = population->data(this->archiveName);

    // The protection and the update of the archive are the same steps at initialization and at each generation.
    const std::shared_ptr<ProtectAggregatedStep> protect = this->protectAggregated ? std::make_shared<ProtectAggregatedStep>(roots) : nullptr;
    const std::shared_ptr<UpdateArchiveStep> updateArchive =
        withArchive ? std::make_shared<UpdateArchiveStep>(roots, trainingMetrics, archive, this->archiveSize, this->isArchiveFixed) : nullptr;

    // Initialization.
    if(withArchive) {
        module.addInitStep(std::make_shared<CreateArchiveStep>(archive, tpgRepresentation->getDimensionFlow().getInputDimensions()));
    }
    module.addInitStep(std::make_shared<InitVerticesStep>(population, transfer, this->memberRepresentation, memberMutator, archive, "InitPopulation"));
    module.addInitStep(std::make_shared<EvaluateStep>(std::vector<std::string>{roots}, trainingMetrics, this->problem, evaluator, initMetrics,
                                                      this->nbTrainingIterations, Evaluation::Mode::TRAINING, "InitialEvaluation"));
    this->addFitnessSteps(module, *population, fitnessMetricHash, true);
    if(protect != nullptr) {
        module.addInitStep(protect);
    }
    if(updateArchive != nullptr) {
        module.addInitStep(updateArchive);
    }

    // Generation.
    this->addReproductionSteps(module, *population);
    // Nodes transferred by the mutation of the offspring are the ones of the current population.
    module.addStep(std::make_shared<MutateVerticesStep>(offspring, roots, archive, tpgRepresentation, transfer, this->memberRepresentation, memberMutator,
                                                        this->nbThreads, "Mutation"));
    if(const auto predictionProblem = std::dynamic_pointer_cast<Evaluation::PredictionProblem>(this->problem)) {
        module.addStep(std::make_shared<CacheMemberOutputsStep>(std::vector<std::string>{roots, offspring}, population->data(this->cacheName), predictionProblem,
                                                                evaluator, this->nbTrainingIterations));
    }
    module.addStep(std::make_shared<EvaluateStep>(std::vector<std::string>{roots, offspring}, trainingMetrics, this->problem, evaluator, generationMetrics,
                                                  this->nbTrainingIterations, Evaluation::Mode::TRAINING, "TrainingEvaluation"));
    this->addFitnessSteps(module, *population, fitnessMetricHash, false);
    this->addSurvivalStep(module, *population);
    if(protect != nullptr) {
        module.addStep(protect);
    }
    if(updateArchive != nullptr) {
        module.addStep(updateArchive);
    }
    this->addValidationSteps(module, *population, evaluator);
}

std::vector<Evolution::LogStep::Column> Evolution::tpgLogColumns(const std::string& roots)
{
    return {
        {"NbVertex", [roots](const EvolutionState& state) { return Evolution::getAllVertices(*state.get<std::vector<std::shared_ptr<const Individual>>>(roots)).size(); }},
        {"NbMember", [roots](const EvolutionState& state) { return Evolution::getMembersOf(Evolution::getAllVertices(*state.get<std::vector<std::shared_ptr<const Individual>>>(roots))).size(); }}};
}
