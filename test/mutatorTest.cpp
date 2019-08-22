#include <gtest/gtest.h>

#include "environment.h"
#include "instructions/instruction.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/multByConstParam.h"
#include "dataHandlers/dataHandler.h"
#include "dataHandlers/primitiveTypeArray.h"
#include "program/program.h"
#include "program/line.h"
#include "program/programExecutionEngine.h"
#include "mutator/rng.h"
#include "mutator/lineMutator.h"

class MutatorTest : public ::testing::Test {
protected:
	const size_t size1{ 24 };
	const size_t size2{ 32 };
	const double value0{ 2.3 };
	const float value1{ 4.2f };
	std::vector<std::reference_wrapper<DataHandlers::DataHandler>> vect;
	Instructions::Set set;
	Environment* e;
	Program::Program* p;

	MutatorTest() : e{ nullptr }, p{ nullptr }{};

	virtual void SetUp() {
		vect.push_back(*(new DataHandlers::PrimitiveTypeArray<int>((unsigned int)size1)));
		vect.push_back(*(new DataHandlers::PrimitiveTypeArray<double>((unsigned int)size2)));

		((DataHandlers::PrimitiveTypeArray<double>&)vect.at(1).get()).setDataAt(typeid(PrimitiveType<double>), 25, value0);

		set.add(*(new Instructions::AddPrimitiveType<double>()));
		set.add(*(new Instructions::MultByConstParam<double, float>()));

		e = new Environment(set, vect, 8);
		p = new Program::Program(*e);
	}

	virtual void TearDown() {
		delete p;
		delete e;
		delete (&(vect.at(0).get()));
		delete (&(vect.at(1).get()));
		delete (&set.getInstruction(0));
		delete (&set.getInstruction(1));
	}
};

TEST_F(MutatorTest, RNG) {
	Mutator::RNG::setSeed(0);

	// With this seed, the current pseudo-random number generator returns 24 
	// on its first use
	ASSERT_EQ(Mutator::RNG::getUnsignedInt64(0, 100), 24) << "Returned pseudo-random value changed with a known seed.";
}

TEST_F(MutatorTest, LineMutatorInitRandomCorrectLine1) {
	Mutator::RNG::setSeed(0);

	// Add a pseudo-random lines to the program
	Program::Line& l0 = p->addNewLine();
	ASSERT_NO_THROW(Mutator::Line::initRandomCorrectLine(l0)) << "Pseudo-Random correct line initialization failed within an environment where failure should not be possible.";
	// With this known seed
	// InstructionIndex=1 > MultByConst<double, float>
	// DestinationIndex=6
	// Operand 0= (0, 4) => 5th register
	// Covers: correct instruction, correct operand type (register), additional uneeded operand (not register)
	ASSERT_EQ(l0.getInstructionIndex(), 1) << "Selected pseudo-random instructionIndex changed with a known seed.";
	ASSERT_EQ(l0.getDestinationIndex(), 6) << "Selected pseudo-random destinationIndex changed with a known seed.";
	ASSERT_EQ(l0.getOperand(0).first, 0) << "Selected pseudo-random operand data source index changed with a known seed.";
	ASSERT_EQ(l0.getOperand(0).second, 12) << "Selected pseudo-random operand location changed with a known seed.";

	// Add another pseudo-random lines to the program
	Program::Line& l1 = p->addNewLine();
	// Additionally covers correct operand type from data source
	// Instruction if MultByConst<double, float>
	// first operand is PrimitiveTypeArray<double>
	ASSERT_NO_THROW(Mutator::Line::initRandomCorrectLine(l1)) << "Pseudo-Random correct line initialization failed within an environment where failure should not be possible.";
	ASSERT_EQ(l1.getInstructionIndex(), 1) << "Selected pseudo-random instructionIndex changed with a known seed.";
	ASSERT_EQ(l1.getOperand(0).first, 2) << "Selected pseudo-random operand data source index changed with a known seed.";

	// Add another pseudo-random lines to the program
	// Additionally covers nothing 
	Program::Line& l2 = p->addNewLine();
	Program::Line& l3 = p->addNewLine();
	ASSERT_NO_THROW(Mutator::Line::initRandomCorrectLine(l2)) << "Pseudo-Random correct line initialization failed within an environment where failure should not be possible.";
	ASSERT_NO_THROW(Mutator::Line::initRandomCorrectLine(l3)) << "Pseudo-Random correct line initialization failed within an environment where failure should not be possible.";

	// Add another pseudo-random lines to the program
	Program::Line& l4 = p->addNewLine();
	// Additionally covers additional uneeded operand (register) 
	ASSERT_NO_THROW(Mutator::Line::initRandomCorrectLine(l4)) << "Pseudo-Random correct line initialization failed within an environment where failure should not be possible.";
	ASSERT_EQ(l4.getInstructionIndex(), 1) << "Selected pseudo-random instructionIndex changed with a known seed.";
	ASSERT_EQ(l4.getOperand(1).first, 0) << "Selected pseudo-random operand data source index changed with a known seed.";

	Program::ProgramExecutionEngine progEngine(*p);
	ASSERT_NO_THROW(progEngine.executeProgram(false)) << "Program with only correct random lines is unexpectedly not correct.";
}

