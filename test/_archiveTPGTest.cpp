#include <algorithm>
#include <fstream>
#include <gtest/gtest.h>
#include <numeric>

#include "instructions/set.h"
#include "instructions/lambdaInstruction.h"
#include "learn/stickGameWithOpponent.h"
#include "evaluation/reinforcementProblem.h"
#include "evaluation/evaluator.h"
#include "evaluation/parallelEvaluator.h"
#include "metrics/rewardsMetric.h"
#include "metrics/inputMetric.h"
#include "metrics/outputMetric.h"

#include "evaluation/dataset.h"
#include "evaluation/predictionProblem.h"
#include "fitnessAssignment/noveltySearchAssigner.h"

#include "representations/LGP.h"
#include "representations/TPG.h"

#include "mutation/pointMutator.h"
#include "reproduction/replicator.h"
#include "selection/randomSelector.h"
#include "selection/truncationSelector.h"
#include "fitnessAssignment/defaultAssigner.h"

#include "util/counterReset.h"
// Set all file in comment

class TPGAlgoTest : public ::testing::Test
{
  protected:
    Instructions::Set set;
    std::shared_ptr<Evaluation::ReinforcementEnvironment> le;


    virtual void SetUp()
    {
        CounterReset::counterReset();
        auto add = [](double a, double b) -> double { return a + b; };
        auto minus = [](int a, double b) -> double { return a - b; };
        auto times = [](double a, int b) -> double { return a * b; };
        auto div = [](int a, double b) -> double { return a / b; };
        
        set.add(*(new Instructions::LambdaInstruction<double, double, double>(add)));
        set.add(*(new Instructions::LambdaInstruction<double, int, double>(minus)));
        set.add(*(new Instructions::LambdaInstruction<double, double, int>(times)));
        set.add(*(new Instructions::LambdaInstruction<double, int, double>(div)));
    
        le = std::make_shared<StickGameWithOpponent>();
    }

    virtual void TearDown()
    {
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
        delete (&set.getInstruction(2));
        delete (&set.getInstruction(3));
    }
};


void updateArchiveDataSetValues(
    Evaluation::DataSet& dataset, 
    const std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>>& evaluationResults, 
    RNG::RNG& rng)
{

    // Empty data set
    while(dataset.size() > 0) {
        dataset.removeData(0);
    }

    // Map of future metrics
    std::map<uint64_t, std::vector<Data::DataValue>> allInputMetrics;

    // For each individual evaluation
    for(const auto& [indiv, metricMap]: evaluationResults) {

        // Get the input metrics
        std::map<uint64_t, const Metrics::InputMetric*> inputMetrics = metricMap->getMetricValues<Metrics::InputMetric>(Metrics::InputMetric::staticHash());

        // For each feature, get the input metric
        for(const auto& [featureHash, inputMetric]: inputMetrics) {

            // For each step in the evaluation feature hash, get the inputs
            for(const std::vector<Data::DataValue>& inputsMetricStep: inputMetric->getInputs()) {

                // Get combined hash of the inputs
                uint64_t currentInputHash = Data::DataValue::getCombinedHash(inputsMetricStep);

                // Inputs are new, dupplicate them and add them to the allInputMetrics Map!
                if(allInputMetrics.find(currentInputHash) == allInputMetrics.end()) {
                    std::vector<Data::DataValue> inputMetricStepClone;
                    for(const Data::DataValue& input: inputsMetricStep) {
                        inputMetricStepClone.push_back(input.clone());
                    }
                    allInputMetrics.insert(std::make_pair(currentInputHash, std::move(inputMetricStepClone)));
                }
            }
        }
    }

    size_t dataSetMaxSize = 50;
    if(allInputMetrics.size() > dataSetMaxSize) {
        while(dataset.size() != dataSetMaxSize) {

            auto it = allInputMetrics.begin();
            std::advance(it, rng.uniformSample<uint64_t>(0, allInputMetrics.size() - 1));

            dataset.addData(std::move(it->second), Data::DataValue::scalar<double>(0.0));
            allInputMetrics.erase(it);
        }
    } else {
        for(auto& [hash, inputs]: allInputMetrics) {
            dataset.addData(std::move(inputs), Data::DataValue::scalar<double>(0.0));
        }
    }
}

