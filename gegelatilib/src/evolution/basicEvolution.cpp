#include "evolution/basicEvolution.h"

#include <stdexcept>
#include <tuple>
#include <utility>

#include "evaluation/parallelEvaluator.h"
#include "evolution/steps/assignFitnessStep.h"
#include "evolution/steps/evaluateStep.h"
#include "evolution/steps/initPopulationStep.h"
#include "evolution/steps/mutateStep.h"
#include "evolution/steps/reproduceStep.h"
#include "evolution/steps/selectStep.h"
#include "evaluation/predictionProblem.h"
#include "evaluation/reinforcementProblem.h"
#include "fitnessAssignment/defaultAssigner.h"
#include "metrics/predictionMetric.h"
#include "metrics/rewardsMetric.h"
#include "mutation/pointMutator.h"
#include "reproduction/replicator.h"
#include "selection/randomSelector.h"
#include "selection/truncationSelector.h"

namespace {
    // Check that a problem can evaluate the individuals of a representation.
    void checkCompatible(const Representations::Representation& representation, const Evaluation::Problem& problem)
    {
        if(!Dimensions::DimensionFlow::acceptsRequirements(problem.getInputDimensions(), representation.getDimensionFlow().getInputDimensions())) {
            throw std::runtime_error("Evolution::BasicEvolution: input requirements not compatible between representation and problem");
        }
        if(!representation.getDimensionFlow().getOutputDimension().isCompatibleWith(problem.getOutputDimension())) {
            throw std::runtime_error("Evolution::BasicEvolution: output requirements not compatible between representation and problem ("
                + representation.getDimensionFlow().getOutputDimension().toString() + " =/= " + problem.getOutputDimension().toString() + ")");
        }
    }
}

Evolution::Module Evolution::BasicEvolution::buildModule() const
{
    Module module(this->seed);
    this->addTo(module);
    return module;
}

void Evolution::BasicEvolution::addTo(Module& module) const
{
    this->checkSettings();
    const std::shared_ptr<Evaluation::Evaluator> evaluator = this->makeEvaluator();
    const auto [metricTemplate, fitnessMetricHash] = this->makeMetrics(*this->problem);
    const std::shared_ptr<Mutation::Mutator> usedMutation =
        (this->mutation != nullptr) ? this->mutation : std::make_shared<Mutation::PointMutator>(0.2, 0.5, 0.5);
    const std::shared_ptr<const GraphBased::GenotypeGenerator> generator(this->representation->getGenotypeGenerator().cloneUniquePtr());

    const std::shared_ptr<const Population> population = module.addPopulation(this->representation, this->sizePopulation, this->populationName);
    const std::string individuals = population->individuals();
    const std::string offspring = population->data(this->offspringName);
    const std::string trainingMetrics = population->data(this->trainingMetricsName);

    // Initialization.
    module.addInitStep(std::make_shared<InitPopulationStep>(population, usedMutation, generator));
    module.addInitStep(std::make_shared<EvaluateStep>(std::vector<std::string>{individuals}, trainingMetrics, this->problem, evaluator, metricTemplate,
                                                      this->nbTrainingIterations, Evaluation::Mode::TRAINING, "InitialEvaluation"));
    this->addFitnessSteps(module, *population, fitnessMetricHash, true);

    // Generation.
    this->addReproductionSteps(module, *population);
    module.addStep(std::make_shared<MutateStep>(offspring, usedMutation, generator, this->nbThreads, "Mutation"));
    module.addStep(std::make_shared<EvaluateStep>(std::vector<std::string>{individuals, offspring}, trainingMetrics, this->problem, evaluator, metricTemplate,
                                                  this->nbTrainingIterations, Evaluation::Mode::TRAINING, "TrainingEvaluation"));
    this->addFitnessSteps(module, *population, fitnessMetricHash, false);
    this->addSurvivalStep(module, *population);
    this->addValidationSteps(module, *population, evaluator);
}

void Evolution::BasicEvolution::checkSettings() const
{
    if(this->representation == nullptr || this->problem == nullptr) {
        throw std::runtime_error("Evolution::BasicEvolution: the representation and the problem must be set");
    }
    if(this->sizePopulation == 0 || this->nbOffspring == 0) {
        throw std::runtime_error("Evolution::BasicEvolution: the size of the population and the number of offspring must be set");
    }
    if(this->validationPeriod == 0) {
        throw std::runtime_error("Evolution::BasicEvolution: the validation period cannot be zero");
    }
    checkCompatible(*this->representation, *this->problem);
    this->makeMetrics(*this->problem);
    if(this->nbValidationIterations > 0 && this->validationProblem != nullptr) {
        checkCompatible(*this->representation, *this->validationProblem);
        this->makeMetrics(*this->validationProblem);
    }
}

