#include <gtest/gtest.h>

#include <set>
#include <algorithm>

#include "environment.h"
#include "instructions/instruction.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/multByConstParam.h"
#include "dataHandlers/dataHandler.h"
#include "dataHandlers/primitiveTypeArray.h"
#include "program/program.h"
#include "program/line.h"
#include "program/programExecutionEngine.h"
#include "tpg/tpgGraph.h"
#include "mutator/rng.h"
#include "mutator/lineMutator.h"
#include "mutator/programMutator.h"
#include "mutator/tpgMutator.h"

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

	ASSERT_EQ(Mutator::RNG::getDouble(0, 1.0), 0.99214520962982877) << "Returned pseudo-random value changed with a known seed.";
}

TEST_F(MutatorTest, LineMutatorInitRandomCorrectLine1) {
	Mutator::RNG::setSeed(0);

	// Add a pseudo-random lines to the program
	Program::Line& l0 = p->addNewLine();
	ASSERT_NO_THROW(Mutator::LineMutator::initRandomCorrectLine(l0)) << "Pseudo-Random correct line initialization failed within an environment where failure should not be possible.";
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
	ASSERT_NO_THROW(Mutator::LineMutator::initRandomCorrectLine(l1)) << "Pseudo-Random correct line initialization failed within an environment where failure should not be possible.";
	ASSERT_EQ(l1.getInstructionIndex(), 1) << "Selected pseudo-random instructionIndex changed with a known seed.";
	ASSERT_EQ(l1.getOperand(0).first, 2) << "Selected pseudo-random operand data source index changed with a known seed.";

	// Add another pseudo-random lines to the program
	// Additionally covers nothing 
	Program::Line& l2 = p->addNewLine();
	Program::Line& l3 = p->addNewLine();
	ASSERT_NO_THROW(Mutator::LineMutator::initRandomCorrectLine(l2)) << "Pseudo-Random correct line initialization failed within an environment where failure should not be possible.";
	ASSERT_NO_THROW(Mutator::LineMutator::initRandomCorrectLine(l3)) << "Pseudo-Random correct line initialization failed within an environment where failure should not be possible.";

	// Add another pseudo-random lines to the program
	Program::Line& l4 = p->addNewLine();
	// Additionally covers additional uneeded operand (register) 
	ASSERT_NO_THROW(Mutator::LineMutator::initRandomCorrectLine(l4)) << "Pseudo-Random correct line initialization failed within an environment where failure should not be possible.";
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
	ASSERT_NO_THROW(Mutator::LineMutator::initRandomCorrectLine(l0)) << "Pseudo-Random correct line initialization failed within an environment where failure should not be possible.";

	delete (&set.getInstruction(2));
}

TEST_F(MutatorTest, LineMutatorInitRandomCorrectLineException) {
	// Create a new set only with only non-usable instructions.
	Instructions::Set faultySet;

	// Add a new instruction for which no data can be found in the environment DataHandler
	faultySet.add(*(new Instructions::AddPrimitiveType<unsigned char>()));
	faultySet.add(*(new Instructions::MultByConstParam<float, int16_t>()));

	// Recreate the environment and program with the new faulty set
	delete p;
	delete e;
	e = new Environment(faultySet, vect, 8);
	p = new Program::Program(*e);

	// Set seed 
	Mutator::RNG::setSeed(0);

	// Add a pseudo-random lines to the program
	Program::Line& l0 = p->addNewLine();
	ASSERT_THROW(Mutator::LineMutator::initRandomCorrectLine(l0), std::runtime_error) << "Pseudo-Random correct line initialization did not fail within an environment where failure should always happen because instruction set and data sources have no compatible types.";

	// cleanup
	delete (&faultySet.getInstruction(0));
	delete (&faultySet.getInstruction(1));
}

