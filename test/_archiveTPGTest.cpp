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
#include "evolution/basicmodule.h"
#include "evolution/originTPGModule.h"

#include "dimensions/activationFunctions.h"
#include "learn/mnist_reader/mnist_reader.hpp"

#include "metrics/predictionMetric.h"
#include "representations/LGP.h"
#include "representations/TPG.h"

#include "fitnessAssignment/classificationF1Assigner.h"

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
    Instructions::Set mnistSet;
    std::shared_ptr<Evaluation::ReinforcementEnvironment> le;


    virtual void SetUp()
    {
        CounterReset::counterReset();
        auto minus = [](double a, double b)->double {return a - b; };
        auto add = [](double a, double b)->double {return a + b; };
        auto mult = [](double a, double b)->double {return a * b; };
        auto div = [](double a, double b)->double {return a / b; };
        auto max = [](double a, double b)->double {return std::max(a, b); };
        auto ln = [](double a)->double {return std::log(a); };
        auto exp = [](double a)->double {return std::exp(a); };
        auto sobelMagn = [](const double a[3][3])->double {
            double result = 0.0;
            double gx =
                -a[0][0] + a[0][2]
                - 2.0 * a[1][0] + 2.0 * a[1][2]
                - a[2][0] + a[2][2];
            double gy = -a[0][0] - 2.0 * a[0][1] - a[0][2]
                + a[2][0] + 2.0 * a[2][1] + a[2][2];
            result = sqrt(gx * gx + gy * gy);
            return result;
        };

        auto sobelDir = [](const double a[3][3])->double {
            double result = 0.0;
            double gx =
                -a[0][0] + a[0][2]
                - 2.0 * a[1][0] + 2.0 * a[1][2]
                - a[2][0] + a[2][2];
            double gy = -a[0][0] - 2.0 * a[0][1] - a[0][2]
                + a[2][0] + 2.0 * a[2][1] + a[2][2];
            result = std::atan(gy / gx);
            return result;
        };

        
        mnistSet.add(*(new Instructions::LambdaInstruction<double, double, double>(minus)));
        mnistSet.add(*(new Instructions::LambdaInstruction<double, double, double>(add)));
        mnistSet.add(*(new Instructions::LambdaInstruction<double, double, double>(mult)));
        mnistSet.add(*(new Instructions::LambdaInstruction<double, double, double>(div)));
        mnistSet.add(*(new Instructions::LambdaInstruction<double, double, double>(max)));
        mnistSet.add(*(new Instructions::LambdaInstruction<double, double>(exp)));
        mnistSet.add(*(new Instructions::LambdaInstruction<double, double>(ln)));
        mnistSet.add(*(new Instructions::LambdaInstruction<double, const double[3][3]>(sobelMagn)));
        mnistSet.add(*(new Instructions::LambdaInstruction<double, const double[3][3]>(sobelDir)));
    
        le = std::make_shared<StickGameWithOpponent>();
    }

    virtual void TearDown()
    {
        delete (&mnistSet.getInstruction(0));
        delete (&mnistSet.getInstruction(1));
        delete (&mnistSet.getInstruction(2));
        delete (&mnistSet.getInstruction(3));
        delete (&mnistSet.getInstruction(4));
        delete (&mnistSet.getInstruction(5));
        delete (&mnistSet.getInstruction(6));
        delete (&mnistSet.getInstruction(7));
        delete (&mnistSet.getInstruction(8));
    }
};

