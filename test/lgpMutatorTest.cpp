
#include <gtest/gtest.h>
#include "instructions/lambdaInstruction.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/multByConstant.h"
#include "representation/lgp/lgpMutator.h"
#include "representation/lgp/lgpIndividual.h"
#include "representation/lgp/lgpLine.h"
#include "representation/lgp/lgpExecutionEngine.h"
#include "util/counterReset.h"
#include "parameters.h"


class LgpMutatorTest : public ::testing::Test
{
  protected:
    const size_t size1{24};
    const size_t size2{32};
    const double value0{2.3};
    const float value1{4.2f};
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
    Instructions::Set set;
    std::shared_ptr<const Representation::LGP::LGPEnvironment> e;
    Parameters params;
    std::shared_ptr<EvoGraph::Graph> graph;
    Output::OutputHandler* lgpOutput;
    Selector::Selector* selector;
    std::shared_ptr<Representation::LGP::LGPPopulation> lgpPopulation;
    const Representation::Individual* individual;
    const Representation::LGP::LgpIndividual* lgpIndividual;
    std::shared_ptr<Representation::LGP::LGPMutator> lgpMutator;

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
        params.representation.lgp.nbRegisters = 8;
        params.representation.lgp.nbProgramConstant = 5;
        e = std::make_shared<Representation::LGP::LGPEnvironment>(set, params.representation.lgp.nbRegisters, params.representation.lgp.nbProgramConstant, vect);
        lgpOutput = new Output::OutputHandler(Output::Output());
        lgpPopulation = std::make_shared<Representation::LGP::LGPPopulation>(*e, *lgpOutput, (uint64_t)1);

        individual = &lgpPopulation->createIndividual(*graph);
        lgpIndividual = dynamic_cast<const Representation::LGP::LgpIndividual*>(individual);
        lgpMutator = std::make_shared<Representation::LGP::LGPMutator>(*selector, (uint64_t)1);
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
    ASSERT_FALSE(lgpMutator->deleteRandomLine(*lgpIndividual, *lgpPopulation, rng));
    ASSERT_EQ(lgpIndividual->getNbLines(), 0);

    // Attempt removing on a LGP with a single line
    lgpPopulation->addNewLine(*lgpIndividual);
    ASSERT_FALSE(lgpMutator->deleteRandomLine(*lgpIndividual, *lgpPopulation, rng));
    ASSERT_EQ(lgpIndividual->getNbLines(), 1);

    // Insert lines
    for (auto i = 0; i < nbLines - 1; i++) {
        lgpPopulation->addNewLine(*lgpIndividual);
    }

    // Delete a random line
    ASSERT_TRUE(lgpMutator->deleteRandomLine(*lgpIndividual, *lgpPopulation, rng));
    ASSERT_EQ(lgpIndividual->getNbLines(), nbLines - 1);
}

TEST_F(LgpMutatorTest, LGPMutatorInsertRandomLine)
{
    RNG::RNG rng;
    rng.setSeed(0);

    // Insert in empty LGP
    ASSERT_NO_THROW(lgpMutator->insertRandomLine(*lgpIndividual, *lgpPopulation, params.representation, rng));
    ASSERT_EQ(lgpIndividual->getNbLines(), 1)
        << "Line insertion in an empty LGP failed.";

    // Insert in non empty LGP
    // in first position (with known seed)
    rng.setSeed(0);
    ASSERT_NO_THROW(lgpMutator->insertRandomLine(*lgpIndividual, *lgpPopulation, params.representation, rng));
    ASSERT_EQ(lgpIndividual->getNbLines(), 2)
        << "Line insertion in a non-empty LGP failed.";

    // Insert in non empty LGP
    // After last position (with known seed)
    rng.setSeed(1);
    ASSERT_NO_THROW(lgpMutator->insertRandomLine(*lgpIndividual, *lgpPopulation, params.representation, rng));
    ASSERT_EQ(lgpIndividual->getNbLines(), 3)
        << "Line insertion in a non-empty LGP failed.";

    // Insert in non empty LGP
    // In the middle position (with known seed)
    rng.setSeed(5);
    ASSERT_NO_THROW(lgpMutator->insertRandomLine(*lgpIndividual, *lgpPopulation, params.representation, rng));
    ASSERT_EQ(lgpIndividual->getNbLines(), 4)
        << "Line insertion in a non-empty LGP failed.";
}

