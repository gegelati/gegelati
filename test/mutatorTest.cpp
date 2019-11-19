#include <gtest/gtest.h>

#include <set>
#include <vector>
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
#include "tpg/tpgExecutionEngine.h"
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
	std::shared_ptr<Program::Program> progPointer;

	MutatorTest() : e{ nullptr }, p{ nullptr }{};

	virtual void SetUp() {
		vect.push_back(*(new DataHandlers::PrimitiveTypeArray<int>((unsigned int)size1)));
		vect.push_back(*(new DataHandlers::PrimitiveTypeArray<double>((unsigned int)size2)));

		((DataHandlers::PrimitiveTypeArray<double>&)vect.at(1).get()).setDataAt(typeid(PrimitiveType<double>), 25, value0);

		set.add(*(new Instructions::AddPrimitiveType<double>()));
		set.add(*(new Instructions::MultByConstParam<double, float>()));

		e = new Environment(set, vect, 8);
		p = new Program::Program(*e);
		progPointer = std::shared_ptr<Program::Program>(new Program::Program(*e));
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

	Mutator::MutationParameters params;
	params.prog.maxProgramSize = 96;

	ASSERT_NO_THROW(Mutator::ProgramMutator::initRandomProgram(*p, params)) << "Empty Program Random init failed";
	ASSERT_EQ(p->getNbLines(), 31) << "Random number of line is not as expected (with known seed).";

	ASSERT_NO_THROW(Mutator::ProgramMutator::initRandomProgram(*p, params)) << "Non-Empty Program Random init failed";
	ASSERT_EQ(p->getNbLines(), 53) << "Random number of line is not as expected (with known seed).";

	// Count lines marked as introns (with a known seed).
	uint64_t nbIntrons = 0;
	for (auto i = 0; i < p->getNbLines(); i++) {
		if (p->isIntron(i)) {
			nbIntrons++;
		}
	}

	// Check nb intron lines with a known seed.
	ASSERT_EQ(nbIntrons, 51);
}

TEST_F(MutatorTest, ProgramMutatorMutateBehavior) {

	// Add 3 lines
	p->addNewLine();
	p->addNewLine();
	p->addNewLine();

	Mutator::MutationParameters params;
	params.prog.maxProgramSize = 15;
	params.prog.pDelete = 0.5;
	params.prog.pAdd = 0.0;
	params.prog.pMutate = 0.0;
	params.prog.pSwap = 0.0;

	Mutator::RNG::setSeed(0);
	ASSERT_TRUE(Mutator::ProgramMutator::mutateProgram(*p, params)) << "Mutation did not occur with known seed.";
	ASSERT_EQ(p->getNbLines(), 2) << "Wrong program mutation occured. Expected: Line deletion.";

	params.prog.pDelete = 0.0;
	params.prog.pAdd = 0.5;
	Mutator::RNG::setSeed(1);
	ASSERT_TRUE(Mutator::ProgramMutator::mutateProgram(*p, params)) << "Mutation did not occur with known seed.";
	ASSERT_EQ(p->getNbLines(), 3) << "Wrong program mutation occured. Expected: Line insertion.";

	params.prog.pAdd = 0.0;
	params.prog.pMutate = 0.01;
	Mutator::RNG::setSeed(86);
	ASSERT_TRUE(Mutator::ProgramMutator::mutateProgram(*p, params)) << "Mutation did not occur with known seed.";

	params.prog.pMutate = 0.00;
	params.prog.pSwap = 0.1;
	Mutator::RNG::setSeed(1);
	ASSERT_TRUE(Mutator::ProgramMutator::mutateProgram(*p, params)) << "Mutation did not occur with known seed.";
}

TEST_F(MutatorTest, TPGMutatorInitRandomTPG) {
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

	// Cover bad parameterization error
	params.tpg.maxInitOutgoingEdges = 6;
	ASSERT_THROW(Mutator::TPGMutator::initRandomTPG(tpg, params), std::runtime_error) << "TPG Initialization should fail with bad parameters.";
	params.tpg.maxInitOutgoingEdges = 0;
	params.tpg.nbActions = 1;
	ASSERT_THROW(Mutator::TPGMutator::initRandomTPG(tpg, params), std::runtime_error) << "TPG Initialization should fail with bad parameters.";
}