TEST_F(MutatorTest, LineMutatorInitRandomCorrectLine2) {
	// Add a new instruction for which no data can be found in the environment DataHandler
	set.add(*(new Instructions::AddPrimitiveType<unsigned char>()));

	// Recreate the environment and program with the new set
	delete p;
	delete e;
	e = new Environment(set, vect, 8);
	p = new Program::Program(*e);

	// Set seed to cover the case where the instruction with no compatible dataSource is selected.
	Mutator::RNG::setSeed(5);

	// Add a pseudo-random lines to the program
	Program::Line& l0 = p->addNewLine();
	ASSERT_NO_THROW(Mutator::Line::initRandomCorrectLine(l0)) << "Pseudo-Random correct line initialization failed within an environment where failure should not be possible.";

	delete (&set.getInstruction(2));
}

TEST_F(MutatorTest, LineMutatorInitRandomCorrectLineException) {
	// Create a new set only with only non-usable instructions.
	Instructions::Set faultySet;

	// Add a new instruction for which no data can be found in the environment DataHandler
	faultySet.add(*(new Instructions::AddPrimitiveType<unsigned char>()));
	faultySet.add(*(new Instructions::MultByConstParam<float, int>()));

	// Recreate the environment and program with the new faulty set
	delete p;
	delete e;
	e = new Environment(faultySet, vect, 8);
	p = new Program::Program(*e);

	// Set seed 
	Mutator::RNG::setSeed(0);

	// Add a pseudo-random lines to the program
	Program::Line& l0 = p->addNewLine();
	ASSERT_THROW(Mutator::Line::initRandomCorrectLine(l0), std::runtime_error) << "Pseudo-Random correct line initialization did not fail within an environment where failure should always happen because instruction set and data sources have no compatible types.";

	// cleanup
	delete (&faultySet.getInstruction(0));
	delete (&faultySet.getInstruction(1));
}

TEST_F(MutatorTest, LineMutatorAlterLine) {

	// Add a 0 lines to the program
	// i=0, d=0, op0=(0,0), op1=(0,0),  param=0
	Program::Line& l0 = p->addNewLine();
	
	// Alter instruction
	// i=1, d=0, op0=(0,0), op1=(0,0),  param=0
	Mutator::RNG::setSeed(31);
	ASSERT_NO_THROW(Mutator::Line::alterCorrectLine(l0)) << "Line mutation of a correct instruction should not throw.";
	ASSERT_EQ(l0.getInstructionIndex(), 1);

	// Alter destination
	// i=1, d=6, op0=(0,0), op1=(0,0),  param=0
	Mutator::RNG::setSeed(51);
	ASSERT_NO_THROW(Mutator::Line::alterCorrectLine(l0)) << "Line mutation of a correct instruction should not throw.";
	ASSERT_EQ(l0.getDestinationIndex(), 6);

	// Alter operand 0 data source 
	// i=1, d=6, op0=(2,0), op1=(0,0),  param=0
	Mutator::RNG::setSeed(60);
	ASSERT_NO_THROW(Mutator::Line::alterCorrectLine(l0)) << "Line mutation of a correct instruction should not throw.";
	ASSERT_EQ(l0.getOperand(0).first, 2);

	// Alter operand 0 location
	// i=1, d=6, op0=(2,18), op1=(0,0),  param=0
	Mutator::RNG::setSeed(17);
	ASSERT_NO_THROW(Mutator::Line::alterCorrectLine(l0)) << "Line mutation of a correct instruction should not throw.";
	ASSERT_EQ(l0.getOperand(0).second, 18);

	// Alter operand 1 data source
	// i=1, d=6, op0=(2,18), op1=(1,0),  param=0
	Mutator::RNG::setSeed(45);
	ASSERT_NO_THROW(Mutator::Line::alterCorrectLine(l0)) << "Line mutation of a correct instruction should not throw.";
	ASSERT_EQ(l0.getOperand(1).first, 1);

	// Alter operand 1 location
	// i=1, d=6, op0=(2,18), op1=(1,24),  param=0
	Mutator::RNG::setSeed(3);
	ASSERT_NO_THROW(Mutator::Line::alterCorrectLine(l0)) << "Line mutation of a correct instruction should not throw.";
	ASSERT_EQ(l0.getOperand(1).second, 24);

	// Alter parameter 0
	// i=1, d=6, op0=(2,18), op1=(1,24),  param=-1103038069
	Mutator::RNG::setSeed(0);
	ASSERT_NO_THROW(Mutator::Line::alterCorrectLine(l0)) << "Line mutation of a correct instruction should not throw.";
	ASSERT_EQ(l0.getParameter(0).i, -1103038069);

	// Alter instruction (causing an alteration of op1 data source)
	// i=0, d=6, op0=(2,18), op1=(2,24),  param=-1103038069
	Mutator::RNG::setSeed(31);
	ASSERT_NO_THROW(Mutator::Line::alterCorrectLine(l0)) << "Line mutation of a correct instruction should not throw.";
	ASSERT_EQ(l0.getInstructionIndex(), 0);
	ASSERT_EQ(l0.getOperand(1).first, 2);
}