TEST_F(MutatorTest, LineMutatorAlterLine) {
	Program::ProgramExecutionEngine pEE(*p);

	// Add a 0 lines to the program
	// i=0, d=0, op0=(0,0), op1=(0,0),  param=0
	Program::Line& l0 = p->addNewLine();

	// Alter instruction
	// i=1, d=0, op0=(0,0), op1=(0,0),  param=0
	Mutator::RNG::setSeed(5);
	ASSERT_NO_THROW(Mutator::LineMutator::alterCorrectLine(l0)) << "Line mutation of a correct instruction should not throw.";
	ASSERT_EQ(l0.getInstructionIndex(), 1) << "Alteration with known seed changed its result.";
	ASSERT_NO_THROW(pEE.executeProgram()) << "Altered line is not executable.";

	// Alter destination
	// i=1, d=3, op0=(0,0), op1=(0,0),  param=0
	Mutator::RNG::setSeed(33);
	ASSERT_NO_THROW(Mutator::LineMutator::alterCorrectLine(l0)) << "Line mutation of a correct instruction should not throw.";
	ASSERT_EQ(l0.getDestinationIndex(), 3) << "Alteration with known seed changed its result.";
	ASSERT_NO_THROW(pEE.executeProgram()) << "Altered line is not executable.";

	// Alter operand 0 data source 
	// i=1, d=3, op0=(2,0), op1=(0,0),  param=0
	Mutator::RNG::setSeed(12);
	ASSERT_NO_THROW(Mutator::LineMutator::alterCorrectLine(l0)) << "Line mutation of a correct instruction should not throw.";
	ASSERT_EQ(l0.getOperand(0).first, 2) << "Alteration with known seed changed its result.";
	ASSERT_NO_THROW(pEE.executeProgram()) << "Altered line is not executable.";

	// Alter operand 0 location
	// i=1, d=3, op0=(2,14), op1=(0,0),  param=0
	Mutator::RNG::setSeed(7);
	ASSERT_NO_THROW(Mutator::LineMutator::alterCorrectLine(l0)) << "Line mutation of a correct instruction should not throw.";
	ASSERT_EQ(l0.getOperand(0).second, 14) << "Alteration with known seed changed its result.";
	ASSERT_NO_THROW(pEE.executeProgram()) << "Altered line is not executable.";


	// Alter operand 1 data source
	// i=1, d=3, op0=(2,14), op1=(1,0),  param=0
	Mutator::RNG::setSeed(323);
	ASSERT_NO_THROW(Mutator::LineMutator::alterCorrectLine(l0)) << "Line mutation of a correct instruction should not throw.";
	ASSERT_EQ(l0.getOperand(1).first, 1) << "Alteration with known seed changed its result.";
	ASSERT_NO_THROW(pEE.executeProgram()) << "Altered line is not executable.";


	// Alter operand 1 location
	// i=1, d=6, op0=(2,14), op1=(1,28),  param=0
	Mutator::RNG::setSeed(2);
	ASSERT_NO_THROW(Mutator::LineMutator::alterCorrectLine(l0)) << "Line mutation of a correct instruction should not throw.";
	ASSERT_EQ(l0.getOperand(1).second, 28) << "Alteration with known seed changed its result.";
	ASSERT_NO_THROW(pEE.executeProgram()) << "Altered line is not executable.";

	// Alter parameter 0
	// i=1, d=6, op0=(2,14), op1=(1,28),  param=31115
	Mutator::RNG::setSeed(0);
	ASSERT_NO_THROW(Mutator::LineMutator::alterCorrectLine(l0)) << "Line mutation of a correct instruction should not throw.";
	ASSERT_EQ((int16_t)l0.getParameter(0), 31115) << "Alteration with known seed changed its result.";
	ASSERT_NO_THROW(pEE.executeProgram()) << "Altered line is not executable.";

	// Alter instruction (causing an alteration of op1 data source)
	// i=0, d=6, op0=(2,14), op1=(0,28),  param=31115
	Mutator::RNG::setSeed(5);
	ASSERT_NO_THROW(Mutator::LineMutator::alterCorrectLine(l0)) << "Line mutation of a correct instruction should not throw.";
	ASSERT_EQ(l0.getInstructionIndex(), 0) << "Alteration with known seed changed its result.";
	ASSERT_EQ(l0.getDestinationIndex(), 3) << "Alteration with known seed changed its result.";
	ASSERT_EQ(l0.getOperand(0).first, 2) << "Alteration with known seed changed its result.";
	ASSERT_EQ(l0.getOperand(0).second, 14) << "Alteration with known seed changed its result.";
	ASSERT_EQ(l0.getOperand(1).first, 0) << "Alteration with known seed changed its result.";
	ASSERT_EQ(l0.getOperand(1).second, 28) << "Alteration with known seed changed its result.";
	ASSERT_EQ((int16_t)l0.getParameter(0), 31115) << "Alteration with known seed changed its result.";
	ASSERT_NO_THROW(pEE.executeProgram()) << "Altered line is not executable.";
}

TEST_F(MutatorTest, ProgramMutatorDeleteRandomLine) {
	const uint64_t nbLines = 10;

	// Attempt removing on an empty program
	ASSERT_FALSE(Mutator::ProgramMutator::deleteRandomLine(*p));
	ASSERT_EQ(p->getNbLines(), 0);

	// Attempt removing on a program with a single line
	p->addNewLine();
	ASSERT_FALSE(Mutator::ProgramMutator::deleteRandomLine(*p));
	ASSERT_EQ(p->getNbLines(), 1);

	// Insert lines
	for (auto i = 0; i < nbLines - 1; i++) {
		p->addNewLine();
	}

	// Delete a random line
	ASSERT_TRUE(Mutator::ProgramMutator::deleteRandomLine(*p));
	ASSERT_EQ(p->getNbLines(), nbLines - 1);
}