class TPGArchiveMetric: public Metrics::ScalarMetric {
    public:
    TPGArchiveMetric(): Metrics::ScalarMetric() {};
    std::unique_ptr<Metric> cloneEmptyPtr() const override {return std::make_unique<TPGArchiveMetric>();};
    void extractAfterExecution(
            const uint64_t& hash, const Individual& individual, 
            const Data::DataValue& output, const Evaluation::Problem& problem) override {
        this->values[0] = std::min(std::max(output.getScalar<double>(), -10000000.0), 10000000.0);
    }
    std::string toString(std::string prefix = "") const override {
        std::stringstream ss;
        ss << prefix << "OutputMetric with "<< this->values[0] << " value extracted: \n";
        return ss.str();
    }
    static uint64_t staticHash() {return std::type_index(typeid(TPGArchiveMetric)).hash_code();}
};


TEST_F(TPGAlgoTest, customEvolutionLGP) {
    RNG::RNG rng;
    rng.setSeed(4);


    Evaluation::DataSet archiveDataSet(le->getInputDimensions(), Dimensions::Requirement::scalar<double>());
    Evaluation::PredictionProblem archiveProblem(archiveDataSet);
    std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>> archiveResults;

    std::shared_ptr<Metrics::MetricMapTemplate> archiveMetrics = std::make_shared<Metrics::MetricMapTemplate>();
    archiveMetrics->addRequiredMetric((std::make_unique<TPGArchiveMetric>()));
    FitnessAssignment::NoveltySearchAssigner archiveNovelty(20, false);


    // Create representation
    Representations::LGP lgpRep(le->getInputDimensions(), 1, set, 8, 10);

    // Create Mutator and breeder
    Reproduction::Replicator breeder;
    Mutation::PointMutator mutator(0.5, 0.1, 0.1);

    // Create selectors
    Selection::RandomSelector parentSelection(true);
    Selection::TruncationSelector survivingSelection;
    FitnessAssignment::DefaultAssigner assigner;
    
    // Create evaluationAgent
    Evaluation::ReinforcementProblem problem(le, 3);
    Evaluation::Evaluator evaluator;
    std::shared_ptr<Metrics::MetricMapTemplate> metrics = std::make_shared<Metrics::MetricMapTemplate>();
    metrics->addRequiredMetric((std::make_unique<Metrics::RewardsMetric>()));
    metrics->addRequiredMetric((std::make_unique<Metrics::InputMetric>()));
    metrics->addRequiredMetric((std::make_unique<Metrics::OutputMetric>()));

    size_t sizePopulation = 100;
    size_t nbOffspring = 100;

    // Initialize population
    std::set<std::shared_ptr<Individual>, SharedLess<Individual>> individuals = Mutation::initIndividuals(mutator, lgpRep, sizePopulation, rng);
    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> population(individuals.begin(), individuals.end());
    individuals.clear();

    // Initial evaluation
    std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>> evaluationResults = 
            evaluator.evaluateIndividuals(population, problem, metrics, 3, 0, Evaluation::Mode::TRAINING);
    // Fitness assignment 
    std::map<std::shared_ptr<const Individual>, double, SharedLess<Individual>> individualFitness = assigner.assignFitness(evaluationResults, Metrics::RewardsMetric::staticHash());

    size_t nbGen = 20;
    for (size_t idxGen = 0; idxGen < nbGen; idxGen++) {

        // Parent selection 
        std::vector<std::shared_ptr<const Individual>> parents = parentSelection.select(individualFitness, nbOffspring, rng);

        // Reproduce the parents
        std::set<std::shared_ptr<Individual>, SharedLess<Individual>> offspring = breeder.reproduce(parents, nbOffspring, rng);

        // Mutate the offspring
        Mutation::mutateIndividuals(mutator, offspring, rng);

        // Evaluate the population
        std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> evaluatedIndividuals(population);
        evaluatedIndividuals.insert(offspring.begin(), offspring.end());
        evaluationResults = evaluator.evaluateIndividuals(evaluatedIndividuals, problem, metrics, 3, idxGen, Evaluation::Mode::TRAINING);

        // Do replacement
        individualFitness = assigner.assignFitness(evaluationResults, Metrics::RewardsMetric::staticHash());
        std::vector<std::shared_ptr<const Individual>> survivors = survivingSelection.select(individualFitness, sizePopulation, rng);
        population.clear();
        population.insert(survivors.begin(), survivors.end());


        // Update dataSet:
        updateArchiveDataSetValues(archiveDataSet, evaluationResults, rng);
        //std::cout<<"DataSet at generation "<<idxGen<<"\n"<<archiveDataSet.toString(50)<<std::endl;

        // Evaluate individaul
        archiveResults = evaluator.evaluateIndividuals(population, archiveProblem, archiveMetrics, 19, idxGen, Evaluation::Mode::TRAINING);
        //std::cout<<"\n\nSample of first individual metric:\n"<<archiveResults.begin()->second->toString()<<std::endl;
        std::map<std::shared_ptr<const Individual>, double, SharedLess<Individual>> archiveFitness 
            = archiveNovelty.assignFitness(archiveResults, TPGArchiveMetric::staticHash());

        /*std::cout<<"Current Novelty Fitness of "<<archiveResults.size()<<" individuals: \n";
        auto orderedFitness = Selection::Selector::getOrderedFitness(archiveFitness, false);
        for(const auto& [indiv, novelty]: orderedFitness) {
            std::cout<<" - indiv "<<indiv->getIndividualID()<<": " << novelty<<";\n";
        }
        std::cout<<std::endl;*/
    }

    //std::cout<<*evaluationResults.begin()->second<<std::endl;
}