TEST_F(TPGAlgoTest, mnistTestLGP) {
    mnist::MNIST_dataset<std::vector, std::vector<double>, uint8_t> mnist(mnist::read_dataset<std::vector, std::vector, double, uint8_t>(MNIST_DATA_LOCATION));

    std::vector<std::vector<double>> trainingImages = mnist.training_images;
    std::vector<uint8_t> trainingLabels = mnist.training_labels;

    Dimensions::Requirement inputReq = Dimensions::Requirement::array2d<double>(28, 28);
    Dimensions::Requirement outputReq = Dimensions::Requirement::scalar<size_t>();
    Evaluation::DataSet dataSet({inputReq}, outputReq);
    for(size_t idx = 0; idx < trainingImages.size(); idx++) {
        std::vector<Data::DataValue> inputs;
        inputs.push_back(Data::DataValue::array2d<std::vector<double>>(trainingImages.at(idx), 28, 28));
        
        dataSet.addData(std::move(inputs), Data::DataValue::scalar<uint64_t>(static_cast<uint64_t>(trainingLabels.at(idx))));
    }
    // Create evaluationAgent
    Evaluation::PredictionProblem problem(dataSet, 3);
    Evaluation::ParallelEvaluator evaluator(20);

    Instructions::Set mnistSet;
	auto minus = [](double a, double b)->double {return a - b; };
	auto add = [](double a, double b)->double {return a + b; };
	auto mult = [](double a, double b)->double {return a * b; };
	auto div = [](double a, double b)->double {return a / b; };
	auto max = [](double a, double b)->double {return std::max(a, b); };
	auto ln = [](double a)->double {return std::log(a); };
	auto exp = [](double a)->double {return std::exp(a); };
	auto sobelMagn = [](const double a[3][3])->double {
		double result = 0.0;
		double gx =
			-a[0][0] + a[0][2]
			- 2.0 * a[1][0] + 2.0 * a[1][2]
			- a[2][0] + a[2][2];
		double gy = -a[0][0] - 2.0 * a[0][1] - a[0][2]
			+ a[2][0] + 2.0 * a[2][1] + a[2][2];
		result = sqrt(gx * gx + gy * gy);
		return result;
	};

	auto sobelDir = [](const double a[3][3])->double {
		double result = 0.0;
		double gx =
			-a[0][0] + a[0][2]
			- 2.0 * a[1][0] + 2.0 * a[1][2]
			- a[2][0] + a[2][2];
		double gy = -a[0][0] - 2.0 * a[0][1] - a[0][2]
			+ a[2][0] + 2.0 * a[2][1] + a[2][2];
		result = std::atan(gy / gx);
		return result;
	};

    
	mnistSet.add(*(new Instructions::LambdaInstruction<double, double, double>(minus)));
	mnistSet.add(*(new Instructions::LambdaInstruction<double, double, double>(add)));
	mnistSet.add(*(new Instructions::LambdaInstruction<double, double, double>(mult)));
	mnistSet.add(*(new Instructions::LambdaInstruction<double, double, double>(div)));
	mnistSet.add(*(new Instructions::LambdaInstruction<double, double, double>(max)));
	mnistSet.add(*(new Instructions::LambdaInstruction<double, double>(exp)));
	mnistSet.add(*(new Instructions::LambdaInstruction<double, double>(ln)));
	mnistSet.add(*(new Instructions::LambdaInstruction<double, const double[3][3]>(sobelMagn)));
	mnistSet.add(*(new Instructions::LambdaInstruction<double, const double[3][3]>(sobelDir)));

    RNG::RNG rng;
    rng.setSeed(4);

    // Create representation
    Representations::LGP lgpRep(dataSet.getInputDimensions(), 1, mnistSet, 8, 1, 20);
    lgpRep.addOutputFunction(std::make_unique<Dimensions::ActivationFunctions::ArgMax<double>>(lgpRep.getDimensionFlow().getOutputDimension()));

    // Create Mutator and breeder
    Reproduction::Replicator breeder;
    Mutation::PointMutator mutator(0.5, 0.1, 0.1);

    // Create selectors
    Selection::RandomSelector parentSelection(true);
    Selection::TruncationSelector survivingSelection;
    FitnessAssignment::DefaultAssigner assigner;
    
    std::shared_ptr<Metrics::MetricMapTemplate> metrics 
        = std::make_shared<Metrics::MetricMapTemplate>(std::make_unique<Metrics::PredictionMetric>());


    size_t sizePopulation = 50;
    size_t nbOffspring = 450;

    // Initialize population
    std::set<std::shared_ptr<Individual>, SharedLess<Individual>> individuals = Mutation::initIndividuals(mutator, lgpRep, lgpRep.getGenotypeGenerator(), sizePopulation, rng);
    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> population(individuals.begin(), individuals.end());
    individuals.clear();

    // Initial evaluation
    std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>> evaluationResults = 
            evaluator.evaluateIndividuals(population, problem, metrics, 500, UINT64_MAX, Evaluation::Mode::TRAINING);
    // Fitness assignment 
    std::map<std::shared_ptr<const Individual>, double, SharedLess<Individual>> individualFitness = assigner.assignFitness(evaluationResults, Metrics::PredictionMetric::staticHash());

    size_t nbGen = 20;
    for (size_t idxGen = 0; idxGen < nbGen; idxGen++) {


        // Parent selection 
        std::vector<std::shared_ptr<const Individual>> parents = parentSelection.select(individualFitness, nbOffspring, rng);

        // Reproduce the parents
        std::set<std::shared_ptr<Individual>, SharedLess<Individual>> offspring = breeder.reproduce(parents, nbOffspring, rng);

        // Mutate the offspring
        Mutation::mutateIndividuals(mutator, offspring, lgpRep.getGenotypeGenerator(), rng);

        // Evaluate the population
        std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> evaluatedIndividuals(population);
        evaluatedIndividuals.insert(offspring.begin(), offspring.end());
        evaluationResults = evaluator.evaluateIndividuals(evaluatedIndividuals, problem, metrics, 500, idxGen, Evaluation::Mode::TRAINING);

        // Do replacement
        individualFitness = assigner.assignFitness(evaluationResults, Metrics::PredictionMetric::staticHash());
        std::vector<std::shared_ptr<const Individual>> survivors = survivingSelection.select(individualFitness, sizePopulation, rng);
        population.clear();
        population.insert(survivors.begin(), survivors.end());


        auto orderedFitness = Selection::Selector::getOrderedFitness(individualFitness, false);
        // Print best individual
        std::cout<<"At gen: "<<idxGen<<" - ID: "<<individualFitness.begin()->first->getIndividualID() 
        <<" and score: " << individualFitness.begin()->second <<std::endl;
    }

    ASSERT_EQ(13253408461481353746U, rng.uniformSample<uint64_t>(0, UINT64_MAX)) << "bouh bouh bouh th determinism";
}

