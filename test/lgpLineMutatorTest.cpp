


#include <gtest/gtest.h>
#include "instructions/lambdaInstruction.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/multByConstant.h"
#include "algorithm/lgp/lgpLineMutator.h"
#include "algorithm/lgp/lgpAgent.h"
#include "algorithm/lgp/lgpLine.h"
#include "algorithm/lgp/lgpExecutionEngine.h"
#include "util/counterReset.h"

class LineMutatorTest : public ::testing::Test
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
    Algorithm::LGP::LGPLineMutator lineMutator;
    std::shared_ptr<Algorithm::LGP::LGPAgent> programAgent;
    Output::OutputHandler* lgpOutput;

    LineMutatorTest() : e{nullptr} {};

    virtual void SetUp()
    {
        
        lgpOutput = new Output::OutputHandler(Output::Output());

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

        // the environment and the programs have 5 Constant parameters
        params.nbRegisters = 8;
        params.nbProgramConstant = 5;
        e = std::make_shared<Environment>(set, params, vect);
        programAgent =
            std::make_shared<Algorithm::LGP::LGPAgent>(e, *lgpOutput, "fake");
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


TEST_F(LineMutatorTest, LineMutatorInitRandomCorrectLine1)
{
    RNG::RNG rng;
    rng.setSeed(0);

    // Add a pseudo-random lines to the program
    programAgent->addNewLine();
    Algorithm::LGP::LGPLine& l0 = programAgent->getLineForMutation(0);
    ASSERT_NO_THROW(lineMutator.initRandomCorrectLine(l0, rng))
        << "Pseudo-Random correct line initialization failed within an "
           "environment where failure should not be possible.";
    // With this known seed
    // InstructionIndex=3 > lambda instruction (plus)
    // DestinationIndex=6
    // Operand 0= (0, 12) => 12th register
    // Covers: correct instruction, correct operand type (register), additional
    // uneeded operand (not register)
    ASSERT_EQ(l0.getInstructionIndex(), 3)
        << "Selected pseudo-random instructionIndex changed with a known seed.";

    ASSERT_EQ(l0.getDestinationIndex(), 6)
        << "Selected pseudo-random destinationIndex changed with a known seed.";

    ASSERT_EQ(l0.getOperand(0).first, 0)
        << "Selected pseudo-random operand data source index changed with a "
           "known seed.";

    ASSERT_EQ(l0.getOperand(0).second, 12)
        << "Selected pseudo-random operand location changed with a known seed.";

    // Add another pseudo-random lines to the program
    programAgent->addNewLine();
    Algorithm::LGP::LGPLine& l1 = programAgent->getLineForMutation(1);

    // Additionally covers correct operand type from data source
    // Instruction if lambda instruction(plus)
    // first operand is register 0
    ASSERT_NO_THROW(lineMutator.initRandomCorrectLine(l1, rng))
        << "Pseudo-Random correct line initialization failed within an "
           "environment where failure should not be possible.";
    ASSERT_EQ(l1.getInstructionIndex(), 3)
        << "Selected pseudo-random instructionIndex changed with a known seed.";
    ASSERT_EQ(l1.getOperand(0).first, 0)
        << "Selected pseudo-random operand data source index changed with a "
           "known seed.";

    // Add another pseudo-random lines to the program
    // Additionally covers nothing
    programAgent->addNewLine();
    Algorithm::LGP::LGPLine& l2 = programAgent->getLineForMutation(2);
    programAgent->addNewLine();
    Algorithm::LGP::LGPLine& l3 = programAgent->getLineForMutation(3);

    ASSERT_NO_THROW(lineMutator.initRandomCorrectLine(l2, rng))
        << "Pseudo-Random correct line initialization failed within an "
           "environment where failure should not be possible.";
    ASSERT_NO_THROW(lineMutator.initRandomCorrectLine(l3, rng))
        << "Pseudo-Random correct line initialization failed within an "
           "environment where failure should not be possible.";

    // Add another pseudo-random lines to the program
    programAgent->addNewLine();
    Algorithm::LGP::LGPLine& l4 = programAgent->getLineForMutation(4);
    // Additionally covers additional uneeded operand (register)
    ASSERT_NO_THROW(lineMutator.initRandomCorrectLine(l4, rng))
        << "Pseudo-Random correct line initialization failed within an "
           "environment where failure should not be possible.";
    ASSERT_EQ(l4.getInstructionIndex(), 3)
        << "Selected pseudo-random instructionIndex changed with a known seed.";
    ASSERT_EQ(l4.getOperand(1).first, 3)
        << "Selected pseudo-random operand data source index changed with a "
           "known seed.";

    Output::OutputHandler output = Output::OutputHandler(Output::Output());
    Algorithm::LGP::LGPExecutionEngine lgpExecutionEngine(programAgent);


    ASSERT_NO_THROW(lgpExecutionEngine.execute())
        << "Program with only correct random lines is unexpectedly not "
           "correct.";

}

TEST_F(LineMutatorTest, LineMutatorAlterLine)
{
    RNG::RNG rng;
    Algorithm::LGP::LGPExecutionEngine lgpExecutionEngine(programAgent);

    // Add a 0 lines to the program
    // i=0, d=0, op0=(0,0), op1=(0,0)
    programAgent->addNewLine();
    Algorithm::LGP::LGPLine& l0 = programAgent->getLineForMutation(0);

    // Alter instruction
    // i=, d=0, op0=(0,0), op1=(0,0)
    rng.setSeed(5);
    ASSERT_NO_THROW(lineMutator.alterCorrectLine(l0, rng))
        << "Line mutation of a correct instruction should not throw.";
    ASSERT_EQ(l0.getInstructionIndex(), 2)
        << "Alteration with known seed changed its result.";
    ASSERT_NO_THROW(lgpExecutionEngine.execute()) << "Altered line is not executable.";

    // Alter destination
    // i=2, d=3, op0=(0,0), op1=(0,0)
    rng.setSeed(29);
    ASSERT_NO_THROW(lineMutator.alterCorrectLine(l0, rng))
        << "Line mutation of a correct instruction should not throw.";
    ASSERT_EQ(l0.getDestinationIndex(), 3)
        << "Alteration with known seed changed its result.";
    ASSERT_NO_THROW(lgpExecutionEngine.execute()) << "Altered line is not executable.";

    // Alter operand 0 data source
    // i=2, d=3, op0=(0,0), op1=(0,0)
    rng.setSeed(8);
    ASSERT_NO_THROW(lineMutator.alterCorrectLine(l0, rng))
        << "Line mutation of a correct instruction should not throw.";
    ASSERT_EQ(l0.getOperand(0).first, 3)
        << "Alteration with known seed changed its result.";
    ASSERT_NO_THROW(lgpExecutionEngine.execute()) << "Altered line is not executable.";

    // Alter operand 0 location
    // i=2, d=3, op0=(0,17), op1=(0,0)
    rng.setSeed(1);
    ASSERT_NO_THROW(lineMutator.alterCorrectLine(l0, rng))
        << "Line mutation of a correct instruction should not throw.";
    ASSERT_EQ(l0.getOperand(0).second, 17)
        << "Alteration with known seed changed its result.";
    ASSERT_NO_THROW(lgpExecutionEngine.execute()) << "Altered line is not executable.";

    // Alter operand 1 data source
    // i=2, d=3, op0=(0,17), op1=(8,0)
    rng.setSeed(320);
    ASSERT_NO_THROW(lineMutator.alterCorrectLine(l0, rng))
        << "Line mutation of a correct instruction should not throw.";
    ASSERT_EQ(l0.getOperand(1).first, 0)
        << "Alteration with known seed changed its result.";
    ASSERT_NO_THROW(lgpExecutionEngine.execute()) << "Altered line is not executable.";

    // Alter operand 1 location
    // i=2, d=3, op0=(0,17), op1=(0,8)
    rng.setSeed(2);
    ASSERT_NO_THROW(lineMutator.alterCorrectLine(l0, rng))
        << "Line mutation of a correct instruction should not throw.";
    ASSERT_EQ(l0.getOperand(1).second, 28)
        << "Alteration with known seed changed its result.";
    ASSERT_NO_THROW(lgpExecutionEngine.execute()) << "Altered line is not executable.";

    // Alter instruction index
    // i=4, d=0, op0=(0,28), op1=(0,8)
    rng.setSeed(6);
    ASSERT_NO_THROW(lineMutator.alterCorrectLine(l0, rng))
        << "Line mutation of a correct instruction should not throw.";
    ASSERT_EQ(l0.getInstructionIndex(), 2)
        << "Alteration with known seed changed its result.";
    ASSERT_EQ(l0.getDestinationIndex(), 4)
        << "Alteration with known seed changed its result.";
    ASSERT_EQ(l0.getOperand(0).first, 3)
        << "Alteration with known seed changed its result.";
    ASSERT_EQ(l0.getOperand(0).second, 17)
        << "Alteration with known seed changed its result.";
    ASSERT_EQ(l0.getOperand(1).first, 0)
        << "Alteration with known seed changed its result.";
    ASSERT_EQ(l0.getOperand(1).second, 28)
        << "Alteration with known seed changed its result.";
    ASSERT_NO_THROW(lgpExecutionEngine.execute()) << "Altered line is not executable.";
}

TEST_F(LineMutatorTest, LineMutatorAlterLineWithCompositeOperands)
{
    RNG::RNG rng;

    // Setup for this test
    set.add(
        *(new Instructions::LambdaInstruction<const double[3], const double[3]>(
            [](const double* a, const double* b) -> double {
                return (a[0] - b[0] + a[1] - b[1] + a[2] - b[2]) / 3.0;
            })));
    std::shared_ptr<const Environment> e2 = std::make_shared<Environment>(set, params, vect);
    std::shared_ptr<Algorithm::LGP::LGPAgent> programAgent2 = std::make_shared<Algorithm::LGP::LGPAgent>(e2, *lgpOutput, "fake2");

    Algorithm::LGP::LGPExecutionEngine lgpExecutionEngine(programAgent2);

    // Add a 0 line to the program
    // i=0, d=0, op0=(0,0), op1=(0,0)
    programAgent2->addNewLine();
    Algorithm::LGP::LGPLine& l0 = programAgent2->getLineForMutation(0);

    // Alter instruction
    // i=2, d=0, op0=(0,0), op1=(0,0)
    rng.setSeed(5);
    ASSERT_NO_THROW(lineMutator.alterCorrectLine(l0, rng))
        << "Line mutation of a correct instruction should not throw.";
    ASSERT_EQ(l0.getInstructionIndex(), 1)
        << "Alteration with known seed changed its result.";
    ASSERT_NO_THROW(lgpExecutionEngine.execute()) << "Altered line is not executable.";

    // Alter op1 location
    // i=2, d=0, op0=(0,0), op1=(0,16),  param=0
    rng.setSeed(4);
    ASSERT_NO_THROW(lineMutator.alterCorrectLine(l0, rng))
        << "Line mutation of a correct instruction should not throw.";
    ASSERT_EQ(l0.getOperand(1).second, 16)
        << "Alteration with known seed changed its result.";
    ASSERT_NO_THROW(lgpExecutionEngine.execute()) << "Altered line is not executable.";

    // Alter op0 source
    // i=2, d=0, op0=(3,0), op1=(0,16),  param=0
    rng.setSeed(3);
    ASSERT_NO_THROW(lineMutator.alterCorrectLine(l0, rng))
        << "Line mutation of a correct instruction should not throw.";
    ASSERT_EQ(l0.getOperand(0).first, 3)
        << "Alteration with known seed changed its result.";
    ASSERT_NO_THROW(lgpExecutionEngine.execute()) << "Altered line is not executable.";

    // Teardown for this test
    delete &set.getInstruction(4);
}