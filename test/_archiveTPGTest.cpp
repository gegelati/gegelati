#include <algorithm>
#include <fstream>
#include <gtest/gtest.h>
#include "evolutionTestHelpers.h"
#include "util/individualUtils.h"
#include <numeric>

#include "instructions/set.h"
#include "instructions/lambdaInstruction.h"
#include "learn/stickGameWithOpponent.h"
#include "evaluation/reinforcementProblem.h"
#include "evaluation/evaluate.h"
#include "metrics/rewardsMetric.h"
#include "metrics/inputMetric.h"
#include "metrics/outputMetric.h"

#include "evaluation/dataset.h"
#include "evaluation/predictionProblem.h"
#include "fitnessAssignment/noveltySearchAssigner.h"
#include "evolution/originTPGEvolutionConfig.h"

#include "dimensions/activationFunctions.h"
#include "learn/mnist_reader/mnist_reader.hpp"

#include "metrics/predictionMetric.h"
#include "representations/LGP.h"
#include "representations/TPG.h"

#include "fitnessAssignment/classificationF1Assigner.h"
#include "evolution/basicEvolutionConfig.h"
#include "evolution/steps/exportStep.h"
#include "evolution/originTPGEvolutionConfig.h"
#include "evolution/steps/logStep.h"
#include "evolution/steps/createGeneratorStep.h"
#include "evolution/steps/lambdaStep.h"
#include "evolution/steps/tpgSteps.h"

#include "mutation/pointMutator.h"
#include "reproduction/replicator.h"
#include "selection/randomSelector.h"
#include "selection/tournamentSelector.h"
#include "selection/truncationSelector.h"
#include "fitnessAssignment/defaultAssigner.h"

#include "evolution/steps/mutateStep.h"

#include "util/counterReset.h"
#include "file/populationDotExporter.h"

namespace {
}
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
    auto problem = std::make_shared<Evaluation::PredictionProblem>(dataSet, 3);

    // Create representation
    auto lgpRep = std::make_shared<Representations::LGP>(dataSet.getInputDimensions(), Data::DataType::type<double>({15}), mnistSet, Data::DataType::type<double>({10}), 1, 20);
    lgpRep->addOutputFunction(std::make_unique<Dimensions::ActivationFunctions::ArgMax<double>>(lgpRep->getDimensionFlow().getOutputDimension()));
    
    Evolution::BasicEvolutionConfig config;
    config.sizePopulation = 50;
    config.nbOffspring = 450;
    config.nbTrainingIterations = 500;
    config.nbThreads = 10;
    config.representation = lgpRep;
    config.problem = problem;
    std::shared_ptr<Evolution::Module> evolutionModule = config.buildModule();

    auto mutator = std::make_shared<Mutation::PointMutator>(0.5, 0.5, 0.5);
    auto newMutateStep = std::make_shared<Evolution::MutateStep>(config.names.offspring, mutator, config.names.generator, 10, "Mutation2");
    evolutionModule->insertAfter(evolutionModule->find("Mutation"), newMutateStep);
    //evolutionModule->replace(evolutionModule->find("Mutation"), newMutateStep);*/


    std::cout<<evolutionModule->summary()<<std::endl;
    Evolution::EvolutionData data = evolutionModule->evolve(20, 4);
    std::cout<<data.summary()<<std::endl;


    ASSERT_EQ(18013509966814137868U, data.rng.uniformSample<uint64_t>(0, UINT64_MAX)) << "bouh bouh bouh th determinism";
}


