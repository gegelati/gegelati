#include "evolution/basicmodule.h"

#include <algorithm>
#include <chrono>
#include <limits>

Evolution::BasicModule::BasicModule(
    const Representations::Representation& representation, 
    Evaluation::Problem& problem,
    size_t sizePopulation, size_t nbOffspring, size_t nbTrainingIterations, size_t nbValidationIterations, uint64_t seed, size_t nbThreads, 
    std::shared_ptr<FitnessAssignment::Assigner> assigner,
    std::shared_ptr<Selection::Selector> parentSelection ,
    std::shared_ptr<Selection::Selector> survivingSelection,
    std::shared_ptr<Reproduction::Reproducer> breeder,
    std::shared_ptr<Mutation::Mutator> mutation
): Module(), representation{representation}, problem{problem}, sizePopulation{sizePopulation}, 
nbOffspring{nbOffspring}, nbTrainingIterations{nbTrainingIterations}, nbValidationIterations{nbValidationIterations}, seed{seed}, nbThreads{nbThreads},
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
        this->mutation = std::make_shared<Mutation::PointMutator>(0.2, 0.5, 0.5);
    }

    this->generator = this->representation.getGenotypeGenerator().cloneUniquePtr();
    this->rng.setSeed(seed);
    this->initializeMetricTemplate();
} 

void Evolution::BasicModule::setExporter(std::string filePath, bool oneFileGen)
{
    this->filePath = filePath;
    this->oneFileGen = oneFileGen;
}

void Evolution::BasicModule::setBestExporter(std::string filePathBest, bool oneFileGenBest)
{
    this->filePathBest = filePathBest;
    this->oneFileGenBest = oneFileGenBest;
}

std::vector<std::shared_ptr<const Individual>> Evolution::BasicModule::getPopulation() const
{
    std::vector<std::shared_ptr<const Individual>> currentPopulation;
    currentPopulation.insert(currentPopulation.begin(), this->population.begin(), this->population.end());
    return std::move(currentPopulation);
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
    
    // Update metric of population
    this->populationMetrics.clear();
    for(const std::shared_ptr<const Individual>& indiv: this->population) {
        this->populationMetrics.insert(std::make_pair(indiv->getIndividualID(), std::move(evaluationResults.at(indiv))));
    }

    this->setBasicLoggerHeader();
}

void Evolution::BasicModule::setBasicLoggerHeader() {
    for(std::shared_ptr<Log::Basiclogger>& defLogger: this->basicloggers) {
        if(this->nbValidationIterations == 0) {
            defLogger->addHeaderValues({"Gen", "Pop_Size", "Fit_Min", "Fit_Avg", "Fit_Max", "T_Mut", "T_Eval", "T_Gen", "T_Total"});
        } else {
            defLogger->addHeaderValues({"Gen", "Pop_Size", "Fit_Min", "Fit_Avg", "Fit_Max", "Val_Min", "Val_Avg", "Val_Max", "T_Mut", "T_Eval", "T_Val", "T_Gen", "T_Total"});
        }
    }
}