TEST_F(MutatorTest, ProgramMutatorInsertRandomLine) {
	Mutator::RNG::setSeed(0);

	// Insert in empty program
	ASSERT_NO_THROW(Mutator::ProgramMutator::insertRandomLine(*p));
	ASSERT_EQ(p->getNbLines(), 1) << "Line insertion in an empty program failed.";
	ASSERT_EQ((int16_t)p->getLine(0).getParameter(0), 31115) << "Inserted random line is not random. (with a known seed).";

	// Insert in non empty program
	// in first position (with known seed)
	Mutator::RNG::setSeed(0);
	ASSERT_NO_THROW(Mutator::ProgramMutator::insertRandomLine(*p));
	ASSERT_EQ(p->getNbLines(), 2) << "Line insertion in a non-empty program failed.";
	// Just to ensure the position of the inserted line is the first
	ASSERT_EQ((int16_t)p->getLine(0).getParameter(0), 26809);
	ASSERT_EQ((int16_t)p->getLine(1).getParameter(0), 31115);

	// Insert in non empty program
	// After last position (with known seed)
	Mutator::RNG::setSeed(1);
	ASSERT_NO_THROW(Mutator::ProgramMutator::insertRandomLine(*p));
	ASSERT_EQ(p->getNbLines(), 3) << "Line insertion in a non-empty program failed.";
	// Just to ensure the position of the inserted line is after the last
	ASSERT_EQ((int16_t)p->getLine(0).getParameter(0), 26809);
	ASSERT_EQ((int16_t)p->getLine(1).getParameter(0), 31115);
	ASSERT_EQ((int16_t)p->getLine(2).getParameter(0), -14950);

	// Insert in non empty program
	// In the middle position (with known seed)
	Mutator::RNG::setSeed(5);
	ASSERT_NO_THROW(Mutator::ProgramMutator::insertRandomLine(*p));
	ASSERT_EQ(p->getNbLines(), 4) << "Line insertion in a non-empty program failed.";
	// Just to ensure the position of the inserted line is after the last
	ASSERT_EQ((int16_t)p->getLine(0).getParameter(0), 26809);
	ASSERT_EQ((int16_t)p->getLine(1).getParameter(0), 31115);
	ASSERT_EQ((int16_t)p->getLine(2).getParameter(0), -17304);
	ASSERT_EQ((int16_t)p->getLine(3).getParameter(0), -14950);
}

TEST_F(MutatorTest, ProgramMutatorSwapRandomLines) {
	Mutator::RNG::setSeed(0);

	std::vector<Program::Line*> lines;
	// Nothing on empty program
	ASSERT_FALSE(Mutator::ProgramMutator::swapRandomLines(*p));

	// Add a first line
	lines.push_back(&p->addNewLine());

	// Nothing on program with one line.
	ASSERT_FALSE(Mutator::ProgramMutator::swapRandomLines(*p));

	// Add a second line
	lines.push_back(&p->addNewLine());

	// Exchanges the two line.
	ASSERT_TRUE(Mutator::ProgramMutator::swapRandomLines(*p));
	ASSERT_EQ(lines.at(0), &p->getLine(1));
	ASSERT_EQ(lines.at(1), &p->getLine(0));

	// Add 8 lines
	for (auto i = 0; i < 8; i++) {
		lines.push_back(&p->addNewLine());
	}
	// Swap two random lines (with a known seed)
	ASSERT_TRUE(Mutator::ProgramMutator::swapRandomLines(*p));
	// Only lines 4 and 7 are swapped
	ASSERT_EQ(lines.at(0), &p->getLine(1));
	ASSERT_EQ(lines.at(1), &p->getLine(0));
	ASSERT_EQ(lines.at(2), &p->getLine(2));
	ASSERT_EQ(lines.at(3), &p->getLine(3));
	ASSERT_EQ(lines.at(4), &p->getLine(7));
	ASSERT_EQ(lines.at(5), &p->getLine(5));
	ASSERT_EQ(lines.at(6), &p->getLine(6));
	ASSERT_EQ(lines.at(7), &p->getLine(4));
	ASSERT_EQ(lines.at(8), &p->getLine(8));
	ASSERT_EQ(lines.at(9), &p->getLine(9));
}

TEST_F(MutatorTest, ProgramMutatorAlterRandomLine) {
	Mutator::RNG::setSeed(0);

	// Nothing on empty program
	ASSERT_FALSE(Mutator::ProgramMutator::alterRandomLine(*p));
	// Add 10 lines
	for (auto i = 0; i < 10; i++) {
		p->addNewLine();
	}
	// Alter a randomly selected line (with a known seed)
	// Parameter of Line 4 is altered.
	ASSERT_TRUE(Mutator::ProgramMutator::alterRandomLine(*p));
	ASSERT_EQ((int16_t)p->getLine(4).getParameter(0), 26809);
}