TEST_F(TPGAlgoTest, customEvolutionTPG2) {
    mnist::MNIST_dataset<std::vector, std::vector<double>, uint8_t> mnist(mnist::read_dataset<std::vector, std::vector, double, uint8_t>(MNIST_DATA_LOCATION));
    Dimensions::Requirement inputReq = Dimensions::Requirement(Data::DataType::type<double>({28, 28}));
    Dimensions::Requirement outputReq = Dimensions::Requirement(Data::DataType::type<size_t>());
    Evaluation::DataSet dataSet({inputReq}, outputReq);

    std::vector<std::vector<double>> trainingImages = mnist.training_images;
    std::vector<uint8_t> trainingLabels = mnist.training_labels;
    for(size_t idx = 0; idx < trainingImages.size(); idx++) {
        std::vector<Data::DataValue> inputs;
        inputs.push_back(Data::DataValue::array2d<std::vector<double>>(trainingImages.at(idx), 28, 28));
        
        dataSet.addData(std::move(inputs), Data::DataValue::scalar<uint64_t>(static_cast<uint64_t>(trainingLabels.at(idx))));
    }

    std::vector<std::vector<double>> testImages = mnist.test_images;
    std::vector<uint8_t> testLabels = mnist.test_labels;
    for(size_t idx = 0; idx < testImages.size(); idx++) {
        std::vector<Data::DataValue> inputs;
        inputs.push_back(Data::DataValue::array2d<std::vector<double>>(testImages.at(idx), 28, 28));
        
        dataSet.addData(std::move(inputs), Data::DataValue::scalar<uint64_t>(static_cast<uint64_t>(testLabels.at(idx))), false);
    }

    // Create evaluationAgent
    auto problem = std::make_shared<Evaluation::PredictionProblem>(dataSet, 3);


    // Create representation
    auto lgpRep = std::make_shared<Representations::LGP>(dataSet.getInputDimensions(), Data::DataType::type<double>({8}), mnistSet, Data::DataType::type<double>({1}), 1, 20);
    auto tpgRep = std::make_shared<Representations::TPG>(problem->getInputDimensions(), 10, 2, 200);
    
    auto assigner = std::make_shared<FitnessAssignment::ClassificationF1Assigner>();
    auto parentSelector = std::make_shared<Selection::TournamentSelector>(2, true);

    Evolution::OriginTPGEvolutionConfig config;
    config.sizePopulation = 100;
    config.nbOffspring = 1000;
    config.nbTrainingIterations = 500;
    config.nbValidationIterations = 500;
    config.nbThreads = 20;
    config.archiveSize = 50;
    config.isArchiveFixed = true;
    config.protectAggregated = false;
    config.assigner = assigner;
    config.representation = tpgRep;
    config.memberRepresentation = lgpRep;
    config.problem = problem;
    auto evolutionModule = config.buildModule();
    const std::string population(config.names.population);
    const std::string populationFitness(config.names.trainingFitness);


    std::cout<<evolutionModule->summary()<<std::endl;


    std::string str = "graph";
    evolutionModule->add(std::make_shared<Evolution::ExportStep>(population, MNIST_DATA_LOCATION + str, false));
    str = "graphBest";
    evolutionModule->add(std::make_shared<Evolution::ExportStep>(population, MNIST_DATA_LOCATION + str, false, populationFitness));


    Evolution::EvolutionData data = evolutionModule->evolve(40, 4);

    EXPECT_EQ(16454860796132402384U, data.rng.uniformSample<uint64_t>(0, UINT64_MAX)) << "bouh bouh bouh th determinism";
    EXPECT_EQ(65221, Individual::getIndividualIDCounter()) << "bouh bouh bouh th determinism";

    std::cout<<data.summary()<<std::endl;
}