TEST_F(LgpMutatorTest, LGPMutatorSwapRandomLines)
{
    RNG::RNG rng;
    rng.setSeed(0);

    std::vector<const Representation::LGP::LGPLine*> lines;
    // Nothing on empty LGP
    ASSERT_FALSE(lgpMutator->swapRandomLines(*lgpIndividual, *lgpPopulation, rng));

    // Add a first line
    lines.push_back(&lgpPopulation->addNewLine(*lgpIndividual));

    // Nothing on LGP with one line.
    ASSERT_FALSE(lgpMutator->swapRandomLines(*lgpIndividual, *lgpPopulation, rng));

    // Add a second line
    lines.push_back(&lgpPopulation->addNewLine(*lgpIndividual));

    // Exchanges the two line.
    ASSERT_TRUE(lgpMutator->swapRandomLines(*lgpIndividual, *lgpPopulation, rng));
    ASSERT_EQ(lines.at(0), &lgpIndividual->getLine(1));
    ASSERT_EQ(lines.at(1), &lgpIndividual->getLine(0));

    // Add 8 lines
    for (auto i = 0; i < 8; i++) {
        lines.push_back(&lgpPopulation->addNewLine(*lgpIndividual));
    }
    // Swap two random lines (with a known seed)
    ASSERT_TRUE(lgpMutator->swapRandomLines(*lgpIndividual, *lgpPopulation, rng));
    // Only lines 4 and 7 are swapped
    ASSERT_EQ(lines.at(0), &lgpIndividual->getLine(1));
    ASSERT_EQ(lines.at(1), &lgpIndividual->getLine(0));
    ASSERT_EQ(lines.at(2), &lgpIndividual->getLine(2));
    ASSERT_EQ(lines.at(3), &lgpIndividual->getLine(3));
    ASSERT_EQ(lines.at(4), &lgpIndividual->getLine(7));
    ASSERT_EQ(lines.at(5), &lgpIndividual->getLine(5));
    ASSERT_EQ(lines.at(6), &lgpIndividual->getLine(6));
    ASSERT_EQ(lines.at(7), &lgpIndividual->getLine(4));
    ASSERT_EQ(lines.at(8), &lgpIndividual->getLine(8));
    ASSERT_EQ(lines.at(9), &lgpIndividual->getLine(9));
}

TEST_F(LgpMutatorTest, LGPMutatorAlterRandomLine)
{
    RNG::RNG rng;
    rng.setSeed(0);

    // Nothing on empty LGP
    ASSERT_FALSE(lgpMutator->alterRandomLine(*lgpIndividual, *lgpPopulation, params.representation, rng));
    // Add 10 lines
    for (auto i = 0; i < 10; i++) {
        lgpPopulation->addNewLine(*lgpIndividual);
    }
    // Alter a randomly selected line (with a known seed)
    // Parameter of Line 4 is altered.
    ASSERT_TRUE(lgpMutator->alterRandomLine(*lgpIndividual, *lgpPopulation, params.representation, rng));
}

