#include "evolution/basicEvolutionConfig.h"

#include <stdexcept>
#include <tuple>
#include <utility>

#include "evolution/steps/assignFitnessStep.h"
#include "evolution/steps/evaluateStep.h"
#include "evolution/steps/initPopulationStep.h"
#include "evolution/steps/mutateStep.h"
#include "evolution/steps/reproduceStep.h"
#include "evolution/steps/selectStep.h"
#include "fitnessAssignment/defaultAssigner.h"
#include "mutation/pointMutator.h"
#include "reproduction/replicator.h"
#include "selection/randomSelector.h"
#include "selection/truncationSelector.h"

// Check that a problem can evaluate the individuals of a representation.
void Evolution::BasicEvolutionConfig::checkCompatible(const Representations::Representation& representation, const Evaluation::Problem& problem)
{
    if(!Dimensions::DimensionFlow::acceptsRequirements(problem.getInputDimensions(), representation.getDimensionFlow().getInputDimensions())) {
        throw std::runtime_error("Evolution::BasicEvolutionConfig: input requirements not compatible between representation and problem");
    }
    if(!representation.getDimensionFlow().getOutputDimension().isCompatibleWith(problem.getOutputDimension())) {
        throw std::runtime_error("Evolution::BasicEvolutionConfig: output requirements not compatible between representation and problem ("
            + representation.getDimensionFlow().getOutputDimension().toString() + " =/= " + problem.getOutputDimension().toString() + ")");
    }
}

void Evolution::BasicEvolutionConfig::updateEmptyComponents()
{
    if(this->representation == nullptr || this->problem == nullptr) {
        throw std::runtime_error("Evolution::BasicEvolutionConfig: the representation and the problem must be set");
    }
    if(this->sizePopulation == 0 || this->nbOffspring == 0) {
        throw std::runtime_error("Evolution::BasicEvolutionConfig: the size of the population and the number of offspring must be set");
    }
    if(this->validationPeriod == 0 && this->nbValidationIterations > 0) {
        throw std::runtime_error("Evolution::BasicEvolutionConfig: the validation period cannot be zero");
    }
    checkCompatible(*this->representation, *this->problem);

    // Training: the assigners read the metrics of the training template.
    if(this->trainingMetricTemplate == nullptr) {
        this->trainingMetricTemplate = std::make_shared<Metrics::MetricMapTemplate>();
    }
    const std::unique_ptr<Metrics::Metric> defaultTrainingMetric = this->problem->getDefaultFitnessMetric();
    if(this->assigner == nullptr) {
        this->assigner = std::make_shared<FitnessAssignment::DefaultAssigner>(defaultTrainingMetric->hash());
    }
    if(this->assigner->getMetricHash() == defaultTrainingMetric->hash()) {
        // Add the default fitness metric to the training template (ignored if already in it, the template being a set).
        this->trainingMetricTemplate->addRequiredMetric(defaultTrainingMetric->cloneEmptyPtr());
    } else if(!this->trainingMetricTemplate->hasMetricHash(this->assigner->getMetricHash())) {
        throw std::runtime_error("Evolution::BasicEvolutionConfig: the trainingMetricTemplate does not contain the metric required by the assigner");
    }

    // Check the metric of the parent assigner, if any.
    if(this->parentAssigner != nullptr) {
        if(this->parentAssigner->getMetricHash() == defaultTrainingMetric->hash()) {
            this->trainingMetricTemplate->addRequiredMetric(defaultTrainingMetric->cloneEmptyPtr());
        } else if(!this->trainingMetricTemplate->hasMetricHash(this->parentAssigner->getMetricHash())) {
            throw std::runtime_error("Evolution::BasicEvolutionConfig: the trainingMetricTemplate does not contain the metric required by the parentAssigner");
        }
    }

    if(this->parentSelection == nullptr) {
        this->parentSelection = std::make_shared<Selection::RandomSelector>(true);
    }
    if(this->survivingSelection == nullptr) {
        this->survivingSelection = std::make_shared<Selection::TruncationSelector>();
    }
    if(this->breeder == nullptr) {
        this->breeder = std::make_shared<Reproduction::Replicator>();
    }

    if(this->mutation == nullptr) {
        this->mutation = std::make_shared<Mutation::PointMutator>(0.2, 0.5, 0.5);
    }

    if(this->generator == nullptr) {
        this->generator = std::make_shared<GraphBased::GenotypeGenerator>(this->representation->getGenotypeConstraint());
    }

    // Validation: evaluated on the validationProblem if set, on the training problem otherwise, with the validationAssigner if set, the assigner otherwise.
    if(this->nbValidationIterations > 0) {
        std::unique_ptr<Metrics::Metric> defaultValidationMetric = this->problem->getDefaultFitnessMetric();
        if(this->validationProblem != nullptr) {
            checkCompatible(*this->representation, *this->validationProblem);
            defaultValidationMetric = this->validationProblem->getDefaultFitnessMetric();
        }

        if(this->validationMetricTemplate == nullptr) {
            this->validationMetricTemplate = std::make_shared<Metrics::MetricMapTemplate>();
        }
        const uint64_t validationMetricHash = (this->validationAssigner != nullptr) ? this->validationAssigner->getMetricHash() : this->assigner->getMetricHash();
        if(validationMetricHash == defaultValidationMetric->hash()) {
            // Add the default fitness metric to the validation template (ignored if already in it).
            this->validationMetricTemplate->addRequiredMetric(defaultValidationMetric->cloneEmptyPtr());
        } else if(!this->validationMetricTemplate->hasMetricHash(validationMetricHash)) {
            throw std::runtime_error("Evolution::BasicEvolutionConfig: the validationMetricTemplate does not contain the metric required by the validationAssigner");
        }
    }
}