TEST_F(TPGAlgoTest, customEvolutionLGPAndTPGApart) {
    mnist::MNIST_dataset<std::vector, std::vector<double>, uint8_t> mnist(mnist::read_dataset<std::vector, std::vector, double, uint8_t>(MNIST_DATA_LOCATION));
    Dimensions::Requirement inputReq = Dimensions::Requirement(Data::DataType::type<double>({28, 28}));

    // The TPG classifies the digits. The LGP evolves on its own on the same images, with a double to predict since its individuals
    // are the bidders of the TPG (there is no regression problem, so the equality of the metric rarely holds: the evolution of the LGP is mainly a plumbing check).
    Evaluation::DataSet dataSet({inputReq}, Dimensions::Requirement(Data::DataType::type<size_t>()));
    Evaluation::DataSet lgpDataSet({inputReq}, Dimensions::Requirement(Data::DataType::type<double>()));
    for(size_t idx = 0; idx < mnist.training_images.size(); idx++) {
        std::vector<Data::DataValue> inputs;
        inputs.push_back(Data::DataValue::array2d<std::vector<double>>(mnist.training_images.at(idx), 28, 28));
        std::vector<Data::DataValue> lgpInputs;
        lgpInputs.push_back(Data::DataValue::array2d<std::vector<double>>(mnist.training_images.at(idx), 28, 28));
        dataSet.addData(std::move(inputs), Data::DataValue::scalar<uint64_t>(static_cast<uint64_t>(mnist.training_labels.at(idx))));
        lgpDataSet.addData(std::move(lgpInputs), Data::DataValue::scalar<double>(static_cast<double>(mnist.training_labels.at(idx))));
    }
    // The test images are the validation data of the TPG.
    for(size_t idx = 0; idx < mnist.test_images.size(); idx++) {
        std::vector<Data::DataValue> inputs;
        inputs.push_back(Data::DataValue::array2d<std::vector<double>>(mnist.test_images.at(idx), 28, 28));
        dataSet.addData(std::move(inputs), Data::DataValue::scalar<uint64_t>(static_cast<uint64_t>(mnist.test_labels.at(idx))), false);
    }

    auto lgpRep = std::make_shared<Representations::LGP>(dataSet.getInputDimensions(), Data::DataType::type<double>({8}), mnistSet, Data::DataType::type<double>({1}), 1, 20);
    auto tpgRep = std::make_shared<Representations::TPG>(dataSet.getInputDimensions(), 10, 2, 20);

    Evolution::BasicEvolutionConfig lgp;
    lgp.representation = lgpRep;
    lgp.problem = std::make_shared<Evaluation::PredictionProblem>(lgpDataSet, 500);
    lgp.sizePopulation = 100;
    lgp.nbOffspring = 1000;
    lgp.nbTrainingIterations = 50;
    lgp.nbThreads = 20;
    lgp.names.population = "LGP";
    lgp.names.parents = "LGP.parents";
    lgp.names.offspring = "LGP.offspring";
    lgp.names.trainingMetrics = "LGP.trainingMetrics";
    lgp.names.trainingFitness = "LGP.trainingFitness";
    lgp.names.generator = "LGP.generator";
    lgp.names.evolution = "LGP";
    lgp.logOutput = nullptr;

    Evolution::BasicEvolutionConfig tpg;
    tpg.representation = tpgRep;
    auto tpgProblem = std::make_shared<Evaluation::PredictionProblem>(dataSet, 500);
    tpg.problem = tpgProblem;
    tpg.sizePopulation = 100;
    tpg.nbOffspring = 1000;
    tpg.nbTrainingIterations = 500;
    tpg.nbValidationIterations = 500;
    tpg.nbThreads = 20;
    tpg.names.population = "TPG";
    tpg.names.parents = "TPG.parents";
    tpg.names.offspring = "TPG.offspring";
    tpg.names.trainingMetrics = "TPG.trainingMetrics";
    tpg.names.trainingFitness = "TPG.trainingFitness";
    tpg.names.evolution = "TPG";
    tpg.names.generator = "TPG.generator";
    tpg.logOutput = nullptr;

    // Before the TPG is created or mutated, a step builds its generator: the members are the current individuals of the LGP,
    // and the destinations are actions or the current individuals of the TPG (none at the beginning).
    const Evolution::BasicEvolutionConfig::Names lgpKeys(lgp.names);
    const Evolution::BasicEvolutionConfig::Names tpgKeys(tpg.names);
    auto makeGeneratorStep = [&]() {
        return std::make_shared<Evolution::LambdaStep>("TPGGenerator", [tpgRep, lgpKeys, tpgKeys](Evolution::EvolutionData& data) {
            // The member of a node is one of the current individuals of the LGP.
            const std::vector<std::shared_ptr<const Individual>> members = constIndividuals(*data.get<std::vector<std::shared_ptr<Individual>>>(lgpKeys.population));
            GraphBased::NodeGenerator bidNodes;
            bidNodes.addGenerator(Dimensions::ListUniformGenerator<std::shared_ptr<const Individual>>(members));

            // The destination is an action or, sometimes, one of the current individuals of the TPG (none at the beginning).
            const std::vector<std::shared_ptr<const Individual>> tangled = data.has(tpgKeys.population) ? constIndividuals(*data.get<std::vector<std::shared_ptr<Individual>>>(tpgKeys.population)) : std::vector<std::shared_ptr<const Individual>>{};
            Dimensions::NumericUniformGenerator<size_t> actionGenerator(0, tpgRep->getNbActions() - 1);
            if(tangled.empty()) {
                bidNodes.addGenerator(actionGenerator);
            } else {
                Dimensions::MultiGenerator destination;
                destination.addGenerator(actionGenerator, 0.995);
                destination.addGenerator(Dimensions::ListUniformGenerator<std::shared_ptr<const Individual>>(tangled), 0.005);
                bidNodes.addGenerator(destination);
            }
            data.set(tpgKeys.generator, std::make_shared<GraphBased::GenotypeGenerator>(bidNodes, tpgRep->getGenotypeConstraint().getRangeAt(0)));
        }, std::set<std::string>{lgp.names.population}, std::set<std::string>{tpg.names.generator});
    };

    // The LGP does a generation, then the TPG does one. Its generator replaces the naive one of its initialization, and is created again before its mutation.
    auto evolutionModule = std::make_shared<Evolution::Module>("Evolution");
    evolutionModule->add(lgp.buildModule());
    auto tpgModule = tpg.buildModule();
    tpgModule->getInitialization()->replace(tpgModule->getInitialization()->find("CreateGenerator"), makeGeneratorStep());
    tpgModule->insertBefore(tpgModule->find("Mutation"), makeGeneratorStep());

    // The outputs of the members (the LGP individuals) on the samples of the evaluation are computed once, whatever the number of vertices using them.
    const std::string cacheKey("TPG.memberOutputCache");
    tpgModule->insertAfter(tpgModule->find("Mutation"), std::make_shared<Evolution::CacheMemberOutputsStep>(
        std::vector<std::string>{tpgKeys.population, tpgKeys.offspring}, cacheKey, tpgProblem,
        tpg.nbTrainingIterations, Evaluation::Mode::TRAINING, tpg.nbThreads));
    // The log of the TPG: the columns of the preset and the number of vertices, and a second log written for another tool with the time of each step.
    std::vector<Evolution::LogStep::Column> columns = tpg.logColumns();
    columns.insert(columns.begin() + 1, {"NbVertex", [tpgKeys](const Evolution::EvolutionData& data) {
        return static_cast<double>(Evolution::getAllVertices(constIndividuals(*data.get<std::vector<std::shared_ptr<Individual>>>(tpgKeys.population))).size());
    }, 0, {tpgKeys.population}});
    std::ostringstream csv;
    const std::vector<Evolution::LogStep::Column> times = Evolution::stepTimeColumns(*tpgModule);
    tpgModule->add(std::make_shared<Evolution::LogStep>(std::cout, columns));
    columns.insert(columns.end(), times.begin(), times.end());
    tpgModule->add(std::make_shared<Evolution::LogStep>(csv, columns, ";", "CsvLog"));
    evolutionModule->add(tpgModule);

    
    std::cout << evolutionModule->summary() << std::endl;

    Evolution::EvolutionData data = evolutionModule->evolve(40, 4);
    std::cout << data.summary() << std::endl;
    std::cout << csv.str() << std::endl;

    EXPECT_EQ(data.nbGenerationsDone, 40U);
    EXPECT_NE(csv.str().find("Gen;Pop_Size;NbVertex;Fit_Min"), std::string::npos);
    EXPECT_EQ(constIndividuals(*data.get<std::vector<std::shared_ptr<Individual>>>(lgpKeys.population)).size(), lgp.sizePopulation);
    const std::vector<std::shared_ptr<const Individual>> roots = constIndividuals(*data.get<std::vector<std::shared_ptr<Individual>>>(tpgKeys.population));
    ASSERT_EQ(roots.size(), tpg.sizePopulation);
    EXPECT_TRUE(data.isUpToDate(tpgKeys.trainingFitness));

    // The cache holds the members used by the TPG, which are individuals of the LGP.
    ASSERT_TRUE(data.has(cacheKey));
    EXPECT_FALSE(data.get<std::vector<std::shared_ptr<const Individual>>>(cacheKey)->empty());
    for(const auto& member: *data.get<std::vector<std::shared_ptr<const Individual>>>(cacheKey)) {
        EXPECT_EQ(&member->getRepresentation(), lgpRep.get());
    }

    // The nodes of the TPG bid with individuals of the LGP.
    for(const auto& vertex: roots) {
        const GraphBased::NodeGroup& group = vertex->getGenotype().getNodeGroup(0);
        for(size_t idx = 0; idx < group.getSize(); idx++) {
            EXPECT_EQ(&group.getNode(idx).getValue(0).getScalar<std::shared_ptr<const Individual>>()->getRepresentation(), lgpRep.get());
        }
    }
}

