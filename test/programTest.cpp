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

TEST_F(ProgramTest, LineConstructor) {
	Program::Line* l;

	ASSERT_NO_THROW({
		l = new Program::Line(*e); }) << "Something went wrong when constructing a Line with a valid Environment.";

	ASSERT_NO_THROW({
		delete l;
		}) << "Something went wrong when destructing a Line with a valid Environment.";
}

TEST_F(ProgramTest, LineDestinatioInstructionSetters) {
	Program::Line l(*e);

	ASSERT_TRUE(l.setDestination(UINT64_MAX, false)) << "With checks deactivated, destination should be successfully settable to abberant value.";
	ASSERT_FALSE(l.setDestination(UINT64_MAX)) << "With checks activated, destination should not be successfully settable to abberant value.";
	ASSERT_TRUE(l.setDestination(5)) << "Set destination to valid value failed.";

	ASSERT_TRUE(l.setInstruction(UINT64_MAX, false)) << "With checks deactivated, instruction should be successfully settable to abberant value.";
	ASSERT_FALSE(l.setInstruction(UINT64_MAX)) << "With checks activated, instruction should not be successfully settable to abberant value.";
	ASSERT_TRUE(l.setInstruction(1)) << "Set destination to valid value failed.";
}

TEST_F(ProgramTest, LineDestinationInstructionGetters) {
	Program::Line l(*e);

	l.setDestination(5, false);
	ASSERT_EQ(l.getDestination(), 5) << "Get after set returned the wrong value.";

	l.setInstruction(1, false);
	ASSERT_EQ(l.getInstruction(), 1) << "Get after set returned the wrong value.";
}

TEST_F(ProgramTest, LineParameterAccessors) {
	Program::Line l(*e); // with the given environment, there is a single Parameter per line.
	ASSERT_NO_THROW(l.setParameter(0, 0.2f)) << "Setting value of a correctly indexed parameter failed.";
	ASSERT_THROW(l.setParameter(1, 0.3f), std::range_error) << "Setting value of an incorrectly indexed parameter did not fail.";

	ASSERT_EQ((float)l.getParameter(0), 0.2f) << "Getting a previously set parameter failed.";
	ASSERT_THROW(l.getParameter(1), std::range_error) << "Getting value of an incorrectly indexed parameter did not fail.";
}

TEST_F(ProgramTest, LineOperandAccessors) {
	Program::Line l(*e); // with the given environment, there are two operands per line and 3 data sources.

	// There are only 2 operands
	ASSERT_THROW(l.setOperand(2, 0, 0), std::range_error) << "Setting value of an incorrectly indexed operand did not fail.";
	// There are only 3 data Sources
	ASSERT_FALSE(l.setOperand(0, 3, 0)) << "Setting value of a correctly indexed operand with invalid dataSource index did not fail as expected.";
	// There are only 8 registers
	ASSERT_FALSE(l.setOperand(0, 0, 9)) << "Setting value of a correctly indexed operand with valid dataSource (registers) but invalid location did not fail as expected.";
	// There are only 24 places in dataSource 1
	ASSERT_FALSE(l.setOperand(1, 1, 24)) << "Setting value of a correctly indexed operand with valid dataSource (not registers) but invalid location did not fail as expected.";
	// Deactivate checks
	ASSERT_TRUE(l.setOperand(1, 1, 24, false)) << "Setting value of a correctly indexed operand with valid dataSource (not registers) but invalid location failed, although checks were deactivated.";

	// Valid Set
	ASSERT_TRUE(l.setOperand(1, 1, 12)) << "Setting value of a correctly indexed operand (with valid dataSource index and location) failed.";

	ASSERT_EQ(l.getOperand(1).first, 1) << "Get after valid set of operand dataSource index failed.";
	ASSERT_EQ(l.getOperand(1).second, 12) << "Get after valid set of operand location failed.";

	// There are only 2 operands
	ASSERT_THROW(l.getOperand(2), std::range_error) << "Getting value of an incorrectly indexed operand did not fail.";
}

TEST_F(ProgramTest, ProgramConstructor) {
	Program::Program* p;
	ASSERT_NO_THROW({
		p = new Program::Program(*e); }) << "Something went wrong when constructing a Program with a valid Environment.";

	ASSERT_NO_THROW({
		delete p;
		}) << "Something went wrong when destructing a Program with a valid Environment and empty lines.";

	Environment e2(set, {}, 8); // empty dataHandler should be a problem.
	ASSERT_THROW({
		p = new Program::Program(e2); }, std::domain_error) << "Something went unexpectedly right when constructing a Program with an invalid Environment.";

}

TEST_F(ProgramTest, AddEmptyLineAndDestruction) {
	Program::Program* p = new Program::Program(*e);
	Program::Line* l;
	ASSERT_NO_THROW(l = &p->addNewLine();) << "Inserting a single empty line in an empty program should not be an issue at insertion.";

	// Check that line is set to only zero values.
	ASSERT_EQ(l->getDestination(), 0) << "New line Destination is not set to 0.";
	ASSERT_EQ(l->getInstruction(), 0) << "New line Instruction is not set to 0.";
	for (int i = 0; i < e->getMaxNbParameters(); i++) {
		ASSERT_EQ(l->getParameter(i).i, 0) << "New line parameter is not set to 0.";
	}
	for (int i = 0; i < e->getMaxNbOperands(); i++) {
		ASSERT_EQ(l->getOperand(i).first, 0) << "New line operand source index is not set to 0.";
		ASSERT_EQ(l->getOperand(i).second, 0) << "New line operand location is not set to 0.";
	}

	ASSERT_NO_THROW(delete p;) << "Destructing a non empty program should not be an issue.";
}

TEST_F(ProgramTest, computeLineSize) {
	Program::Program p(*e);
	// Expected answer:
	// n = 8
	// i = 2
	// p = 1
	// nbSrc = 3
	// largestAddressSpace = 32
	// m = 2
	// ceil(log2(n)) + ceil(log2(i)) + m * (ceil(log2(nb_{ src })) + ceil(log2(largestAddressSpace)) + p * sizeof(Param) * 8
	// ceil(log2(8)) + ceil(log2(2)) + 2 * (ceil(log2(3)) + ceil(log2(32)) + 1 * 4 * 8
	//            3  +             1 + 2 * (            2 +             5) +  32 
	ASSERT_EQ(Program::Program::computeLineSize(*e), 50) << "Program Line size is incorrect. Expected value is 50 for (n=8,i=2,p=1,nbSrc=3,largAddrSpace=32,m=2). ";
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
	
	ASSERT_EQ(&p.getLine(1), &l2) << "Line retrieved is not the right one (based on pointer comparison).";

	ASSERT_THROW(p.getLine(3), std::range_error) << "Getting line outside of the Program did not fail as expected.";
}