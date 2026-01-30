
#include <gtest/gtest.h>
#include "instructions/lambdaInstruction.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/multByConstant.h"
#include "algorithm/lgp/lgpMutator.h"
#include "algorithm/lgp/lgpAgent.h"
#include "algorithm/lgp/lgpLine.h"
#include "algorithm/lgp/lgpExecutionEngine.h"
#include "util/counterReset.h"


class LgpMutatorTest : public ::testing::Test
{
  protected:
    const size_t size1{24};
    const size_t size2{32};
    const double value0{2.3};
    const float value1{4.2f};
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
    Instructions::Set set;
    std::shared_ptr<const Environment> e;
    Learn::LearningParameters params;
    std::shared_ptr<EvoGraph::Graph> graph;
    Output::OutputHandler* lgpOutput;
    std::shared_ptr<Algorithm::LGP::LGPManager> lgpManager;
    std::shared_ptr<const Algorithm::LGP::LGPAgent> lgpAgent;
    std::shared_ptr<Algorithm::LGP::LGPMutator> lgpMutator;

    LgpMutatorTest() : e{nullptr} {};

    virtual void SetUp()
    {

        CounterReset::counterReset();
        vect.push_back(
            *(new Data::PrimitiveTypeArray<int>((unsigned int)size1)));
        vect.push_back(
            *(new Data::PrimitiveTypeArray<double>((unsigned int)size2)));

        ((Data::PrimitiveTypeArray<double>&)vect.at(1).get())
            .setDataAt(typeid(double), 25, value0);

        std::function<double(double, double)> minus =
            [](double a, double b) -> double { return a - b; };
        std::function<double(double, double)> add =
            [](double a, double b) -> double { return a + b; };

        set.add(*(new Instructions::MultByConstant<double>()));
        set.add(*(new Instructions::AddPrimitiveType<double>()));
        set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));
        set.add(*(new Instructions::LambdaInstruction<double, double>(add)));

        // the environment and the LGPs have 5 Constant parameters
        params.nbRegisters = 8;
        params.nbProgramConstant = 5;
        e = std::make_shared<Environment>(set, params, vect);
        lgpOutput = new Output::OutputHandler(Output::Output());
        lgpManager = std::make_shared<Algorithm::LGP::LGPManager>(e, *lgpOutput);
        lgpManager->setAlgorithmName("fake");

        lgpAgent = std::dynamic_pointer_cast<const Algorithm::LGP::LGPAgent>(lgpManager->createAgent(graph));
        lgpMutator = std::make_shared<Algorithm::LGP::LGPMutator>();
    }

    virtual void TearDown()
    {
        delete (&(vect.at(0).get()));
        delete (&(vect.at(1).get()));
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
        delete (&set.getInstruction(2));
        delete (&set.getInstruction(3));
    }
};


TEST_F(LgpMutatorTest, LGPMutatorDeleteRandomLine)
{
    const uint64_t nbLines = 10;
    RNG::RNG rng;
    rng.setSeed(0);

    // Attempt removing on an empty LGP
    ASSERT_FALSE(lgpMutator->deleteRandomLine(lgpAgent, lgpManager, rng));
    ASSERT_EQ(lgpAgent->getNbLines(), 0);

    // Attempt removing on a LGP with a single line
    lgpManager->addNewLine(lgpAgent);
    ASSERT_FALSE(lgpMutator->deleteRandomLine(lgpAgent, lgpManager, rng));
    ASSERT_EQ(lgpAgent->getNbLines(), 1);

    // Insert lines
    for (auto i = 0; i < nbLines - 1; i++) {
        lgpManager->addNewLine(lgpAgent);
    }

    // Delete a random line
    ASSERT_TRUE(lgpMutator->deleteRandomLine(lgpAgent, lgpManager, rng));
    ASSERT_EQ(lgpAgent->getNbLines(), nbLines - 1);
}