TEST_F(TPGAlgoTest, customEvolutionTPG3) {
    mnist::MNIST_dataset<std::vector, std::vector<double>, uint8_t> mnist(mnist::read_dataset<std::vector, std::vector, double, uint8_t>(MNIST_DATA_LOCATION));
    Dimensions::Requirement inputReq = Dimensions::Requirement(Data::DataType::type<double>({28, 28}));
    Dimensions::Requirement outputReq = Dimensions::Requirement(Data::DataType::type<size_t>());
    Evaluation::DataSet dataSet({inputReq}, outputReq);

    std::vector<std::vector<double>> trainingImages = mnist.training_images;
    std::vector<uint8_t> trainingLabels = mnist.training_labels;
    for(size_t idx = 0; idx < trainingImages.size(); idx++) {
        std::vector<Data::DataValue> inputs;
        inputs.push_back(Data::DataValue::array2d<std::vector<double>>(trainingImages.at(idx), 28, 28));
        
        dataSet.addData(std::move(inputs), Data::DataValue::scalar<uint64_t>(static_cast<uint64_t>(trainingLabels.at(idx))));
    }

    std::vector<std::vector<double>> testImages = mnist.test_images;
    std::vector<uint8_t> testLabels = mnist.test_labels;
    for(size_t idx = 0; idx < testImages.size(); idx++) {
        std::vector<Data::DataValue> inputs;
        inputs.push_back(Data::DataValue::array2d<std::vector<double>>(testImages.at(idx), 28, 28));
        
        dataSet.addData(std::move(inputs), Data::DataValue::scalar<uint64_t>(static_cast<uint64_t>(testLabels.at(idx))), false);
    }

    // Create evaluationAgent
    auto problem = std::make_shared<Evaluation::PredictionProblem>(dataSet, 3);


    // Create representation
    auto lgpRep = std::make_shared<Representations::LGP>(dataSet.getInputDimensions(), Data::DataType::type<double>({8}), mnistSet, Data::DataType::type<double>({1}), 1, 20);
    auto tpgRep = std::make_shared<Representations::TPG>(problem->getInputDimensions(), 10, 2, 200);
    
    auto assigner = std::make_shared<FitnessAssignment::ClassificationF1Assigner>();
    auto parentSelector = std::make_shared<Selection::TournamentSelector>(2, true);

    Evolution::OriginTPGEvolutionConfig config;
    config.sizePopulation = 100;
    config.nbOffspring = 1000;
    config.nbTrainingIterations = 500;
    config.nbValidationIterations = 500;
    config.nbThreads = 20;
    config.archiveSize = 50;
    config.isArchiveFixed = false;
    config.protectAggregated = false;
    config.assigner = assigner;
    config.representation = tpgRep;
    config.memberRepresentation = lgpRep;
    config.problem = problem;
    auto evolutionModule = config.buildModule();
    Evolution::EvolutionData state(4);
    //evolutionModule->remove(evolutionModule->findStep("CacheMemberOutputs"));
    std::cout<<evolutionModule->summary()<<std::endl;

    std::string str = "graph";
    evolutionModule->add(std::make_shared<Evolution::ExportStep>(config.names.population, MNIST_DATA_LOCATION + str, false));
    str = "graphBest";
    evolutionModule->add(std::make_shared<Evolution::ExportStep>(config.names.population, MNIST_DATA_LOCATION + str, false, config.names.trainingFitness));


    evolutionModule->initialize(state);
    evolutionModule->evolve(state, 40);

    EXPECT_EQ(13348146703180701991U, state.rng.uniformSample<uint64_t>(0, UINT64_MAX)) << "bouh bouh bouh th determinism";
    EXPECT_EQ(62131, Individual::getIndividualIDCounter()) << "bouh bouh bouh th determinism";
}

