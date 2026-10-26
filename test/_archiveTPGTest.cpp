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
#include "selection/tournamentSelector.h"
#include "selection/truncationSelector.h"
#include "fitnessAssignment/defaultAssigner.h"

#include "util/counterReset.h"
#include "file/populationDotExporter.h"

#include "instructions/imageInstructionSet.h"
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
        //mnistSet.add(*(new Instructions::LambdaInstruction<double, const double[3][3]>(sobelMagn)));
        //mnistSet.add(*(new Instructions::LambdaInstruction<double, const double[3][3]>(sobelDir)));
    
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

    Dimensions::Requirement inputReq = Dimensions::Requirement(Data::DataType::type<double>({28, 28}));
    Dimensions::Requirement outputReq = Dimensions::Requirement(Data::DataType::type<size_t>());
    Evaluation::DataSet dataSet({inputReq}, outputReq);
    for(size_t idx = 0; idx < trainingImages.size(); idx++) {
        std::vector<Data::DataValue> inputs;
        inputs.push_back(Data::DataValue::array<std::vector<double>>(trainingImages.at(idx), {28, 28}));
        
        dataSet.addData(std::move(inputs), Data::DataValue::scalar<uint64_t>(static_cast<uint64_t>(trainingLabels.at(idx))));
    }
    // Create evaluationAgent
    Evaluation::PredictionProblem problem(dataSet, 3);
    Evaluation::ParallelEvaluator evaluator(1);

    RNG::RNG rng;
    rng.setSeed(4);

    // Create representation
    Representations::LGP lgpRep(dataSet.getInputDimensions(), Data::DataType::type<double>({15}), mnistSet, Data::DataType::type<double>({10}), 1, 20);
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

    ASSERT_EQ(18013509966814137868U, rng.uniformSample<uint64_t>(0, UINT64_MAX)) << "bouh bouh bouh th determinism";
}

TEST_F(TPGAlgoTest, customEvolutionLGP2) {
    mnist::MNIST_dataset<std::vector, std::vector<double>, uint8_t> mnist(mnist::read_dataset<std::vector, std::vector, double, uint8_t>(MNIST_DATA_LOCATION));

    std::vector<std::vector<double>> trainingImages = mnist.training_images;
    std::vector<uint8_t> trainingLabels = mnist.training_labels;

    Dimensions::Requirement inputReq = Dimensions::Requirement(Data::DataType::type<double>({28, 28}));
    Dimensions::Requirement outputReq = Dimensions::Requirement(Data::DataType::type<size_t>());
    Evaluation::DataSet dataSet({inputReq}, outputReq);
    for(size_t idx = 0; idx < trainingImages.size(); idx++) {
        std::vector<Data::DataValue> inputs;
        inputs.push_back(Data::DataValue::array2d<std::vector<double>>(trainingImages.at(idx), 28, 28));
        
        dataSet.addData(std::move(inputs), Data::DataValue::scalar<uint64_t>(static_cast<uint64_t>(trainingLabels.at(idx))));
    }
    // Create evaluationAgent
    Evaluation::PredictionProblem problem(dataSet, 3);

    // Create representation
    Representations::LGP lgpRep(dataSet.getInputDimensions(), Data::DataType::type<double>({15}), mnistSet, Data::DataType::type<double>({10}), 1, 20);
    lgpRep.addOutputFunction(std::make_unique<Dimensions::ActivationFunctions::ArgMax<double>>(lgpRep.getDimensionFlow().getOutputDimension()));
    
    Evolution::BasicModule module(lgpRep, problem, 50, 450, 500, 4, 10);
    module.addBasiclogger();
    module.evolve(20);

    ASSERT_EQ(18013509966814137868U, module.rng.uniformSample<uint64_t>(0, UINT64_MAX)) << "bouh bouh bouh th determinism";
}