void Evolution::BasicEvolutionConfig::addComponents(Module& module) const
{
    EvolutionData& data = module.getData();
    data.set(this->problem->getName(), this->problem);
    data.set(this->representation->getName(), this->representation);
    data.set(this->assigner->getName(), this->assigner);
    data.set(this->parentSelection->getName(), this->parentSelection);
    data.set(this->survivingSelection->getName(), this->survivingSelection);
    data.set(this->breeder->getName(), this->breeder);
    data.set(this->names.trainingMetricTemplate, this->trainingMetricTemplate);
    data.set(this->mutation->getName(), this->mutation);
    data.set(this->names.generator, this->generator);

    // Specific assigner for parent selection if required.
    if(this->parentAssigner != nullptr) {
        data.set(this->parentAssigner->getName(), this->parentAssigner);
    }
    // Validation problem and assigner (if any, the training ones are used otherwise) and metrics.
    if(this->nbValidationIterations > 0) {
        if(this->validationProblem != nullptr) {
            data.set(this->validationProblem->getName(), this->validationProblem);
        }
        data.set(this->names.validationMetricTemplate, this->validationMetricTemplate);
        if(this->validationAssigner != nullptr) {
            data.set(this->validationAssigner->getName(), this->validationAssigner);
        }
    }
}

std::shared_ptr<Evolution::Module> Evolution::BasicEvolutionConfig::makeValidationModule(const std::string& name, size_t period) const
{
    // The training problem and assigner are used if the validation has none.
    const std::string problemName = (this->validationProblem != nullptr) ? this->validationProblem->getName() : this->problem->getName();
    const std::string assignerName = (this->validationAssigner != nullptr) ? this->validationAssigner->getName() : this->assigner->getName();

    std::shared_ptr<Module> validation = std::make_shared<Module>(name);
    validation->addSubModule(std::make_shared<EvaluateStep>(
        std::vector<std::string>{this->names.population}, this->names.validationMetrics, problemName,
        this->names.validationMetricTemplate, this->nbValidationIterations, Evaluation::Mode::VALIDATION, this->nbThreads,
        this->names.validationEvaluationStep));
    validation->addSubModule(std::make_shared<AssignFitnessStep>(
        this->names.validationMetrics, this->names.validationFitness, assignerName, this->names.validationFitnessStep));
    validation->setPeriod(period);
    return validation;
}