#ifdef GEGELATI_WITH_OPENCV


#include "instructions/imageInstructionSet.h"
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
    auto problem = std::make_shared<Evaluation::PredictionProblem>(dataSet, 3);

    auto instructions = Instructions::ImageInstructions::createAll<double>(28, 28);
    Instructions::Set imageSet;
    for(auto& intr: instructions) {
        imageSet.add(*intr);
    }

    std::vector<size_t> axes = {1, 2};

    // Create representation
    auto lgpRep = std::make_shared<Representations::LGP>(dataSet.getInputDimensions(), Data::DataType::type<double>({10, 28, 28}), imageSet, Data::DataType::type<double>({10, 28, 28}), 1, 20);
    lgpRep->addOutputFunction(std::make_unique<Dimensions::ActivationFunctions::Sum<double>>(lgpRep->getDimensionFlow().getOutputDimension(), axes));
    lgpRep->addOutputFunction(std::make_unique<Dimensions::ActivationFunctions::ArgMax<double>>(lgpRep->getDimensionFlow().getOutputDimension()));
    std::cout<<lgpRep->summary()<<std::endl;
    
    Evolution::BasicEvolutionConfig config;
    config.sizePopulation = 50;
    config.nbOffspring = 450;
    config.nbTrainingIterations = 500;
    config.nbThreads = 20;
    config.representation = lgpRep;
    config.problem = problem;
    auto evolutionModule = config.buildModule();
    Evolution::EvolutionData state(4);
    const Evolution::BasicEvolutionConfig::Names population;
    std::string str = "graphLGP";
    evolutionModule->add(std::make_shared<Evolution::ExportStep>(population.population, MNIST_DATA_LOCATION + str, false));
    str = "graphLGPBest";
    evolutionModule->add(std::make_shared<Evolution::ExportStep>(population.population, MNIST_DATA_LOCATION + str, false, population.trainingFitness));
    evolutionModule->initialize(state);
    evolutionModule->evolve(state, 200);

    ASSERT_EQ(13253408461481353746U, state.rng.uniformSample<uint64_t>(0, UINT64_MAX)) << "bouh bouh bouh th determinism";
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
    auto problem = std::make_shared<Evaluation::PredictionProblem>(dataSet, 3);

    auto lgpRep = std::make_shared<Representations::LGP>(dataSet.getInputDimensions(), Data::DataType::type<double>({8, 28, 28}), imageSet, Data::DataType::type<double>({1, 28, 28}), 1, 20);
    lgpRep->addOutputFunction(std::make_unique<Dimensions::ActivationFunctions::Sum<double>>(lgpRep->getDimensionFlow().getOutputDimension()));
    std::cout<<lgpRep->summary()<<std::endl;
    auto tpgRep = std::make_shared<Representations::TPG>(problem->getInputDimensions(), 10, 2, 200);

    auto assigner = std::make_shared<FitnessAssignment::ClassificationF1Assigner>();

    Evolution::OriginTPGEvolutionConfig config;
    config.sizePopulation = 100;
    config.nbOffspring = 400;
    config.nbTrainingIterations = 500;
    config.nbThreads = 20;
    config.archiveSize = 0;
    config.isArchiveFixed = true;
    config.protectAggregated = false;
    config.assigner = assigner;
    config.representation = tpgRep;
    config.memberRepresentation = lgpRep;
    config.problem = problem;
    auto evolutionModule = config.buildModule();
    Evolution::EvolutionData state(4);
    const Evolution::BasicEvolutionConfig::Names population;

    std::string str = "graph";
    evolutionModule->add(std::make_shared<Evolution::ExportStep>(population.population, MNIST_DATA_LOCATION + str, false));
    str = "graphBest";
    evolutionModule->add(std::make_shared<Evolution::ExportStep>(population.population, MNIST_DATA_LOCATION + str, false, population.trainingFitness));


    evolutionModule->initialize(state);
    evolutionModule->evolve(state, 10000);

    ASSERT_EQ(17819704308639420184U, state.rng.uniformSample<uint64_t>(0, UINT64_MAX)) << "bouh bouh bouh th determinism";
    ASSERT_EQ(22492, Individual::getIndividualIDCounter()) << "bouh bouh bouh th determinism";
}
#endif