TEST_F(TPGAlgoTest, customEvolutionTPG2) {
    mnist::MNIST_dataset<std::vector, std::vector<double>, uint8_t> mnist(mnist::read_dataset<std::vector, std::vector, double, uint8_t>(MNIST_DATA_LOCATION));

    std::vector<std::vector<double>> trainingImages = mnist.training_images;
    std::vector<uint8_t> trainingLabels = mnist.training_labels;

    Dimensions::Requirement inputReq = Dimensions::Requirement(Data::DataType::type<double>({28, 28}));
    Dimensions::Requirement outputReq = Dimensions::Requirement(Data::DataType::type<size_t>());
    Evaluation::DataSet dataSet({inputReq}, outputReq);
    for(size_t idx = 0; idx < trainingImages.size(); idx++) {
        std::vector<Data::DataValue> inputs;
        inputs.push_back(Data::DataValue::array2d<std::vector<double>>(trainingImages.at(idx), 28, 28));
        
        dataSet.addData(std::move(inputs), Data::DataValue::scalar<uint64_t>(static_cast<uint64_t>(trainingLabels.at(idx))));
    }

    // Create evaluationAgent
    Evaluation::PredictionProblem problem(dataSet, 3);


    // Create representation
    Representations::LGP lgpRep(dataSet.getInputDimensions(), Data::DataType::type<double>({8}), mnistSet, Data::DataType::type<double>({1}), 1, 20);
    Representations::TPG tpgRep(problem.getInputDimensions(), 10, 2, 200);
    
    auto assigner = std::make_shared<FitnessAssignment::ClassificationF1Assigner>();
    auto parentSelector = std::make_shared<Selection::TournamentSelector>(2, true);

    Evolution::OriginTPGModule module(tpgRep, lgpRep, problem, 500, 1000, 500, 500, 4, 20, 50, true, false, assigner);
    module.addBasiclogger();

    std::string str = "graph";
    module.setExporter(MNIST_DATA_LOCATION + str, false);
    str = "graphBest";
    module.setBestExporter(MNIST_DATA_LOCATION + str, false);


    module.evolve(4000);

    EXPECT_EQ(15954241334055558952U, module.rng.uniformSample<uint64_t>(0, UINT64_MAX)) << "bouh bouh bouh th determinism";
    EXPECT_EQ(51691, Individual::getIndividualIDCounter()) << "bouh bouh bouh th determinism";
}

#ifdef GEGELATI_WITH_OPENCV

TEST_F(TPGAlgoTest, customEvolutionLGP2D) {
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

    auto instructions = Instructions::ImageInstructions::createAll<double>(28, 28);
    Instructions::Set imageSet;
    for(auto& intr: instructions) {
        imageSet.add(*intr);
    }

    std::vector<size_t> axes = {1, 2};

    // Create representation
    Representations::LGP lgpRep(dataSet.getInputDimensions(), Data::DataType::type<double>({10, 28, 28}), imageSet, Data::DataType::type<double>({10, 28, 28}), 1, 20);
    lgpRep.addOutputFunction(std::make_unique<Dimensions::ActivationFunctions::Sum<double>>(lgpRep.getDimensionFlow().getOutputDimension(), axes));
    lgpRep.addOutputFunction(std::make_unique<Dimensions::ActivationFunctions::ArgMax<double>>(lgpRep.getDimensionFlow().getOutputDimension()));
    std::cout<<lgpRep.summary()<<std::endl;
    
    Evolution::BasicModule module(lgpRep, problem, 50, 450, 500, 4, 20);
    module.addBasiclogger();
    std::string str = "graphLGP";
    module.setExporter(MNIST_DATA_LOCATION + str, false);
    str = "graphLGPBest";
    module.setBestExporter(MNIST_DATA_LOCATION + str, false);
    module.evolve(200);

    ASSERT_EQ(13253408461481353746U, module.rng.uniformSample<uint64_t>(0, UINT64_MAX)) << "bouh bouh bouh th determinism";
}



TEST_F(TPGAlgoTest, customEvolutionTPG3) {
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

    auto instructions = Instructions::ImageInstructions::createAll<double>(28, 28);
    Instructions::Set imageSet;
    for(auto& intr: instructions) {
        imageSet.add(*intr);
    }

    // Create evaluationAgent
    Evaluation::PredictionProblem problem(dataSet, 3);

    Representations::LGP lgpRep(dataSet.getInputDimensions(), Data::DataType::type<double>({8, 28, 28}), imageSet, Data::DataType::type<double>({1, 28, 28}), 1, 20);
    lgpRep.addOutputFunction(std::make_unique<Dimensions::ActivationFunctions::Sum<double>>(lgpRep.getDimensionFlow().getOutputDimension()));
    std::cout<<lgpRep.summary()<<std::endl;
    Representations::TPG tpgRep(problem.getInputDimensions(), 10, 2, 200);

    auto assigner = std::make_shared<FitnessAssignment::ClassificationF1Assigner>();

    Evolution::OriginTPGModule module(tpgRep, lgpRep, problem, 100, 400, 500, 4, 20, 0, true, false, assigner);
    module.addBasiclogger();

    std::string str = "graph";
    module.setExporter(MNIST_DATA_LOCATION + str, false);
    str = "graphBest";
    module.setBestExporter(MNIST_DATA_LOCATION + str, false);


    module.evolve(10000);

    ASSERT_EQ(17819704308639420184U, module.rng.uniformSample<uint64_t>(0, UINT64_MAX)) << "bouh bouh bouh th determinism";
    ASSERT_EQ(22492, Individual::getIndividualIDCounter()) << "bouh bouh bouh th determinism";
}
#endif