TEST_F(TPGAlgoTest, customEvolutionLGP2) {
    mnist::MNIST_dataset<std::vector, std::vector<double>, uint8_t> mnist(mnist::read_dataset<std::vector, std::vector, double, uint8_t>(MNIST_DATA_LOCATION));

    std::vector<std::vector<double>> trainingImages = mnist.training_images;
    std::vector<uint8_t> trainingLabels = mnist.training_labels;

    Dimensions::Requirement inputReq = Dimensions::Requirement::array2d<double>(28, 28);
    Dimensions::Requirement outputReq = Dimensions::Requirement::scalar<size_t>();
    Evaluation::DataSet dataSet({inputReq}, outputReq);
    for(size_t idx = 0; idx < trainingImages.size(); idx++) {
        std::vector<Data::DataValue> inputs;
        inputs.push_back(Data::DataValue::array2d<std::vector<double>>(trainingImages.at(idx), 28, 28));
        
        dataSet.addData(std::move(inputs), Data::DataValue::scalar<uint64_t>(static_cast<uint64_t>(trainingLabels.at(idx))));
    }
    // Create evaluationAgent
    Evaluation::PredictionProblem problem(dataSet, 3);

    // Create representation
    Representations::LGP lgpRep(dataSet.getInputDimensions(), 10, mnistSet, 15, 1, 30);
    lgpRep.addOutputFunction(std::make_unique<Dimensions::ActivationFunctions::ArgMax<double>>(lgpRep.getDimensionFlow().getOutputDimension()));
    
    Evolution::BasicModule module(lgpRep, problem, 50, 450, 500, 4, 10);
    module.addBasiclogger();
    module.evolve(200);

    ASSERT_EQ(13253408461481353746U, module.rng.uniformSample<uint64_t>(0, UINT64_MAX)) << "bouh bouh bouh th determinism";
}


