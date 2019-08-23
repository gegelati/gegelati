#include <gtest/gtest.h>
#include <vector>

#include "instructions/set.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/multByConstParam.h"
#include "dataHandlers/dataHandler.h"
#include "dataHandlers/primitiveTypeArray.h"
#include "program/program.h"
#include "program/line.h"

class ProgramTest : public ::testing::Test {
protected:
	const size_t size1{ 24 };
	const size_t size2{ 32 };
	std::vector<std::reference_wrapper<DataHandlers::DataHandler>> vect;
	Instructions::Set set;
	Environment* e;

	virtual void SetUp() {
		vect.push_back(*(new DataHandlers::PrimitiveTypeArray<double>((unsigned int)size1)));
		vect.push_back(*(new DataHandlers::PrimitiveTypeArray<int>((unsigned int)size2)));

		set.add(*(new Instructions::AddPrimitiveType<float>()));
		set.add(*(new Instructions::MultByConstParam<double, float>()));

		e = new Environment(set, vect, 8);
	}

	virtual void TearDown() {
		delete e;
		delete (&(vect.at(0).get()));
		delete (&(vect.at(1).get()));
		delete (&set.getInstruction(0));
		delete (&set.getInstruction(1));
	}
};

TEST_F(ProgramTest, ProgramConstructor) {
	Program::Program* p;
	ASSERT_NO_THROW({
		p = new Program::Program(*e); }) << "Something went wrong when constructing a Program with a valid Environment.";

	ASSERT_NO_THROW({
		delete p;
		}) << "Something went wrong when destructing a Program with a valid Environment and empty lines.";
}

TEST_F(ProgramTest, ProgramGetEnvironment) {
	Program::Program p(*e);

	ASSERT_EQ(&p.getEnvironment(), e) << "Environment of a Program differs from the one given at construction. (pointer comparison)";
}

TEST_F(ProgramTest, AddEmptyLineAtKnownPosition) {
	Program::Program* p = new Program::Program(*e);
	Program::Line* l;
	ASSERT_NO_THROW(l = &p->addNewLine(0);) << "Inserting a single empty line at position 0 in an empty program should not be an issue at insertion.";

	// Check that line is set to only zero values.
	ASSERT_EQ(l->getDestinationIndex(), 0) << "New line Destination is not set to 0.";
	ASSERT_EQ(l->getInstructionIndex(), 0) << "New line Instruction is not set to 0.";
	for (int i = 0; i < e->getMaxNbParameters(); i++) {
		ASSERT_EQ(l->getParameter(i).i, 0) << "New line parameter is not set to 0.";
	}
	for (int i = 0; i < e->getMaxNbOperands(); i++) {
		ASSERT_EQ(l->getOperand(i).first, 0) << "New line operand source index is not set to 0.";
		ASSERT_EQ(l->getOperand(i).second, 0) << "New line operand location is not set to 0.";
	}

	ASSERT_THROW(p->addNewLine(2), std::out_of_range) << "Insertion of a line beyond the program end should fail.";

	ASSERT_NO_THROW(p->addNewLine(0);) << "Inserting a single empty line at position 0 in a nonempty program should not be an issue at insertion.";
	ASSERT_NO_THROW(p->addNewLine(p->getNbLines());) << "Inserting a single empty line at the end of a nonempty program should not be an issue at insertion.";
	ASSERT_NO_THROW(p->addNewLine(1);) << "Inserting a single empty line in the middle of a nonempty program should not be an issue at insertion.";

	ASSERT_NO_THROW(delete p;) << "Destructing a non empty program should not be an issue.";
}

TEST_F(ProgramTest, AddEmptyLineAndDestruction) {
	Program::Program* p = new Program::Program(*e);
	Program::Line* l;
	ASSERT_NO_THROW(l = &p->addNewLine();) << "Inserting a single empty line in an empty program should not be an issue at insertion.";

	// Check that line is set to only zero values.
	ASSERT_EQ(l->getDestinationIndex(), 0) << "New line Destination is not set to 0.";
	ASSERT_EQ(l->getInstructionIndex(), 0) << "New line Instruction is not set to 0.";
	for (int i = 0; i < e->getMaxNbParameters(); i++) {
		ASSERT_EQ(l->getParameter(i).i, 0) << "New line parameter is not set to 0.";
	}
	for (int i = 0; i < e->getMaxNbOperands(); i++) {
		ASSERT_EQ(l->getOperand(i).first, 0) << "New line operand source index is not set to 0.";
		ASSERT_EQ(l->getOperand(i).second, 0) << "New line operand location is not set to 0.";
	}

	ASSERT_NO_THROW(delete p;) << "Destructing a non empty program should not be an issue.";
}


TEST_F(ProgramTest, getProgramNbLines) {
	Program::Program p(*e);
	ASSERT_EQ(p.getNbLines(), 0) << "Empty program nb lines should be 0.";
	p.addNewLine();
	ASSERT_EQ(p.getNbLines(), 1) << "A single line was just added to the Program.";
}

TEST_F(ProgramTest, GetProgramLine) {
	Program::Program p(*e);
	Program::Line& l1 = p.addNewLine();
	Program::Line& l2 = p.addNewLine();
	Program::Line& l3 = p.addNewLine();

	Program::Line& l4 = p.getLine(0);
	ASSERT_EQ(&l4, &l1) << "Line retrieved is not the right one (based on pointer comparison).";

	const Program::Program& constP = p;
	const Program::Line& l5 = constP.getLine(1);
	ASSERT_EQ(&l5, &l2) << "Line retrieved is not the right one (based on pointer comparison).";

	ASSERT_THROW(p.getLine(3), std::out_of_range) << "Getting line outside of the Program did not fail as expected.";
	ASSERT_THROW(constP.getLine(3), std::out_of_range) << "Getting line outside of the Program did not fail as expected.";
}

TEST_F(ProgramTest, RemoveProgramLine) {
	Program::Program p(*e);
	Program::Line& l1 = p.addNewLine();
	Program::Line& l2 = p.addNewLine();
	Program::Line& l3 = p.addNewLine();

	ASSERT_NO_THROW(p.removeLine(1)) << "Could not remove a line with a valid index.";
	ASSERT_EQ(p.getNbLines(), 2) << "Program length after removal of a line is incorrect.";
	ASSERT_THROW(p.removeLine(2), std::out_of_range) << "Removing a non-existing line should throw an exception.";
}