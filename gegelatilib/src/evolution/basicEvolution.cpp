#include "evolution/basicEvolution.h"

#include <stdexcept>
#include <tuple>
#include <utility>

#include "evaluation/parallelEvaluator.h"
#include "evaluation/predictionProblem.h"
#include "evaluation/reinforcementProblem.h"
#include "evolution/steps/assignFitnessStep.h"
#include "evolution/steps/createGeneratorStep.h"
#include "evolution/steps/evaluateStep.h"
#include "evolution/steps/initPopulationStep.h"
#include "evolution/steps/mutateStep.h"
#include "evolution/steps/reproduceStep.h"
#include "evolution/steps/selectStep.h"
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

std::shared_ptr<Evolution::Module> Evolution::BasicEvolution::build() const
{
    this->checkSettings();

    // Step 1: from the settings, create what the modules need.
    const auto [metricTemplate, fitnessMetricHash] = this->makeMetrics(*this->problem);
    const std::shared_ptr<Evaluation::Evaluator> evaluator = this->makeEvaluator();
    const std::shared_ptr<Mutation::Mutator> mutator =
        (this->mutation != nullptr) ? this->mutation : std::make_shared<Mutation::PointMutator>(0.2, 0.5, 0.5);

    // Step 2: build the modules. The initialization creates and evaluates the population...
    auto initModule = std::make_shared<Module>(this->names.initialization);
    initModule->add((this->generator != nullptr)
                        ? std::make_shared<CreateGeneratorStep>(this->names.generator, [custom = this->generator](const EvolutionData&) { return custom; }, std::set<std::string>{},
                                                                this->names.createGeneratorStep)
                        : std::make_shared<CreateGeneratorStep>(this->names.generator, this->representation, this->names.createGeneratorStep));
    initModule->add(std::make_shared<InitPopulationStep>(this->names.population, this->representation, this->sizePopulation, mutator, this->names.generator, this->names.initPopulationStep));
    initModule->add(std::make_shared<EvaluateStep>(std::vector<std::string>{this->names.population}, this->names.trainingMetrics, this->problem,
                                             evaluator, metricTemplate, this->nbTrainingIterations, Evaluation::Mode::TRAINING, this->names.initialEvaluationStep));
    this->addFitnessSteps(*initModule, fitnessMetricHash);

    // ... then a generation selects parents, creates and mutates the offspring, evaluates them and keeps the best.
    auto evolutionModule = std::make_shared<Module>(this->names.evolution);
    this->addReproductionSteps(*evolutionModule);
    evolutionModule->add(std::make_shared<MutateStep>(this->names.offspring, mutator, this->names.generator, this->nbThreads, this->names.mutationStep));
    evolutionModule->add(std::make_shared<EvaluateStep>(std::vector<std::string>{this->names.population, this->names.offspring},
                                                   this->names.trainingMetrics, this->problem, evaluator, metricTemplate, this->nbTrainingIterations,
                                                   Evaluation::Mode::TRAINING, this->names.trainingEvaluationStep));
    this->addFitnessSteps(*evolutionModule, fitnessMetricHash);
    this->addSurvivalStep(*evolutionModule);
    this->addValidationSteps(*evolutionModule, evaluator);
    this->addLogStep(*evolutionModule);

    evolutionModule->setInitialization(initModule);
    return evolutionModule;
}

std::vector<Evolution::LogStep::Column> Evolution::BasicEvolution::logColumns() const
{
    std::vector<LogStep::Column> columns{sizeColumn(this->names.population)};
    const auto add = [&columns](const std::vector<LogStep::Column>& more) { columns.insert(columns.end(), more.begin(), more.end()); };
    add(fitnessColumns("Fit", this->names.population, this->names.trainingFitness));
    if(this->nbValidationIterations > 0) {
        add(fitnessColumns("Val", this->names.population, this->names.validationFitness));
    }
    return columns;
}

void Evolution::BasicEvolution::addLogStep(Module& step) const
{
    if(this->logOutput != nullptr) {
        step.add(std::make_shared<LogStep>(*this->logOutput, this->logColumns(), "", this->names.logStep));
    }
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

void Evolution::BasicEvolution::addFitnessSteps(Module& step, uint64_t metricHash) const
{
    step.add(std::make_shared<AssignFitnessStep>(this->names.trainingMetrics, this->names.trainingFitness, this->usedAssigner(), metricHash,
                                                     this->names.trainingFitnessStep));
    // Parents are selected with their own fitness, computed for the initial population and at each generation.
    if(this->parentAssigner != nullptr) {
        step.add(std::make_shared<AssignFitnessStep>(this->names.trainingMetrics, this->names.parentFitness, this->parentAssigner, metricHash,
                                                         this->names.parentFitnessStep));
    }
}

void Evolution::BasicEvolution::addReproductionSteps(Module& step) const
{
    const std::shared_ptr<Selection::Selector> selector =
        (this->parentSelection != nullptr) ? this->parentSelection : std::make_shared<Selection::RandomSelector>(true);
    const std::shared_ptr<Reproduction::Reproducer> reproducer =
        (this->breeder != nullptr) ? this->breeder : std::make_shared<Reproduction::Replicator>();
    const auto& fitness = (this->parentAssigner != nullptr) ? this->names.parentFitness : this->names.trainingFitness;

    step.add(std::make_shared<SelectStep>(std::vector<std::string>{this->names.population}, fitness, selector,
                                              this->nbOffspring, this->names.parents, false, this->names.parentSelectionStep));
    step.add(std::make_shared<ReproduceStep>(this->names.parents, reproducer, this->nbOffspring, this->names.offspring, this->names.reproductionStep));
}

void Evolution::BasicEvolution::addSurvivalStep(Module& step) const
{
    const std::shared_ptr<Selection::Selector> selector =
        (this->survivingSelection != nullptr) ? this->survivingSelection : std::make_shared<Selection::TruncationSelector>();
    step.add(std::make_shared<SelectStep>(std::vector<std::string>{this->names.population, this->names.offspring},
                                              this->names.trainingFitness, selector, this->sizePopulation, this->names.population, true, this->names.survivorSelectionStep));
}

void Evolution::BasicEvolution::addValidationSteps(Module& step, std::shared_ptr<Evaluation::Evaluator> evaluator) const
{
    if(this->nbValidationIterations == 0) {
        return;
    }
    const std::shared_ptr<Evaluation::Problem> evaluatedProblem = (this->validationProblem != nullptr) ? this->validationProblem : this->problem;
    const auto [metricTemplate, metricHash] = this->makeMetrics(*evaluatedProblem);

    auto evaluate = std::make_shared<EvaluateStep>(std::vector<std::string>{this->names.population},
                                                   this->names.validationMetrics, evaluatedProblem, std::move(evaluator), metricTemplate,
                                                   this->nbValidationIterations, Evaluation::Mode::VALIDATION, this->names.validationEvaluationStep);
    auto assign = std::make_shared<AssignFitnessStep>(this->names.validationMetrics, this->names.validationFitness,
                                                      (this->validationAssigner != nullptr) ? this->validationAssigner : this->usedAssigner(), metricHash,
                                                      this->names.validationFitnessStep);
    evaluate->setPeriod(this->validationPeriod);
    assign->setPeriod(this->validationPeriod);
    step.add(evaluate);
    step.add(assign);
}
