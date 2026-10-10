#include "evolution/basicmodule.h"

#include <algorithm>
#include <chrono>
#include <limits>

Evolution::BasicModule::BasicModule(
    const Representations::Representation& representation, 
    Evaluation::Problem& problem,
    size_t sizePopulation, size_t nbOffspring, size_t nbTrainingIterations, uint64_t seed, size_t nbThreads, 
    std::shared_ptr<FitnessAssignment::Assigner> assigner,
    std::shared_ptr<Selection::Selector> parentSelection ,
    std::shared_ptr<Selection::Selector> survivingSelection,
    std::shared_ptr<Reproduction::Reproducer> breeder,
    std::shared_ptr<Mutation::Mutator> mutation
): Module(), representation{representation}, problem{problem}, sizePopulation{sizePopulation}, 
nbOffspring{nbOffspring}, nbTrainingIterations{nbTrainingIterations}, seed{seed}, nbThreads{nbThreads},
assigner{assigner}, parentSelection{parentSelection}, survivingSelection{survivingSelection},
breeder{breeder}, mutation{mutation}
{
    if(!Dimensions::DimensionFlow::acceptsRequirements(problem.getInputDimensions(), representation.getDimensionFlow().getInputDimensions())) {
        throw std::runtime_error("Evolution::BasicModule: Cannot build module with input requirements not compatible between representation and problem");
    }
    if(!representation.getDimensionFlow().getOutputDimension().isCompatibleWith(problem.getOutputDimension())) {
        throw std::runtime_error("Evolution::BasicModule: Cannot build module with output requirements not compatible between representation and problem (" + representation.getDimensionFlow().getOutputDimension().toString() + " =/= " + problem.getOutputDimension().toString() + ")");
    }

    if(this->nbThreads > 1) {
        this->evaluator = std::make_unique<Evaluation::ParallelEvaluator>(nbThreads);
    } else {
        this->evaluator = std::make_unique<Evaluation::Evaluator>();
    }

    if(this->assigner == nullptr) {
        this->assigner = std::make_shared<FitnessAssignment::DefaultAssigner>();
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
        this->mutation = std::make_shared<Mutation::PointMutator>(0.5, 0.1, 0.1);
    }

    this->generator = this->representation.getGenotypeGenerator().cloneUniquePtr();
    this->rng.setSeed(seed);
    this->initializeMetricTemplate();
} 

void Evolution::BasicModule::initializeMetricTemplate() 
{
    if(dynamic_cast<Evaluation::PredictionProblem*>(&this->problem) != nullptr) {
        this->fitnessMetricHash = Metrics::PredictionMetric::staticHash();
        this->metricTemplate = std::make_shared<Metrics::MetricMapTemplate>(std::make_unique<Metrics::PredictionMetric>());
    } else if(dynamic_cast<Evaluation::ReinforcementProblem*>(&this->problem) != nullptr) {
        this->fitnessMetricHash = Metrics::RewardsMetric::staticHash();
        this->metricTemplate = std::make_shared<Metrics::MetricMapTemplate>(std::make_unique<Metrics::RewardsMetric>());
    }
}

void Evolution::BasicModule::addMetricToTemplate(std::unique_ptr<Metrics::Metric> metric) 
{
    this->metricTemplate->addRequiredMetric(std::move(metric));
}

std::string Evolution::BasicModule::summary() const 
{
    return "todo";
}

void Evolution::BasicModule::addBasiclogger(std::ostream& output)
{
    auto logger = std::make_shared<Log::Basiclogger>(output);
    this->basicloggers.push_back(logger);
    this->loggers.push_back(logger);
}

void Evolution::BasicModule::initialize()
{
    this->population.clear();

    // Initialize population
    std::set<std::shared_ptr<Individual>, SharedLess<Individual>> individuals 
        = Mutation::initIndividuals(*this->mutation, this->representation, *this->generator, this->sizePopulation, this->rng);
    this->population.insert(individuals.begin(), individuals.end());
    individuals.clear();

    // Initial evaluation
    std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>> evaluationResults = 
            evaluator->evaluateIndividuals(this->population, this->problem, this->metricTemplate, this->nbTrainingIterations, UINT64_MAX, Evaluation::Mode::TRAINING);
    // Fitness assignment 
    this->individualFitness = this->assigner->assignFitness(evaluationResults, this->fitnessMetricHash);

    this->setBasicLoggerHeader();
}

void Evolution::BasicModule::setBasicLoggerHeader() {
    for(std::shared_ptr<Log::Basiclogger>& defLogger: this->basicloggers) {
        defLogger->addHeaderValues({"Gen", "Pop_Size", "Fit_Min", "Fit_Avg", "Fit_Max", "T_Gen", "T_Total"});
    }
}

void Evolution::BasicModule::doGeneration(size_t generationNumber) 
{

    // Parent selection 
    std::vector<std::shared_ptr<const Individual>> parents = this->parentSelection->select(this->individualFitness, this->nbOffspring, this->rng);


    // Reproduce the parents
    std::set<std::shared_ptr<Individual>, SharedLess<Individual>> offspring = this->breeder->reproduce(parents, this->nbOffspring, this->rng);


    // Mutate the offspring
    Mutation::mutateIndividuals(*this->mutation, offspring, *this->generator, this->rng);


    // Evaluate the population
    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> evaluatedIndividuals(this->population);
    evaluatedIndividuals.insert(offspring.begin(), offspring.end());
    std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>> evaluationResults = 
        this->evaluator->evaluateIndividuals(evaluatedIndividuals, this->problem, this->metricTemplate, this->nbTrainingIterations, generationNumber, Evaluation::Mode::TRAINING);


    // Do replacement
    this->individualFitness = this->assigner->assignFitness(evaluationResults, this->fitnessMetricHash);


    std::vector<std::shared_ptr<const Individual>> survivors = this->survivingSelection->select(individualFitness, this->sizePopulation, this->rng);
    this->population.clear();
    this->population.insert(survivors.begin(), survivors.end());

    this->setNewBasicLoggerRow(generationNumber);
}

void Evolution::BasicModule::setNewBasicLoggerRow(size_t generationNumber)
{
    this->stopwatch.checkpoint();
    for(std::shared_ptr<Log::Basiclogger>& defLogger: this->basicloggers) {
        defLogger->addGenerationValues<size_t>({generationNumber, this->population.size()});
    }

    double minFitness = std::numeric_limits<double>::max();
    double maxFitness = std::numeric_limits<double>::lowest();
    double averageFitness = 0.0;
    for (const auto& individual : this->population) {
        const double fitness = this->individualFitness.at(individual);
        minFitness = std::min(minFitness, fitness);
        maxFitness = std::max(maxFitness, fitness);
        averageFitness += fitness;
    }
    averageFitness /= this->population.size();

    double generationTime = this->stopwatch.getCheckpoints().at(0);
    for(std::shared_ptr<Log::Basiclogger>& defLogger: this->basicloggers) {
        defLogger->addGenerationValues<double>({minFitness, averageFitness, maxFitness, generationTime, this->stopwatch.elapsed()});
    }
}