TEST_F(MutatorTest, TPGMutatorRemoveRandomEdge) {
	TPG::TPGGraph tpg(*e);
	const TPG::TPGTeam& vertex0 = tpg.addNewTeam();
	const TPG::TPGAction& vertex1 = tpg.addNewAction(0);
	const TPG::TPGTeam& vertex2 = tpg.addNewTeam();
	const TPG::TPGAction& vertex3 = tpg.addNewAction(1);
	const TPG::TPGEdge& edge0 = tpg.addNewEdge(vertex0, vertex1, progPointer);
	const TPG::TPGEdge& edge1 = tpg.addNewEdge(vertex0, vertex2, progPointer);
	const TPG::TPGEdge& edge2 = tpg.addNewEdge(vertex0, vertex3, progPointer);

	Mutator::RNG::setSeed(0);
	ASSERT_NO_THROW(Mutator::TPGMutator::removeRandomEdge(tpg, vertex0)) << "Removing a random edge failed unexpectedly.";
	// Check properties of the tpg
	ASSERT_EQ(tpg.getEdges().size(), 2) << "No edge was removed from the TPG.";
	// With known seed edge 0 was removed
	ASSERT_EQ(std::count_if(tpg.getEdges().begin(), tpg.getEdges().end(),
		[&edge0](const TPG::TPGEdge& other) {return &edge0 == &other; }), 0) << "With a known seed, edge0 should be removed from the TPG.";
	ASSERT_EQ(std::count_if(tpg.getEdges().begin(), tpg.getEdges().end(),
		[&edge1](const TPG::TPGEdge& other) {return &edge1 == &other; }), 1) << "With a known seed, edge1 should not be removed from the TPG.";
	ASSERT_EQ(std::count_if(tpg.getEdges().begin(), tpg.getEdges().end(),
		[&edge2](const TPG::TPGEdge& other) {return &edge2 == &other; }), 1) << "With a known seed, edge2 should not be removed from the TPG.";

	// Remove again to cover the "1 action remaining" code.
	ASSERT_NO_THROW(Mutator::TPGMutator::removeRandomEdge(tpg, vertex0)) << "Removing a random edge failed unexpectedly.";
	// Check properties of the tpg
	ASSERT_EQ(tpg.getEdges().size(), 1) << "No edge was removed from the TPG.";
	// Edge 1 was removed
	ASSERT_EQ(std::count_if(tpg.getEdges().begin(), tpg.getEdges().end(),
		[&edge1](const TPG::TPGEdge& other) {return &edge1 == &other; }), 0) << "With a known seed, edge1 should be removed from the TPG.";
	ASSERT_EQ(std::count_if(tpg.getEdges().begin(), tpg.getEdges().end(),
		[&edge2](const TPG::TPGEdge& other) {return &edge2 == &other; }), 1) << "With a known seed, edge2 should not be removed from the TPG.";
}

TEST_F(MutatorTest, TPGMutatorAddRandomEdge) {
	TPG::TPGGraph tpg(*e);
	const TPG::TPGTeam& vertex0 = tpg.addNewTeam();
	const TPG::TPGAction& vertex1 = tpg.addNewAction(0);
	const TPG::TPGTeam& vertex2 = tpg.addNewTeam();
	const TPG::TPGAction& vertex3 = tpg.addNewAction(1);
	const TPG::TPGAction& vertex4 = tpg.addNewAction(2);
	std::list<const TPG::TPGEdge*> edges;

	edges.push_back(&tpg.addNewEdge(vertex0, vertex1, progPointer));
	edges.push_back(&tpg.addNewEdge(vertex0, vertex2, progPointer));
	edges.push_back(&tpg.addNewEdge(vertex0, vertex3, progPointer));
	edges.push_back(&tpg.addNewEdge(vertex2, vertex4, progPointer));

	Mutator::RNG::setSeed(0);
	// Run the add
	ASSERT_NO_THROW(Mutator::TPGMutator::addRandomEdge(tpg, vertex2, edges)) << "Adding an edge to the TPG should succeed.";

	// Check properties of the tpg
	ASSERT_EQ(tpg.getEdges().size(), 5) << "No edge was added from the TPG.";
	ASSERT_EQ(vertex2.getOutgoingEdges().size(), 2) << "The random edge was not added to the right team.";

	// Edge was added with vertex1 (with known seed)
	ASSERT_EQ(vertex1.getIncomingEdges().size(), 2) << "The random edge was not added with the right (pseudo)random destination.";

	// Force a failure
	TPG::TPGEdge newEdge(&vertex0, &vertex1, progPointer);
	ASSERT_THROW(Mutator::TPGMutator::addRandomEdge(tpg, vertex2, { &newEdge }), std::runtime_error) << "Picking an edge not belonging to the graph should fail.";
}