TEST_F(LgpMutatorTest, LGPMutatorInsertRandomLine)
{
    RNG::RNG rng;
    rng.setSeed(0);

    // Insert in empty LGP
    ASSERT_NO_THROW(lgpMutator->insertRandomLine(lgpAgent, lgpManager, rng));
    ASSERT_EQ(lgpAgent->getNbLines(), 1)
        << "Line insertion in an empty LGP failed.";

    // Insert in non empty LGP
    // in first position (with known seed)
    rng.setSeed(0);
    ASSERT_NO_THROW(lgpMutator->insertRandomLine(lgpAgent, lgpManager, rng));
    ASSERT_EQ(lgpAgent->getNbLines(), 2)
        << "Line insertion in a non-empty LGP failed.";

    // Insert in non empty LGP
    // After last position (with known seed)
    rng.setSeed(1);
    ASSERT_NO_THROW(lgpMutator->insertRandomLine(lgpAgent, lgpManager, rng));
    ASSERT_EQ(lgpAgent->getNbLines(), 3)
        << "Line insertion in a non-empty LGP failed.";

    // Insert in non empty LGP
    // In the middle position (with known seed)
    rng.setSeed(5);
    ASSERT_NO_THROW(lgpMutator->insertRandomLine(lgpAgent, lgpManager, rng));
    ASSERT_EQ(lgpAgent->getNbLines(), 4)
        << "Line insertion in a non-empty LGP failed.";
}

TEST_F(LgpMutatorTest, LGPMutatorSwapRandomLines)
{
    RNG::RNG rng;
    rng.setSeed(0);

    std::vector<Algorithm::LGP::LGPLine*> lines;
    // Nothing on empty LGP
    ASSERT_FALSE(lgpMutator->swapRandomLines(lgpAgent, lgpManager, rng));

    // Add a first line
    lines.push_back(&lgpManager->addNewLine(lgpAgent));

    // Nothing on LGP with one line.
    ASSERT_FALSE(lgpMutator->swapRandomLines(lgpAgent, lgpManager, rng));

    // Add a second line
    lines.push_back(&lgpManager->addNewLine(lgpAgent));

    // Exchanges the two line.
    ASSERT_TRUE(lgpMutator->swapRandomLines(lgpAgent, lgpManager, rng));
    ASSERT_EQ(lines.at(0), &lgpAgent->getLine(1));
    ASSERT_EQ(lines.at(1), &lgpAgent->getLine(0));

    // Add 8 lines
    for (auto i = 0; i < 8; i++) {
        lines.push_back(&lgpManager->addNewLine(lgpAgent));
    }
    // Swap two random lines (with a known seed)
    ASSERT_TRUE(lgpMutator->swapRandomLines(lgpAgent, lgpManager, rng));
    // Only lines 4 and 7 are swapped
    ASSERT_EQ(lines.at(0), &lgpAgent->getLine(1));
    ASSERT_EQ(lines.at(1), &lgpAgent->getLine(0));
    ASSERT_EQ(lines.at(2), &lgpAgent->getLine(2));
    ASSERT_EQ(lines.at(3), &lgpAgent->getLine(3));
    ASSERT_EQ(lines.at(4), &lgpAgent->getLine(7));
    ASSERT_EQ(lines.at(5), &lgpAgent->getLine(5));
    ASSERT_EQ(lines.at(6), &lgpAgent->getLine(6));
    ASSERT_EQ(lines.at(7), &lgpAgent->getLine(4));
    ASSERT_EQ(lines.at(8), &lgpAgent->getLine(8));
    ASSERT_EQ(lines.at(9), &lgpAgent->getLine(9));
}

TEST_F(LgpMutatorTest, LGPMutatorAlterRandomLine)
{
    RNG::RNG rng;
    rng.setSeed(0);

    // Nothing on empty LGP
    ASSERT_FALSE(lgpMutator->alterRandomLine(lgpAgent, lgpManager, rng));
    // Add 10 lines
    for (auto i = 0; i < 10; i++) {
        lgpManager->addNewLine(lgpAgent);
    }
    // Alter a randomly selected line (with a known seed)
    // Parameter of Line 4 is altered.
    ASSERT_TRUE(lgpMutator->alterRandomLine(lgpAgent, lgpManager, rng));
}

TEST_F(LgpMutatorTest, LGPMutatorInitAgent)
{
    RNG::RNG rng;
    rng.setSeed(0);

    params.mutation.prog.initMaxProgramSize = 96;
    params.mutation.prog.initMinProgramSize = 1;
    params.mutation.prog.maxConstValue = 10;
    params.mutation.prog.minConstValue = 0;

    std::shared_ptr<EvoGraph::Graph> graph = std::make_shared<EvoGraph::Graph>();

    ASSERT_NO_THROW(lgpAgent = std::dynamic_pointer_cast<const Algorithm::LGP::LGPAgent>(lgpMutator->initRandomAgent(graph, lgpManager, params, rng)))
        << "Empty LGP Random init failed";
    ASSERT_EQ(lgpAgent->getNbLines(), 15)
        << "Random number of line is not as expected (with known seed).";


    ASSERT_NO_THROW(lgpAgent = std::dynamic_pointer_cast<const Algorithm::LGP::LGPAgent>(lgpMutator->initRandomAgent(nullptr, lgpManager, params, rng)))
        << "Non-Empty LGP Random init failed";
    ASSERT_EQ(lgpAgent->getNbLines(), 38)
        << "Random number of line is not as expected (with known seed).";

    // Count lines marked as introns (with a known seed).
    uint64_t nbIntrons = 0;
    for (auto i = 0; i < lgpAgent->getNbLines(); i++) {
        if (lgpAgent->isIntron(i)) {
            nbIntrons++;
        }
    }


    // Check nb intron lines with a known seed.
    ASSERT_EQ(nbIntrons, 36);
}