void Evolution::BasicModule::doGeneration(size_t generationNumber) 
{

    this->stopwatch.checkpoint();
    // Parent selection 
    std::vector<std::shared_ptr<const Individual>> parents = this->parentSelection->select(this->individualFitness, this->nbOffspring, this->rng);

    this->stopwatch.checkpoint();


    // Reproduce the parents
    std::set<std::shared_ptr<Individual>, SharedLess<Individual>> offspring = this->breeder->reproduce(parents, this->nbOffspring, this->rng);

    this->stopwatch.checkpoint();

    // Mutate the offspring
    Mutation::mutateIndividualsInParallel(*this->mutation, offspring, *this->generator, this->rng, this->nbThreads);

    this->stopwatch.checkpoint();

    // Evaluate the population
    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> evaluatedIndividuals(this->population);
    evaluatedIndividuals.insert(offspring.begin(), offspring.end());
    std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>> evaluationResults = 
        this->evaluator->evaluateIndividuals(evaluatedIndividuals, this->problem, this->metricTemplate, this->nbTrainingIterations, generationNumber, Evaluation::Mode::TRAINING);

        
    this->stopwatch.checkpoint();

    // Do replacement
    this->individualFitness = this->assigner->assignFitness(evaluationResults, this->fitnessMetricHash);

    
    this->stopwatch.checkpoint();

    std::vector<std::shared_ptr<const Individual>> survivors = this->survivingSelection->select(individualFitness, this->sizePopulation, this->rng);

    // Update population
    this->population.clear();
    this->population.insert(survivors.begin(), survivors.end());

    // Update metric of population
    this->populationMetrics.clear();
    for(const std::shared_ptr<const Individual>& indiv: this->population) {
        this->populationMetrics.insert(std::make_pair(indiv->getIndividualID(), std::move(evaluationResults.at(indiv))));
    }

    this->stopwatch.checkpoint();
    if(this->nbValidationIterations > 0) {
        evaluationResults = this->evaluator->evaluateIndividuals(this->population, this->problem, this->metricTemplate, this->nbValidationIterations, generationNumber, Evaluation::Mode::VALIDATION);
        this->validationFitness = this->assigner->assignFitness(evaluationResults, this->fitnessMetricHash);
        this->stopwatch.checkpoint();
    }


    this->setNewBasicLoggerRow(generationNumber);

    if(this->filePath != "") {
        std::string path = this->filePath + ((this->oneFileGen) ? std::to_string(generationNumber) : "") + ".dot";
        File::PopulationDotExporter::exportToFile(path, this->getPopulation());
    }
    if(this->filePathBest != "") {
        std::string path = this->filePathBest + ((this->oneFileGenBest) ? std::to_string(generationNumber) : "") + ".dot";
        File::PopulationDotExporter::exportToFile(path, {this->individualFitness.begin()->first});
    }
}

void Evolution::BasicModule::setNewBasicLoggerRow(size_t generationNumber)
{
    for(std::shared_ptr<Log::Basiclogger>& defLogger: this->basicloggers) {
        defLogger->addGenerationValues<size_t>({generationNumber, this->population.size()});
    }

    double minFitness = std::numeric_limits<double>::max();
    double maxFitness = std::numeric_limits<double>::lowest();
    double averageFitness = 0.0;

    
    double minFitnessVal = std::numeric_limits<double>::max();
    double maxFitnessVal = std::numeric_limits<double>::lowest();
    double averageFitnessVal = 0.0;
    for (const auto& individual : this->population) {
        const double fitness = this->individualFitness.at(individual);
        minFitness = std::min(minFitness, fitness);
        maxFitness = std::max(maxFitness, fitness);
        averageFitness += fitness;
        
        if(this->nbValidationIterations > 0) {
            const double valFitness = this->validationFitness.at(individual);
            minFitnessVal = std::min(minFitnessVal, valFitness);
            maxFitnessVal = std::max(maxFitnessVal, valFitness);
            averageFitnessVal += valFitness;
        }
    }
    averageFitness /= this->population.size();
    averageFitnessVal /= this->population.size();


    std::vector<double> genCheckpoints = this->stopwatch.getCheckpoints();
    double mutTime = genCheckpoints.at(3);
    double evalTime = genCheckpoints.at(4);
    double generationTime = std::accumulate(genCheckpoints.begin(), genCheckpoints.end(), 0.0);
    if(this->nbValidationIterations == 0) {
        for(std::shared_ptr<Log::Basiclogger>& defLogger: this->basicloggers) {
            defLogger->addGenerationValues<double>({minFitness, averageFitness, maxFitness, mutTime, evalTime, generationTime, this->stopwatch.elapsed()});
        }
    } else {
        double validationTime = genCheckpoints.at(7);
        for(std::shared_ptr<Log::Basiclogger>& defLogger: this->basicloggers) {
            defLogger->addGenerationValues<double>({minFitness, averageFitness, maxFitness, minFitnessVal, averageFitnessVal, maxFitnessVal, mutTime, evalTime, validationTime, generationTime, this->stopwatch.elapsed()});
        }
    }
}