TEST_F(MutatorTest, TPGMutatorMutateEdgeDestination) {
	TPG::TPGGraph tpg(*e);
	const TPG::TPGTeam& vertex0 = tpg.addNewTeam();
	const TPG::TPGAction& vertex1 = tpg.addNewAction(0);
	const TPG::TPGAction& vertex2 = tpg.addNewAction(1);
	const TPG::TPGTeam& vertex3 = tpg.addNewTeam();
	const TPG::TPGTeam& vertex4 = tpg.addNewTeam();

	const TPG::TPGEdge& edge0 = tpg.addNewEdge(vertex0, vertex1, progPointer);
	const TPG::TPGEdge& edge1 = tpg.addNewEdge(vertex0, vertex3, progPointer);

	Mutator::MutationParameters params;
	params.tpg.pEdgeDestinationIsAction = 0.5;

	Mutator::RNG::setSeed(2);
	ASSERT_NO_THROW(Mutator::TPGMutator::mutateEdgeDestination(tpg, vertex0, &edge1, { &vertex3, &vertex4 }, { &vertex1, &vertex2 }, params));
	// Check properties of the tpg
	ASSERT_EQ(tpg.getEdges().size(), 2) << "Number of edge should remain unchanged after destination change.";
	ASSERT_EQ(vertex0.getOutgoingEdges().size(), 2) << "The edge source should not be altered.";
	ASSERT_EQ(vertex3.getIncomingEdges().size(), 0) << "The edge Destination should be vertex4 (with known seed).";
	ASSERT_EQ(vertex4.getIncomingEdges().size(), 1) << "The edge Destination should be vertex4 (with known seed).";

	// Cover the only action case
	params.tpg.pEdgeDestinationIsAction = 0.0; // even with a probability of 0.
	ASSERT_NO_THROW(Mutator::TPGMutator::mutateEdgeDestination(tpg, vertex0, &edge0, { &vertex3, &vertex4 }, { &vertex2 }, params));
	ASSERT_EQ(vertex2.getIncomingEdges().size(), 1) << "The only choice of action given to the mutation should have been used.";
}

TEST_F(MutatorTest, TPGMutatorMutateOutgoingEdge) {
	// Init a TPG
	TPG::TPGGraph tpg(*e);
	const TPG::TPGTeam& vertex0 = tpg.addNewTeam();
	const TPG::TPGAction& vertex1 = tpg.addNewAction(0);
	const TPG::TPGEdge& edge0 = tpg.addNewEdge(vertex0, vertex1, progPointer);

	// Init its program and fill the archive
	Mutator::MutationParameters params;
	Archive arch;
	TPG::TPGExecutionEngine tee(*e, &arch);
	params.prog.maxProgramSize = 96;
	Mutator::ProgramMutator::initRandomProgram(*progPointer, params);
	tee.executeFromRoot(vertex0);

	// Mutate (params selected for code coverage)
	params.prog.pAdd = 0.5;
	params.prog.pDelete = 0.5;
	params.prog.pMutate = 1.0;
	params.prog.pSwap = 1.0;
	params.tpg.pEdgeDestinationChange = 1.0;

	ASSERT_NO_THROW(Mutator::TPGMutator::mutateOutgoingEdge(tpg, arch, vertex0, &edge0, { &vertex0 }, { &vertex1 }, params));
	// Check that progPointer use count was decreased since the mutated program is a copy of the original
	ASSERT_EQ(progPointer.use_count(), 1) << "Shared pointer should no longer be used inside the TPG after mutation.";
	// Verify new program uniqueness
	Program::ProgramExecutionEngine pee(edge0.getProgram());
	double result = pee.executeProgram();
	ASSERT_TRUE(arch.areProgramResultsUnique({ { arch.getCombinedHash(e->getDataSources()), result } })) << "Mutated program associated to the edge should return a unique bid on the environment.";
}