TEST_F(LgpMutatorTest, LGPMutatorMutateBehavior)
{
    RNG::RNG rng;
    // specific for this test
    // we need an instruction with three operands to
    // trigger a special condition in LineMutator
    set.add(*(new Instructions::LambdaInstruction<const double, const double,
                                                  const double>(
        [](const double a, const double b, const double c) -> double {
            return (cos(a + b + c));
        })));

    std::shared_ptr<const Environment> e2 = std::make_shared<Environment>(set, params, vect);
    auto lgpManager2 = std::make_shared<Algorithm::LGP::LGPManager>(e2, *lgpOutput);
    lgpManager2->setAlgorithmName("fake");
    std::shared_ptr<const Algorithm::LGP::LGPAgent> lgpAgent2 = std::dynamic_pointer_cast<const Algorithm::LGP::LGPAgent>(lgpManager2->createAgent(graph));

    Algorithm::LGP::LGPExecutionEngine lgpExecutionEngine(lgpAgent);
    Algorithm::LGP::LGPLineMutator lineMutator;
    Selector::SelectionContext context;

    rng.setSeed(14);
    Algorithm::LGP::LGPLine& l = lgpManager2->addNewLine(lgpAgent2);
    lineMutator.initRandomCorrectLine(l, rng);
    Algorithm::LGP::LGPLine& l2 = lgpManager2->addNewLine(lgpAgent2);
    lineMutator.initRandomCorrectLine(l2, rng);
    Algorithm::LGP::LGPLine& l3 = lgpManager2->addNewLine(lgpAgent2);
    lineMutator.initRandomCorrectLine(l3, rng);

    params.mutation.prog.maxProgramSize = 15;
    params.mutation.prog.pDelete = 0.5;
    params.mutation.prog.pAdd = 0.0;
    params.mutation.prog.pMutate = 0.0;
    params.mutation.prog.pSwap = 0.0;
    params.mutation.prog.maxConstValue = 1;
    params.mutation.prog.minConstValue = 0;
    params.mutation.prog.pConstantMutation = 0.2;

    rng.setSeed(0);
    ASSERT_TRUE(lgpMutator->mutateLGPAgent(lgpAgent2, lgpManager2, params, rng))
        << "Mutation did not occur with known seed.";
    ASSERT_EQ(lgpAgent2->getNbLines(), 2)
        << "Wrong LGP mutation occured. Expected: Line deletion.";

    params.mutation.prog.pDelete = 0.0;
    params.mutation.prog.pAdd = 0.5;
    rng.setSeed(1);
    ASSERT_TRUE(lgpMutator->mutateLGPAgent(lgpAgent2, lgpManager2, params, rng))
        << "Mutation did not occur with known seed.";
    ASSERT_EQ(lgpAgent2->getNbLines(), 3)
        << "Wrong LGP mutation occured. Expected: Line insertion.";

    params.mutation.prog.pAdd = 0.0;
    params.mutation.prog.pMutate = 0.01;
    rng.setSeed(86);
    ASSERT_TRUE(lgpMutator->mutateLGPAgent(lgpAgent2, lgpManager2, params, rng))
        << "Mutation did not occur with known seed.";

    params.mutation.prog.pMutate = 0.00;
    params.mutation.prog.pSwap = 0.1;
    rng.setSeed(1);
    ASSERT_TRUE(lgpMutator->mutateLGPAgent(lgpAgent2, lgpManager2, params, rng))
        << "Mutation did not occur with known seed.";

    // mutate other instructions
    params.mutation.prog.pSwap = 0.0;
    params.mutation.prog.pMutate = 1;
    rng.setSeed(114);
    ASSERT_TRUE(lgpMutator->mutateLGPAgent(lgpAgent2, lgpManager2, params, rng))
        << "Mutation did not occur with known seed.";

    // Teardown for this test
    delete &set.getInstruction(4);
}