TEST_F(TPGAlgoTest, customEvolutionTPG2) {
    mnist::MNIST_dataset<std::vector, std::vector<double>, uint8_t> mnist(mnist::read_dataset<std::vector, std::vector, double, uint8_t>(MNIST_DATA_LOCATION));

    std::vector<std::vector<double>> trainingImages = mnist.training_images;
    std::vector<uint8_t> trainingLabels = mnist.training_labels;

    Dimensions::Requirement inputReq = Dimensions::Requirement::array2d<double>(28, 28);
    Dimensions::Requirement outputReq = Dimensions::Requirement::scalar<size_t>();
    Evaluation::DataSet dataSet({inputReq}, outputReq);
    for(size_t idx = 0; idx < trainingImages.size(); idx++) {
        std::vector<Data::DataValue> inputs;
        inputs.push_back(Data::DataValue::array2d<std::vector<double>>(trainingImages.at(idx), 28, 28));
        
        dataSet.addData(std::move(inputs), Data::DataValue::scalar<uint64_t>(static_cast<uint64_t>(trainingLabels.at(idx))));
    }
    // Create evaluationAgent
    Evaluation::PredictionProblem problem(dataSet, 3);


    // Create representation
    Representations::LGP lgpRep(problem.getInputDimensions(), 10, mnistSet, 15, 1, 30);
    Representations::TPG tpgRep(problem.getInputDimensions(), 10, 2, 10);
    
    auto assigner = std::make_shared<FitnessAssignment::ClassificationF1Assigner>();

    Evolution::OriginTPGModule module(tpgRep, lgpRep, problem, 20, 200, 1000, 4, 20, 50, true, assigner);
    module.addBasiclogger();
    module.evolve(50000);

    //ASSERT_EQ(13253408461481353746U, module.rng.uniformSample<uint64_t>(0, UINT64_MAX)) << "bouh bouh bouh th determinism";
}


#if 0
std::string printTracking(const std::vector<std::weak_ptr<const Individual>>& tracker, std::string name, bool full = false){

    size_t globalSize = tracker.size();
    size_t currentlyUsed = 0;
    std::map<size_t, size_t> counts;
    for(const std::weak_ptr<const Individual>& indiv: tracker) {
        if(!indiv.expired()) {
            size_t indivCount = indiv.use_count();
            currentlyUsed ++;
            if(counts.find(indivCount) != counts.end()) {
                counts.at(indivCount) += 1;
            } else {
                counts.insert(std::make_pair(indivCount, 1));
            }
        }
    }
    std::ostringstream ss;
    ss << "Tracking " << name << ":";
    ss << "Individual created: " << globalSize<< ", ";
    ss << "Individual used:" << currentlyUsed<< ",";
    if(full) {
        ss << "\n\t-counts: ";
        for(const auto& [count, occurence]: counts) {
            ss << count << "="<< occurence<< ", ";
        } ss << "\n\t-details: ";
        
        size_t i = 0;
        for(const std::weak_ptr<const Individual>& indiv: tracker) {
            if(!indiv.expired()) {
                size_t indivCount = indiv.use_count();
                ss <<"id("<< indiv.lock()->getIndividualID() << ")=" << indivCount << ", ";
                i++;
                if(i % 10 == 0) {
                    ss << "\n\t          ";
                }
            }
        }
    }
    else if(counts.size() < 6) {
        ss << "\n\t-counts: ";
        for(const auto& [count, occurence]: counts) {
            ss << count << "="<< occurence<< ", ";
        }
    } else {
        ss << "\n\t-Low counts: ";
        auto it = counts.begin();
        for(size_t idx = 0; idx < 3; idx++) {
            ss << it->first << "="<< it->second<< ", ";
            it++;
        } 
        ss << "\n\t-High counts: ";
        auto itEnd = counts.rbegin();
        for(size_t idx = 0; idx < 3; idx++) {
            ss << itEnd->first << "="<< itEnd->second<< ", ";
            itEnd++;
        } 
    }
    return ss.str();

}


