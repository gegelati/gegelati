#include <gtest/gtest.h>
#include <vector>

#include "instructions/set.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/multByConstParam.h"
#include "dataHandlers/dataHandler.h"
#include "dataHandlers/primitiveTypeArray.h"
#include "program/program.h"
#include "program/line.h"

class LineTest : public ::testing::Test {
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

TEST_F(LineTest, LineConstructor) {
	Program::Line* l;

	ASSERT_NO_THROW({
		l = new Program::Line(*e); }) << "Something went wrong when constructing a Line with a valid Environment.";

	ASSERT_NO_THROW({
		delete l;
		}) << "Something went wrong when destructing a Line with a valid Environment.";
}

TEST_F(LineTest, LineGetEnvironment) {
	Program::Line l(*e);

	ASSERT_EQ(&l.getEnvironment(), e) << "Environment of the Line does not corresponds to the one given at creation (pointer comparison)";
}

TEST_F(LineTest, LineDestinatioInstructionSetters) {
	Program::Line l(*e);

	ASSERT_TRUE(l.setDestinationIndex(UINT64_MAX, false)) << "With checks deactivated, destinationIndex should be successfully settable to abberant value.";
	ASSERT_FALSE(l.setDestinationIndex(UINT64_MAX)) << "With checks activated, destinationIndex should not be successfully settable to abberant value.";
	ASSERT_TRUE(l.setDestinationIndex(5)) << "Set destinationIndex to valid value failed.";

	ASSERT_TRUE(l.setInstructionIndex(UINT64_MAX, false)) << "With checks deactivated, instructionIndex should be successfully settable to abberant value.";
	ASSERT_FALSE(l.setInstructionIndex(UINT64_MAX)) << "With checks activated, instructionIndex should not be successfully settable to abberant value.";
	ASSERT_TRUE(l.setInstructionIndex(1)) << "Set destinationIndex to valid value failed.";
}

TEST_F(LineTest, LineDestinationInstructionGetters) {
	Program::Line l(*e);

	l.setDestinationIndex(5, false);
	ASSERT_EQ(l.getDestinationIndex(), 5) << "Get after set returned the wrong value.";

	l.setInstructionIndex(1, false);
	ASSERT_EQ(l.getInstructionIndex(), 1) << "Get after set returned the wrong value.";
}

TEST_F(LineTest, LineParameterAccessors) {
	Program::Line l(*e); // with the given environment, there is a single Parameter per line.
	ASSERT_NO_THROW(l.setParameter(0, 0.2f)) << "Setting value of a correctly indexed parameter failed.";
	ASSERT_THROW(l.setParameter(1, 0.3f), std::range_error) << "Setting value of an incorrectly indexed parameter did not fail.";

	ASSERT_EQ((const float&)l.getParameter(0), 0.2f) << "Getting a previously set parameter failed.";
	ASSERT_THROW(l.getParameter(1), std::range_error) << "Getting value of an incorrectly indexed parameter did not fail.";
}

TEST_F(LineTest, LineOperandAccessors) {
	Program::Line l(*e); // with the given environment, there are two operands per line and 3 data sources.

	// There are only 2 operands
	ASSERT_THROW(l.setOperand(2, 0, 0), std::range_error) << "Setting value of an incorrectly indexed operand did not fail.";
	// There are only 3 data Sources
	ASSERT_FALSE(l.setOperand(0, 3, 0)) << "Setting value of a correctly indexed operand with invalid dataSource index did not fail as expected.";
	// There are only 8 registers but scaling of operand location will take care of this.
	ASSERT_TRUE(l.setOperand(0, 0, 9)) << "Setting value of a correctly indexed operand with valid dataSource (registers) and a location larger than the corresponding largestAddressSpace for this operand pair should succeed.";
	// There are only 24 places in dataSource 1 but scaling of operand location will take care of this.
	ASSERT_TRUE(l.setOperand(1, 1, 24)) << "Setting value of a correctly indexed operand with valid dataSource (not registers) and a location larger than the corresponding largestAddressSpace for this dataHandler (but lower than the environment largestAddressSpace) should succeed.";
	// There are only 32 places in the largest dataSource of the envirnoment.
	ASSERT_FALSE(l.setOperand(0, 0, 32)) << "Setting value of a correctly indexed operand with valid dataSource (not registers) and a location larger than the largestAddressSpace of the environment should fail.";
	// Deactivate checks
	ASSERT_TRUE(l.setOperand(0, 3, 0, false)) << "Setting value of a correctly indexed operand with invalid dataSource index failed when check were deactivated.";
	ASSERT_TRUE(l.setOperand(0, 0, 32, false)) << "Setting value of a correctly indexed operand with valid dataSource (not registers) and a location larger than the largestAddressSpace of the environment should succeed without checks.";

	// Valid Set
	ASSERT_TRUE(l.setOperand(1, 1, 12)) << "Setting value of a correctly indexed operand (with valid dataSource index and location) failed.";

	ASSERT_EQ(l.getOperand(1).first, 1) << "Get after valid set of operand dataSource index failed.";
	ASSERT_EQ(l.getOperand(1).second, 12) << "Get after valid set of operand location failed.";

	// There are only 2 operands
	ASSERT_THROW(l.getOperand(2), std::range_error) << "Getting value of an incorrectly indexed operand did not fail.";
}