TEST_F(TPGAlgoTest, customEvolutionTPGPlusLGP) {
    RNG::RNG rng;
    rng.setSeed(4);

    // Create representations
    Representations::LGP lgpRep(le->getInputDimensions(), 1, set, 8, 10);
    Representations::TPG tpgRep(le->getInputDimensions(), 3, 2, 10);

    // Create Mutator and breeder
    Reproduction::Replicator breeder;
    Mutation::PointMutator mutator(0.5, 0.1, 0.1);

    // Create selectors
    Selection::RandomSelector parentSelection(true);
    Selection::TruncationSelector survivingSelection;
    FitnessAssignment::DefaultAssigner assigner;
    
    // Create evaluationAgent for TPG.
    Evaluation::ReinforcementProblem problem(le, 3);
    Evaluation::ParallelEvaluator evaluator(16);
    std::shared_ptr<Metrics::MetricMapTemplate> metrics = std::make_shared<Metrics::MetricMapTemplate>();
    metrics->addRequiredMetric((std::make_unique<Metrics::RewardsMetric>()));
    metrics->addRequiredMetric((std::make_unique<Metrics::InputMetric>()));

    // Create evaluationAgent and noveltySearch assigner for LGP .
    Evaluation::DataSet archiveDataSet(le->getInputDimensions(), Dimensions::Requirement::scalar<double>());
    Evaluation::PredictionProblem archiveProblem(archiveDataSet);
    std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>> archiveResultsLGP;

    std::shared_ptr<Metrics::MetricMapTemplate> archiveMetrics = std::make_shared<Metrics::MetricMapTemplate>();
    archiveMetrics->addRequiredMetric((std::make_unique<TPGArchiveMetric>()));
    FitnessAssignment::NoveltySearchAssigner archiveNovelty(10, false);



    size_t sizePopulation = 100;
    size_t nbOffspring = 100;

    // Initialize LGP population
    std::set<std::shared_ptr<Individual>, SharedLess<Individual>> individualsLGP = Mutation::initIndividuals(mutator, lgpRep, sizePopulation, rng);
    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> populationLGP(individualsLGP.begin(), individualsLGP.end());
    individualsLGP.clear();


    // Initialize TPG population
    std::vector<std::shared_ptr<const Individual>> members(populationLGP.begin(), populationLGP.end());
    tpgRep.setAvailableMembers(members);

    std::set<std::shared_ptr<Individual>, SharedLess<Individual>> individualsTPG = Mutation::initIndividuals(mutator, tpgRep, sizePopulation, rng);
    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> populationTPG(individualsTPG.begin(), individualsTPG.end());
    individualsTPG.clear();
    std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>> evaluationResultsTPG = 
            evaluator.evaluateIndividuals(populationTPG, problem, metrics, 3, 0, Evaluation::Mode::TRAINING);
    // Fitness assignment 
    std::map<std::shared_ptr<const Individual>, double, SharedLess<Individual>> individualFitnessTPG = assigner.assignFitness(evaluationResultsTPG, Metrics::RewardsMetric::staticHash());
    
    // Update dataSet:
    updateArchiveDataSetValues(archiveDataSet, evaluationResultsTPG, rng);
    archiveResultsLGP = evaluator.evaluateIndividuals(populationLGP, archiveProblem, archiveMetrics, archiveDataSet.size(), 0, Evaluation::Mode::TRAINING);
    // Fitness assignment 
    std::map<std::shared_ptr<const Individual>, double, SharedLess<Individual>> individualFitnessLGP 
        = archiveNovelty.assignFitness(archiveResultsLGP, TPGArchiveMetric::staticHash());

    size_t nbGen = 20;
    for (size_t idxGen = 0; idxGen < nbGen; idxGen++) {

        // LGP Evolution : variation
        std::vector<std::shared_ptr<const Individual>> parentsLGP = parentSelection.select(individualFitnessLGP, nbOffspring, rng);
        std::set<std::shared_ptr<Individual>, SharedLess<Individual>> offspringLGP = breeder.reproduce(parentsLGP, nbOffspring, rng);
        Mutation::mutateIndividuals(mutator, offspringLGP, rng);
        // LGP Evolution : evaluation
        std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> evaluatedIndividualsLGP(populationLGP);
        evaluatedIndividualsLGP.insert(offspringLGP.begin(), offspringLGP.end());
        archiveResultsLGP = evaluator.evaluateIndividuals(evaluatedIndividualsLGP, archiveProblem, archiveMetrics, archiveDataSet.size(), idxGen, Evaluation::Mode::TRAINING);
        // LGP Evolution : replacement
        individualFitnessLGP = archiveNovelty.assignFitness(archiveResultsLGP, TPGArchiveMetric::staticHash());
        std::vector<std::shared_ptr<const Individual>> survivorsLGP = survivingSelection.select(individualFitnessLGP, sizePopulation, rng);
        populationLGP.clear();
        populationLGP.insert(survivorsLGP.begin(), survivorsLGP.end());


        /*auto orderedFitness = Selection::Selector::getOrderedFitness(individualFitnessLGP, false);
        std::cout<<"Current Novelty Fitness of "<<orderedFitness.size()<<" individuals: \n";
        size_t idx = 0;
        for(const auto& [indiv, novelty]: orderedFitness) {
            std::cout<<" - at "<<idx<<": indiv "<<indiv->getIndividualID()<<": " << novelty<<";\n";
            idx++;
        }
        std::cout<<std::endl;*/

        // TPG Evolution : variation
        std::vector<std::shared_ptr<const Individual>> parentsTPG = parentSelection.select(individualFitnessTPG, nbOffspring, rng);
        std::set<std::shared_ptr<Individual>, SharedLess<Individual>> offspringTPG = breeder.reproduce(parentsTPG, nbOffspring, rng);

        
        std::vector<std::shared_ptr<const Individual>> members = parentSelection.select(individualFitnessLGP, nbOffspring, rng);
        std::vector<std::shared_ptr<const Individual>> tangleds = parentSelection.select(individualFitnessTPG, nbOffspring, rng);
        tpgRep.setAvailableMembers(members);
        tpgRep.setAvailableTangledIndiv(tangleds);

        Mutation::mutateIndividuals(mutator, offspringTPG, rng);
        // TPG Evolution : evaluation
        std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> evaluatedIndividualsTPG(populationTPG);
        evaluatedIndividualsTPG.insert(offspringTPG.begin(), offspringTPG.end());
        evaluationResultsTPG = evaluator.evaluateIndividuals(evaluatedIndividualsTPG, problem, metrics, 3, idxGen, Evaluation::Mode::TRAINING);
        // TPG Evolution : replacement
        individualFitnessTPG = assigner.assignFitness(evaluationResultsTPG, Metrics::RewardsMetric::staticHash());
        std::vector<std::shared_ptr<const Individual>> survivorsTPG = survivingSelection.select(individualFitnessTPG, sizePopulation, rng);
        populationTPG.clear();
        populationTPG.insert(survivorsTPG.begin(), survivorsTPG.end());


        // Print best individual
        std::cout<<"ID: "<<individualFitnessTPG.begin()->first->getIndividualID() 
        <<" and score: " << individualFitnessTPG.begin()->second <<std::endl;
    }

    ASSERT_EQ(8847042229433144007U, rng.uniformSample<uint64_t>(0, UINT64_MAX)) << "bouh bouh bouh th determinism";
}