TEST_F(TPGAlgoTest, mnistTestLGPPlusTPG) {
    mnist::MNIST_dataset<std::vector, std::vector<double>, uint8_t> mnist(mnist::read_dataset<std::vector, std::vector, double, uint8_t>(MNIST_DATA_LOCATION));

    std::vector<std::vector<double>> trainingImages = mnist.training_images;
    std::vector<uint8_t> trainingLabels = mnist.training_labels;

    Dimensions::Requirement inputReq = Dimensions::Requirement::array2d<double>(28, 28);
    Dimensions::Requirement outputReq = Dimensions::Requirement::scalar<size_t>();
    Evaluation::DataSet dataSet({inputReq}, outputReq);
    for(size_t idx = 0; idx < trainingImages.size(); idx++) {
        std::vector<Data::DataValue> inputs;
        inputs.push_back(Data::DataValue::array2d<std::vector<double>>(trainingImages.at(idx), 28, 28));
        
        dataSet.addData(std::move(inputs), Data::DataValue::scalar<uint64_t>(static_cast<uint64_t>(trainingLabels.at(idx))));
    }

    Instructions::Set mnistSet;
	auto minus = [](double a, double b)->double {return a - b; };
	auto add = [](double a, double b)->double {return a + b; };
	auto mult = [](double a, double b)->double {return a * b; };
	auto div = [](double a, double b)->double {return a / b; };
	auto max = [](double a, double b)->double {return std::max(a, b); };
	auto ln = [](double a)->double {return std::log(a); };
	auto exp = [](double a)->double {return std::exp(a); };
	auto sobelMagn = [](const double a[3][3])->double {
		double result = 0.0;
		double gx =
			-a[0][0] + a[0][2]
			- 2.0 * a[1][0] + 2.0 * a[1][2]
			- a[2][0] + a[2][2];
		double gy = -a[0][0] - 2.0 * a[0][1] - a[0][2]
			+ a[2][0] + 2.0 * a[2][1] + a[2][2];
		result = sqrt(gx * gx + gy * gy);
		return result;
	};

	auto sobelDir = [](const double a[3][3])->double {
		double result = 0.0;
		double gx =
			-a[0][0] + a[0][2]
			- 2.0 * a[1][0] + 2.0 * a[1][2]
			- a[2][0] + a[2][2];
		double gy = -a[0][0] - 2.0 * a[0][1] - a[0][2]
			+ a[2][0] + 2.0 * a[2][1] + a[2][2];
		result = std::atan(gy / gx);
		return result;
	};

    
	mnistSet.add(*(new Instructions::LambdaInstruction<double, double, double>(minus)));
	mnistSet.add(*(new Instructions::LambdaInstruction<double, double, double>(add)));
	mnistSet.add(*(new Instructions::LambdaInstruction<double, double, double>(mult)));
	mnistSet.add(*(new Instructions::LambdaInstruction<double, double, double>(div)));
	mnistSet.add(*(new Instructions::LambdaInstruction<double, double, double>(max)));
	mnistSet.add(*(new Instructions::LambdaInstruction<double, double>(exp)));
	mnistSet.add(*(new Instructions::LambdaInstruction<double, double>(ln)));
	//mnistSet.add(*(new Instructions::LambdaInstruction<double, const double[3][3]>(sobelMagn)));
	//mnistSet.add(*(new Instructions::LambdaInstruction<double, const double[3][3]>(sobelDir)));

    bool testDefinedARchive = false;
    
    RNG::RNG rng;
    rng.setSeed(1);

    /** TRACKING **/
    bool tracking = true;
    std::vector<std::weak_ptr<const Individual>> tpgIndivTracker;
    std::vector<std::weak_ptr<const Individual>> lgpIndivTracker;
    /** TRACKING **/

    size_t sizePopulation = 50;
    size_t nbOffspring = 450;
    size_t nbGen = 100;
    size_t nbImagesPerEval = 500;

    // Create evaluationAgent for TPG.
    Evaluation::PredictionProblem problem(dataSet, 3);

    // Create representation
    // Create representations
    Representations::LGP lgpRep(dataSet.getInputDimensions(), 1, mnistSet, 8, 1, 20);
    Representations::TPG* tpgRepPtr = new Representations::TPG(dataSet.getInputDimensions(), 10, 2, 5);
    Representations::TPG& tpgRep = *tpgRepPtr;

    // Create Mutator and breeder
    Reproduction::Replicator breeder;
    Mutation::PointMutator mutator(0.2, 0.7, 0.7);

    // Create selectors
    Selection::RandomSelector parentSelection(true);
    Selection::TruncationSelector survivingSelection;
    FitnessAssignment::DefaultAssigner assigner;
    
    Evaluation::ParallelEvaluator evaluator(20);
    std::shared_ptr<Metrics::MetricMapTemplate> metrics = std::make_shared<Metrics::MetricMapTemplate>();
    metrics->addRequiredMetric((std::make_unique<Metrics::PredictionMetric>()));
    if(testDefinedARchive) {
        metrics->addRequiredMetric((std::make_unique<Metrics::InputMetric>()));
    }

    // Create evaluationAgent and noveltySearch assigner for LGP .
    Evaluation::DataSet archiveDataSet(dataSet.getInputDimensions(), Dimensions::Requirement::scalar<double>());
    if(!testDefinedARchive) {
        for(size_t idx = 0; idx < 50; idx++) {
            uint64_t sampleIndex = rng.getUnsignedInt64(0, dataSet.size() - 1);
            const std::vector<Data::DataValue>& sampleInput = dataSet.getInputsAt(sampleIndex);
            const Data::DataValue& sampleOutput = dataSet.getOutputAt(sampleIndex);

            std::vector<Data::DataValue> copyInput;
            for(const Data::DataValue& input: sampleInput) {
                copyInput.push_back(input.clone());
            }
            archiveDataSet.addData(std::move(copyInput), Data::DataValue::scalar<double>(0.0));
        }
    }


    Evaluation::PredictionProblem archiveProblem(archiveDataSet);
    std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>> archiveResultsLGP;

    std::shared_ptr<Metrics::MetricMapTemplate> archiveMetrics = std::make_shared<Metrics::MetricMapTemplate>();
    archiveMetrics->addRequiredMetric((std::make_unique<TPGArchiveMetric>()));
    FitnessAssignment::NoveltySearchAssigner archiveNovelty(10, false);




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
            evaluator.evaluateIndividuals(populationTPG, problem, metrics, nbImagesPerEval, 0, Evaluation::Mode::TRAINING);
    // Fitness assignment 
    std::map<std::shared_ptr<const Individual>, double, SharedLess<Individual>> individualFitnessTPG = assigner.assignFitness(evaluationResultsTPG, Metrics::PredictionMetric::staticHash());
    
    // Update dataSet:
    if(testDefinedARchive) {
        updateArchiveDataSetValues(archiveDataSet, evaluationResultsTPG, rng);
    }
    archiveResultsLGP = evaluator.evaluateIndividuals(populationLGP, archiveProblem, archiveMetrics, archiveDataSet.size(), 0, Evaluation::Mode::TRAINING);
    // Fitness assignment 
    std::map<std::shared_ptr<const Individual>, double, SharedLess<Individual>> individualFitnessLGP 
        = archiveNovelty.assignFitness(archiveResultsLGP, TPGArchiveMetric::staticHash());

    
    /** TRACKING ALL INDIVS **/
    if(tracking) {
        for(auto indiv: populationLGP) {
            lgpIndivTracker.push_back(std::weak_ptr(indiv));
        }
        for(auto indiv: populationTPG) {
            tpgIndivTracker.push_back(std::weak_ptr(indiv));
        }
    }
    /** TRACKING ALL INDIVS **/

    double globalTime = 0.0;            
    double lgpGen = 0;
    double tpgGen = 0;
    double timeGen = 0;
    for (size_t idxGen = 0; idxGen < nbGen; idxGen++) {

        {
            std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> startGen = std::chrono::system_clock::now();

            // LGP Evolution : variation
            std::vector<std::shared_ptr<const Individual>> parentsLGP = parentSelection.select(individualFitnessLGP, nbOffspring, rng);
            std::set<std::shared_ptr<Individual>, SharedLess<Individual>> offspringLGP = breeder.reproduce(parentsLGP, nbOffspring, rng);

            /** TRACKING ALL LGPS **/
            if(tracking) {
                for(auto indiv: offspringLGP) {
                    lgpIndivTracker.push_back(std::weak_ptr(indiv));
                }
            }
            /** TRACKING ALL LGPS **/

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

            
            std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> midGen = std::chrono::system_clock::now();

            // TPG Evolution : variation
            std::vector<std::shared_ptr<const Individual>> parentsTPG = parentSelection.select(individualFitnessTPG, nbOffspring, rng);
            std::set<std::shared_ptr<Individual>, SharedLess<Individual>> offspringTPG = breeder.reproduce(parentsTPG, nbOffspring, rng);

            /** TRACKING ALL TPGS **/
            if(tracking) {
                for(auto indiv: offspringTPG) {
                    tpgIndivTracker.push_back(std::weak_ptr(indiv));
                }
            }
            /** TRACKING ALL TPGS **/
            
            members = parentSelection.select(individualFitnessLGP, nbOffspring, rng);
            tpgRep.setAvailableMembers(members);
            std::vector<std::shared_ptr<const Individual>> tangleds = parentSelection.select(individualFitnessTPG, nbOffspring, rng);
            tpgRep.setAvailableTangledIndiv(tangleds);

            Mutation::mutateIndividuals(mutator, offspringTPG, rng);
            // TPG Evolution : evaluation
            std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> evaluatedIndividualsTPG(populationTPG);
            evaluatedIndividualsTPG.insert(offspringTPG.begin(), offspringTPG.end());
            evaluationResultsTPG = evaluator.evaluateIndividuals(evaluatedIndividualsTPG, problem, metrics, nbImagesPerEval, idxGen, Evaluation::Mode::TRAINING);
            // TPG Evolution : replacement
            individualFitnessTPG = assigner.assignFitness(evaluationResultsTPG, Metrics::PredictionMetric::staticHash());
            std::vector<std::shared_ptr<const Individual>> survivorsTPG = survivingSelection.select(individualFitnessTPG, sizePopulation, rng);
            populationTPG.clear();
            populationTPG.insert(survivorsTPG.begin(), survivorsTPG.end());


            // Update dataSet:
            if(testDefinedARchive) {
                updateArchiveDataSetValues(archiveDataSet, evaluationResultsTPG, rng);
            }

            std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> endGen = std::chrono::system_clock::now();
            lgpGen = ((std::chrono::duration<double>)(midGen - startGen)).count();
            tpgGen = ((std::chrono::duration<double>)(endGen - midGen)).count();
            timeGen = ((std::chrono::duration<double>)(endGen - startGen)).count();
        }
        
        
        globalTime += timeGen;


        if( (idxGen % 10) == 0 || idxGen == (nbGen - 1)){
            // Print best individual
            auto orderedFitness = Selection::Selector::getOrderedFitness(individualFitnessTPG, false);
            std::cout<<"Gen "<<idxGen<<" lasted "<<timeGen<<" ("<<lgpGen<<","<<tpgGen<<") sec for a total of "<<globalTime<<" sec and achieved urrent accuracy Fitness of "<<orderedFitness.size()<<" individuals: \n";
            size_t idx = 0;
            for(const auto& [indiv, novelty]: orderedFitness) {
                std::cout<<" - at "<<idx<<": indiv "<<indiv->getIndividualID()<<": " << novelty<<";\n";
                idx++;
                if(idx >= 1) {
                    break;
                }
            }
            if(tracking) {
                std::cout<<printTracking(lgpIndivTracker, "LGP")<<std::endl;
                std::cout<<printTracking(tpgIndivTracker, "TPG")<<std::endl;
            }
            std::cout<<std::endl;
        }
    }

    std::cout<<"\n\nFinal tracking"<<std::endl;
    if(tracking) {
        std::cout<<printTracking(lgpIndivTracker, "LGP")<<std::endl;
        std::cout<<printTracking(tpgIndivTracker, "TPG")<<std::endl;
    }

    individualFitnessLGP.clear();
    archiveResultsLGP.clear();
    individualFitnessTPG.clear();
    evaluationResultsTPG.clear();
    individualsTPG.clear();
    members.clear();
    individualsLGP.clear();
    delete tpgRepPtr;

    std::cout<<"\n\nClearing all except Populations"<<std::endl;
    if(tracking) {
        std::cout<<printTracking(lgpIndivTracker, "LGP")<<std::endl;
        std::cout<<printTracking(tpgIndivTracker, "TPG")<<std::endl;
    }

    std::cout<<"\n\nClearing populations"<<std::endl;

    populationTPG.clear();
    populationLGP.clear();
    if(tracking) {
        std::cout<<printTracking(lgpIndivTracker, "LGP")<<std::endl;
        std::cout<<printTracking(tpgIndivTracker, "TPG")<<std::endl;
    }
}

#endif