TEST_F(MutatorTest, TPGMutatorMutateTeam) {
	// Create a TPG
	TPG::TPGGraph tpg(*e);
	const TPG::TPGTeam& vertex0 = tpg.addNewTeam();
	const TPG::TPGAction& vertex1 = tpg.addNewAction(0);
	const TPG::TPGAction& vertex2 = tpg.addNewAction(1);
	const TPG::TPGEdge& edge0 = tpg.addNewEdge(vertex0, vertex1, progPointer);
	const TPG::TPGEdge& edge1 = tpg.addNewEdge(vertex0, vertex2, progPointer);
	const TPG::TPGAction& vertex3 = tpg.addNewAction(2);
	const TPG::TPGTeam& vertex4 = tpg.addNewTeam();
	const TPG::TPGEdge& edge2 = tpg.addNewEdge(vertex4, vertex3, progPointer);
	const TPG::TPGEdge& edge3 = tpg.addNewEdge(vertex0, vertex3, progPointer);

	Mutator::MutationParameters params;
	params.prog.maxProgramSize = 96;
	params.tpg.pEdgeDeletion = 0.7;
	params.tpg.pEdgeAddition = 0.7;
	params.tpg.pProgramMutation = 0.2;
	params.tpg.pEdgeDestinationChange = 0.1;
	params.tpg.pEdgeDestinationIsAction = 0.5;
	params.prog.pAdd = 0.5;
	params.prog.pDelete = 0.5;
	params.prog.pMutate = 1.0;
	params.prog.pSwap = 1.0;

	// Init its program and fill the archive
	Archive arch;
	TPG::TPGExecutionEngine tee(*e, &arch);
	Mutator::ProgramMutator::initRandomProgram(*progPointer, params);
	tee.executeFromRoot(vertex0);

	// Test the function in normal conditions
	Mutator::RNG::setSeed(0);
	ASSERT_NO_THROW(Mutator::TPGMutator::mutateTPGTeam(tpg, arch, vertex0, { &vertex0, &vertex4 }, { &vertex1, &vertex2, &vertex3 }, { &edge0, &edge1, &edge2, &edge3 }, params)) << "Mutate team should not fail in these conditions.";

	// No other check really needed since individual mutation functions are already covered in other unit tests.
}

TEST_F(MutatorTest, TPGMutatorPopulate) {
	TPG::TPGGraph tpg(*e);

	Mutator::MutationParameters params;

	params.tpg.nbActions = 4;
	params.tpg.maxInitOutgoingEdges = 3;
	params.prog.maxProgramSize = 96;
	params.tpg.nbRoots = 7;
	// Proba as in Kelly's paper
	params.tpg.pEdgeDeletion = 0.7;
	params.tpg.pEdgeAddition = 0.7;
	params.tpg.pProgramMutation = 0.2;
	params.tpg.pEdgeDestinationChange = 0.1;
	params.tpg.pEdgeDestinationIsAction = 0.5;
	params.prog.pAdd = 0.5;
	params.prog.pDelete = 0.5;
	params.prog.pMutate = 1.0;
	params.prog.pSwap = 1.0;
	Archive arch;

	Mutator::TPGMutator::initRandomTPG(tpg, params);
	// fill the archive before populating to test uniqueness of new prog
	TPG::TPGExecutionEngine tee(*e, &arch);
	for (auto rootVertex : tpg.getRootVertices()) {
		tee.executeFromRoot(*rootVertex);
	}

	// Check the correct execution
	ASSERT_NO_THROW(Mutator::TPGMutator::populateTPG(tpg, arch, params)) << "Populating a TPG failed.";
	// Check the number of roots
	ASSERT_EQ(tpg.getRootVertices().size(), params.tpg.nbRoots);

	// Increase coverage with a TPG that has no root team
	TPG::TPGGraph tpg2(*e);
	ASSERT_NO_THROW(Mutator::TPGMutator::populateTPG(tpg2, arch, params)) << "Populating an empty TPG failed.";
}