TEST_F(LgpMutatorTest, LGPMutatorInitIndividual)
{
    RNG::RNG rng;
    rng.setSeed(0);

    params.representation.lgp.initMaxProgramSize = 96;
    params.representation.lgp.initMinProgramSize = 1;
    params.representation.lgp.maxConstValue = 10;
    params.representation.lgp.minConstValue = 0;

    std::shared_ptr<EvoGraph::Graph> graph = std::make_shared<EvoGraph::Graph>();

    ASSERT_NO_THROW(lgpIndividual = dynamic_cast<const Representation::LGP::LgpIndividual*>(&lgpMutator->initRandomIndividual(*graph, *lgpPopulation, params.representation, rng)))
        << "Empty LGP Random init failed";
    ASSERT_EQ(lgpIndividual->getNbLines(), 15)
        << "Random number of line is not as expected (with known seed).";


    ASSERT_NO_THROW(lgpMutator->initRandomSpecificIndividual(*lgpIndividual, *graph, *lgpPopulation, params.representation, rng))
        << "Non-Empty LGP Random init failed";
    ASSERT_EQ(lgpIndividual->getNbLines(), 38)
        << "Random number of line is not as expected (with known seed).";

    // Count lines marked as introns (with a known seed).
    uint64_t nbIntrons = 0;
    for (auto i = 0; i < lgpIndividual->getNbLines(); i++) {
        if (lgpIndividual->isIntron(i)) {
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
    const Representation::LGP::LgpIndividual& lgpIndividual2 = *dynamic_cast<const Representation::LGP::LgpIndividual*>(&lgpPopulation->createIndividual(*graph));

    Representation::LGP::LGPLineMutator lineMutator;
    Selector::SelectionContext context;

    rng.setSeed(14);
    lgpPopulation->addNewLine(lgpIndividual2);
    Representation::LGP::LGPLine& l = lgpPopulation->getLineForMutation(lgpIndividual2, 0);
    lineMutator.initRandomCorrectLine(l, rng);
    lgpPopulation->addNewLine(lgpIndividual2);
    Representation::LGP::LGPLine& l2 = lgpPopulation->getLineForMutation(lgpIndividual2, 1);
    lineMutator.initRandomCorrectLine(l2, rng);
    lgpPopulation->addNewLine(lgpIndividual2);
    Representation::LGP::LGPLine& l3 = lgpPopulation->getLineForMutation(lgpIndividual2, 2);
    lineMutator.initRandomCorrectLine(l3, rng);

    params.representation.lgp.maxProgramSize = 15;
    params.representation.lgp.pDelete = 0.5;
    params.representation.lgp.pAdd = 0.0;
    params.representation.lgp.pMutate = 0.0;
    params.representation.lgp.pSwap = 0.0;
    params.representation.lgp.maxConstValue = 1;
    params.representation.lgp.minConstValue = 0;
    params.representation.lgp.pConstantMutation = 0.2;

    rng.setSeed(0);
    ASSERT_TRUE(lgpMutator->mutateLgpIndividual(lgpIndividual2, *lgpPopulation, params.representation, rng))
        << "Mutation did not occur with known seed.";
    ASSERT_EQ(lgpIndividual2.getNbLines(), 2)
        << "Wrong LGP mutation occured. Expected: Line deletion.";

    params.representation.lgp.pDelete = 0.0;
    params.representation.lgp.pAdd = 0.5;
    rng.setSeed(1);
    ASSERT_TRUE(lgpMutator->mutateLgpIndividual(lgpIndividual2, *lgpPopulation, params.representation, rng))
        << "Mutation did not occur with known seed.";
    ASSERT_EQ(lgpIndividual2.getNbLines(), 3)
        << "Wrong LGP mutation occured. Expected: Line insertion.";

    params.representation.lgp.pAdd = 0.0;
    params.representation.lgp.pMutate = 0.01;
    rng.setSeed(86);
    ASSERT_TRUE(lgpMutator->mutateLgpIndividual(lgpIndividual2, *lgpPopulation, params.representation, rng))
        << "Mutation did not occur with known seed.";

    params.representation.lgp.pMutate = 0.00;
    params.representation.lgp.pSwap = 0.1;
    rng.setSeed(1);
    ASSERT_TRUE(lgpMutator->mutateLgpIndividual(lgpIndividual2, *lgpPopulation, params.representation, rng))
        << "Mutation did not occur with known seed.";

    // mutate other instructions
    params.representation.lgp.pSwap = 0.0;
    params.representation.lgp.pMutate = 1;
    rng.setSeed(114);
    ASSERT_TRUE(lgpMutator->mutateLgpIndividual(lgpIndividual2, *lgpPopulation, params.representation, rng))
        << "Mutation did not occur with known seed.";

    // Teardown for this test
    delete &set.getInstruction(4);
}