std::shared_ptr<Evolution::Module> Evolution::BasicEvolutionConfig::buildModule()
{
    // Step 1: from the settings, create what the modules need.
    this->updateEmptyComponents();

    // Step 2: create the evolutionary module and store the components in its data.
    std::shared_ptr<Module> evolutionModule = std::make_shared<Module>(this->names.evolution);
    this->addComponents(*evolutionModule);

    // Step 3: build the initialization module: creates and evaluates the population...
    Module& initModule = evolutionModule->setInitialization();
    initModule.addSubModule(std::make_shared<InitPopulationStep>(
        this->names.population, this->representation->getName(),
        this->sizePopulation, this->mutation->getName(),
        this->names.generator, this->names.initPopulationStep));

    initModule.addSubModule(std::make_shared<EvaluateStep>(
        std::vector<std::string>{this->names.population},
        this->names.trainingMetrics, this->problem->getName(), this->names.trainingMetricTemplate,
        this->nbTrainingIterations, Evaluation::Mode::TRAINING, this->nbThreads, this->names.initialEvaluationStep));

    initModule.addSubModule(std::make_shared<AssignFitnessStep>(
        this->names.trainingMetrics, this->names.trainingFitness, this->assigner->getName(), this->names.trainingFitnessStep));

    // Validation of the initial population.
    if(this->nbValidationIterations > 0) {
        initModule.addSubModule(this->makeValidationModule("Validation", 1));
    }

    // Step 4: build the main module executed at each generation.

    // Parent fitness assignment (not mandatory), from the metrics of the previous generation.
    std::string parentFitnessName = this->names.trainingFitness;
    if(this->parentAssigner != nullptr) {
        evolutionModule->addSubModule(std::make_shared<AssignFitnessStep>(
            this->names.trainingMetrics, this->names.parentFitness, this->parentAssigner->getName(), this->names.parentFitnessStep));
        parentFitnessName = this->names.parentFitness;
    }

    // Parent selection
    evolutionModule->addSubModule(std::make_shared<SelectStep>(
        std::vector<std::string>{this->names.population}, parentFitnessName,
        this->parentSelection->getName(), this->nbOffspring,
        this->names.parents, this->names.parentSelectionStep));

    // Reproduction
    evolutionModule->addSubModule(std::make_shared<ReproduceStep>(
        this->names.parents, this->breeder->getName(), this->nbOffspring, this->names.offspring, this->names.reproductionStep));

    // Mutation of offspring
    evolutionModule->addSubModule(std::make_shared<MutateStep>(
        this->names.offspring, this->mutation->getName(), this->names.generator, this->nbThreads, this->names.mutationStep));

    // Evaluation
    evolutionModule->addSubModule(std::make_shared<EvaluateStep>(
        std::vector<std::string>{this->names.population, this->names.offspring},
        this->names.trainingMetrics, this->problem->getName(), this->names.trainingMetricTemplate, this->nbTrainingIterations,
        Evaluation::Mode::TRAINING, this->nbThreads, this->names.trainingEvaluationStep));

    // Fitness assignment
    evolutionModule->addSubModule(std::make_shared<AssignFitnessStep>(
        this->names.trainingMetrics, this->names.trainingFitness, this->assigner->getName(), this->names.trainingFitnessStep));

    // Surviving selection
    evolutionModule->addSubModule(std::make_shared<SelectStep>(
        std::vector<std::string>{this->names.population, this->names.offspring},
        this->names.trainingFitness, this->survivingSelection->getName(), this->sizePopulation,
        this->names.population, this->names.survivorSelectionStep));

    // Validation, every validationPeriod generations.
    if(this->nbValidationIterations > 0) {
        evolutionModule->addSubModule(this->makeValidationModule("Validation", this->validationPeriod));
    }

    // Step 5: add the logger step (not mandatory).
    if(this->logOutput != nullptr) {
        evolutionModule->addSubModule(this->buildLogModule());
    }
    return evolutionModule;
}

std::shared_ptr<Evolution::LogModule> Evolution::BasicEvolutionConfig::buildLogModule() const
{
    BasicLogConfig logConfig;
    logConfig.output = this->logOutput;
    logConfig.name = this->names.logStep;
    logConfig.population = this->names.population;
    logConfig.trainingFitness = this->names.trainingFitness;
    if(this->nbValidationIterations > 0) {
        logConfig.validationFitness = this->names.validationFitness;
    }
    return logConfig.buildModule();
}