TEST_F(MutatorTest, ProgramMutatorInitProgram) {
	Mutator::RNG::setSeed(0);

	ASSERT_NO_THROW(Mutator::ProgramMutator::initRandomProgram(*p, 96)) << "Empty Program Random init failed";
	ASSERT_EQ(p->getNbLines(), 31) << "Random number of line is not as expected (with known seed).";

	ASSERT_NO_THROW(Mutator::ProgramMutator::initRandomProgram(*p, 96)) << "Non-Empty Program Random init failed";
	ASSERT_EQ(p->getNbLines(), 53) << "Random number of line is not as expected (with known seed).";
}

TEST_F(MutatorTest, ProgramMutatorMutateBehavior) {

	// Add 3 lines
	p->addNewLine();
	p->addNewLine();
	p->addNewLine();

	Mutator::RNG::setSeed(0);
	ASSERT_TRUE(Mutator::ProgramMutator::mutateProgram(*p, 0.5, 0.0, 15, 0.0, 0.0)) << "Mutation did not occur with known seed.";
	ASSERT_EQ(p->getNbLines(), 2) << "Wrong program mutation occured. Expected: Line deletion.";

	Mutator::RNG::setSeed(1);
	ASSERT_TRUE(Mutator::ProgramMutator::mutateProgram(*p, 0.0, 0.5, 15, 0.0, 0.0)) << "Mutation did not occur with known seed.";
	ASSERT_EQ(p->getNbLines(), 3) << "Wrong program mutation occured. Expected: Line insertion.";

	Mutator::RNG::setSeed(86);
	ASSERT_TRUE(Mutator::ProgramMutator::mutateProgram(*p, 0.0, 0.0, 15, 0.01, 0.0)) << "Mutation did not occur with known seed.";

	Mutator::RNG::setSeed(1);
	ASSERT_TRUE(Mutator::ProgramMutator::mutateProgram(*p, 0.0, 0.0, 15, 0.00, 0.1)) << "Mutation did not occur with known seed.";
}

TEST_F(MutatorTest, TPGMutatorInitProgram) {
	TPG::TPGGraph tpg(*e);
	Mutator::MutationParameters params;

	params.tpg.nbActions = 5;
	params.tpg.maxInitOutgoingEdges = 4;
	params.prog.maxProgramSize = 96;

	ASSERT_NO_THROW(Mutator::TPGMutator::initRandomTPG(tpg, params)) << "TPG Initialization failed.";
	auto vertexSet = tpg.getVertices();
	// Check number or vertex, roots, actions, teams, edges
	ASSERT_EQ(vertexSet.size(), 2 * params.tpg.nbActions) << "Number of vertices after initialization is incorrect.";
	ASSERT_EQ(tpg.getRootVertices().size(), params.tpg.nbActions) << "Number of root vertices after initialization is incorrect.";
	ASSERT_EQ(std::count_if(vertexSet.begin(), vertexSet.end(),
		[](const TPG::TPGVertex* vert) {
			return typeid(*vert) == typeid(TPG::TPGAction);
		}), params.tpg.nbActions) << "Number of action vertex in the graph is incorrect.";
	ASSERT_EQ(std::count_if(vertexSet.begin(), vertexSet.end(),
		[](const TPG::TPGVertex* vert) {
			return typeid(*vert) == typeid(TPG::TPGTeam);
		}), params.tpg.nbActions) << "Number of team vertex in the graph is incorrect.";
	ASSERT_GE(tpg.getEdges().size(), 2 * params.tpg.nbActions) << "Insufficient number of edges in the initialized TPG.";
	ASSERT_LE(tpg.getEdges().size(), params.tpg.nbActions * params.tpg.maxInitOutgoingEdges) << "Too many edges in the initialized TPG.";

	// Check number of Programs.
	std::set<Program::Program*> programs;
	std::for_each(tpg.getEdges().begin(), tpg.getEdges().end(),
		[&programs](const TPG::TPGEdge& edge) {
			programs.insert(&edge.getProgram());
		});
	ASSERT_EQ(programs.size(), params.tpg.nbActions * 2) << "Number of distinct program in the TPG is incorrect.";
	// Check that no team has the same program twice
	for (auto team : tpg.getRootVertices()) {
		std::set<Program::Program*> teamPrograms;
		std::for_each(team->getOutgoingEdges().begin(), team->getOutgoingEdges().end(),
			[&teamPrograms](const TPG::TPGEdge* edge) {
				teamPrograms.insert(&edge->getProgram());
			});
		ASSERT_EQ(teamPrograms.size(), team->getOutgoingEdges().size()) << "A team is connected to the same program twice.";
	}
}