std::pair<std::shared_ptr<Metrics::MetricMapTemplate>, uint64_t> Evolution::BasicEvolution::makeMetrics(const Evaluation::Problem& evaluatedProblem) const
{
    std::pair<std::shared_ptr<Metrics::MetricMapTemplate>, uint64_t> result;
    if(dynamic_cast<const Evaluation::PredictionProblem*>(&evaluatedProblem) != nullptr) {
        result = {std::make_shared<Metrics::MetricMapTemplate>(std::make_unique<Metrics::PredictionMetric>()), Metrics::PredictionMetric::staticHash()};
    } else if(dynamic_cast<const Evaluation::ReinforcementProblem*>(&evaluatedProblem) != nullptr) {
        result = {std::make_shared<Metrics::MetricMapTemplate>(std::make_unique<Metrics::RewardsMetric>()), Metrics::RewardsMetric::staticHash()};
    } else {
        throw std::runtime_error("Evolution::BasicEvolution: the problems must be PredictionProblem or ReinforcementProblem");
    }
    for(const std::shared_ptr<const Metrics::Metric>& metric: this->metrics) {
        if(metric == nullptr) {
            throw std::runtime_error("Evolution::BasicEvolution: the metrics cannot be null");
        }
        result.first->addRequiredMetric(metric->cloneEmptyPtr());
    }
    return result;
}

std::shared_ptr<Evaluation::Evaluator> Evolution::BasicEvolution::makeEvaluator() const
{
    if(this->nbThreads > 1) {
        return std::make_shared<Evaluation::ParallelEvaluator>(this->nbThreads);
    }
    return std::make_shared<Evaluation::Evaluator>();
}

std::shared_ptr<FitnessAssignment::Assigner> Evolution::BasicEvolution::usedAssigner() const
{
    return (this->assigner != nullptr) ? this->assigner : std::make_shared<FitnessAssignment::DefaultAssigner>();
}

void Evolution::BasicEvolution::addFitnessSteps(Module& module, const Population& population, uint64_t metricHash, bool atInitialization) const
{
    std::vector<std::shared_ptr<Step>> steps;
    steps.push_back(std::make_shared<AssignFitnessStep>(
        population.data(this->trainingMetricsName),
        population.data(this->trainingFitnessName),
        this->usedAssigner(),
        metricHash,
        "TrainingFitness"));
    // Parents are selected with their own fitness, computed for the initial population and at each generation.
    if(this->parentAssigner != nullptr) {
        steps.push_back(
            std::make_shared<AssignFitnessStep>(
                population.data(this->trainingMetricsName), population.data(this->parentFitnessName), this->parentAssigner, metricHash, "ParentFitness"));
    }

    for(const std::shared_ptr<Step>& step: steps) {
        if(atInitialization) {
            module.addInitStep(step);
        } else {
            module.addStep(step);
        }
    }
}

void Evolution::BasicEvolution::addReproductionSteps(Module& module, const Population& population) const
{
    const std::shared_ptr<Selection::Selector> selector =
        (this->parentSelection != nullptr) ? this->parentSelection : std::make_shared<Selection::RandomSelector>(true);
    const std::shared_ptr<Reproduction::Reproducer> reproducer =
        (this->breeder != nullptr) ? this->breeder : std::make_shared<Reproduction::Replicator>();
    const std::string parents = population.data(this->parentsName);

    module.addStep(std::make_shared<SelectStep>(std::vector<std::string>{population.individuals()},
                                                population.data((this->parentAssigner != nullptr) ? this->parentFitnessName : this->trainingFitnessName),
                                                selector, this->nbOffspring, parents, false, "ParentSelection"));
    module.addStep(std::make_shared<ReproduceStep>(parents, reproducer, this->nbOffspring, population.data(this->offspringName), "Reproduction"));
}

void Evolution::BasicEvolution::addSurvivalStep(Module& module, const Population& population) const
{
    const std::shared_ptr<Selection::Selector> selector =
        (this->survivingSelection != nullptr) ? this->survivingSelection : std::make_shared<Selection::TruncationSelector>();
    module.addStep(std::make_shared<SelectStep>(std::vector<std::string>{population.individuals(), population.data(this->offspringName)}, population.data(this->trainingFitnessName),
                                                selector, this->sizePopulation, population.individuals(), true, "SurvivorSelection"));
}

void Evolution::BasicEvolution::addValidationSteps(Module& module, const Population& population, std::shared_ptr<Evaluation::Evaluator> evaluator) const
{
    if(this->nbValidationIterations == 0) {
        return;
    }
    const std::shared_ptr<Evaluation::Problem> evaluatedProblem = (this->validationProblem != nullptr) ? this->validationProblem : this->problem;
    const auto [metricTemplate, metricHash] = this->makeMetrics(*evaluatedProblem);
    const std::string validationMetrics = population.data(this->validationMetricsName);

    auto evaluate = module.addStep(std::make_shared<EvaluateStep>(std::vector<std::string>{population.individuals()}, validationMetrics, evaluatedProblem,
                                                                  std::move(evaluator), metricTemplate, this->nbValidationIterations,
                                                                  Evaluation::Mode::VALIDATION, "ValidationEvaluation"));
    auto assign = module.addStep(std::make_shared<AssignFitnessStep>(validationMetrics, population.data(this->validationFitnessName),
                                                                     (this->validationAssigner != nullptr) ? this->validationAssigner : this->usedAssigner(),
                                                                     metricHash, "ValidationFitness"));
    evaluate->setPeriod(this->validationPeriod);
    assign->setPeriod(this->validationPeriod);
}
