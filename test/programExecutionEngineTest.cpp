#include <gtest/gtest.h>
#include <vector>

#include "instructions/set.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/multByConstParam.h"
#include "dataHandlers/dataHandler.h"
#include "dataHandlers/primitiveTypeArray.h"
#include "program/program.h"
#include "program/line.h"
#include "program/programExecutionEngine.h"

class ProgramExecutionEngineTest : public ::testing::Test {
protected:
	const size_t size1{ 24 };
	const size_t size2{ 32 };
	const double value0{ 2.3 };
	std::vector<std::reference_wrapper<DataHandlers::DataHandler>> vect;
	Instructions::Set set;
	Environment* e;
	Program::Program* p;

	virtual void SetUp() {
		vect.push_back(*(new DataHandlers::PrimitiveTypeArray<int>((unsigned int)size1)));
		vect.push_back(*(new DataHandlers::PrimitiveTypeArray<double>((unsigned int)size2)));

		((DataHandlers::PrimitiveTypeArray<double>&)vect.at(1).get()).setDataAt(typeid(PrimitiveType<double>), 25, value0);

		set.add(*(new Instructions::AddPrimitiveType<double>()));
		set.add(*(new Instructions::MultByConstParam<double, float>()));

		e = new Environment(set, vect, 8);
		p = new Program::Program(*e);

		Program::Line& l0 = p->addNewLine();
		l0.setInstruction(0); // Instruction is addPrimitiveType<double>.
		l0.setOperand(0, 0, 5); // 1st operand: 6th register.
		l0.setOperand(1, 2, 25); // 2nd operand: 26th double in the PrimitiveTypeArray of double.
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

TEST_F(ProgramExecutionEngineTest, ConstructorDestructor) {
	Program::ProgramExecutionEngine* progExecEng;
	ASSERT_NO_THROW(progExecEng = new Program::ProgramExecutionEngine(*p)) << "Construction failed.";

	ASSERT_NO_THROW(delete progExecEng) << "Destruction failed.";
}

TEST_F(ProgramExecutionEngineTest, getCurrentLine) {
	Program::ProgramExecutionEngine progExecEng(*p);

	// Valid since the program has more than 0 line and program counter is initialized to 0.
	ASSERT_EQ(&progExecEng.getCurrentLine(), &p->getLine(0)) << "First line of the Program not accessible from the ProgramExecutionEngine.";
}

TEST_F(ProgramExecutionEngineTest, getCurrentInstruction) {
	Program::ProgramExecutionEngine progExecEng(*p);

	// Valid since the program has more than 0 line and program counter is initialized to 0.
	const Instructions::Instruction& instr = progExecEng.getCurrentInstruction();
	// First Instruction of the set (from Fixture) is Instructions::AddPrimitiveType<double>.
	// Since the Line was initialized to 0, its instruction index is 0.
	ASSERT_EQ(typeid(instr), typeid(Instructions::AddPrimitiveType<double>)) << "The type of the instruction does not correspond to the Set of the Environment.";
}

TEST_F(ProgramExecutionEngineTest, fetchOperands) {
	Program::ProgramExecutionEngine progExecEng(*p);
	std::vector<std::reference_wrapper<const SupportedType>> operands;
	// From Fixture:
	// Program line 0
	// Instruction is AddPrimitiveType<double>.
	// Operands are: 6th (index = 5) register and 26th (index = 25) element of an double array.
	ASSERT_NO_THROW(progExecEng.fetchCurrentOperands(operands)) << "Fetching the operands of a valid Program from fixtures failed.";
	// Check number of operands
	ASSERT_EQ(operands.size(), 2) << "Insufficient number of operands were fetched by previous call.";
	// Check operand value. Register is 0.0, array element is value0: 2.3
	ASSERT_EQ((double)((const PrimitiveType<double>&)operands.at(0).get()), 0.0) << "Value of fetched operand from register is incorrect.";
	ASSERT_EQ((double)((const PrimitiveType<double>&)operands.at(1).get()), value0) << "Value of fetched operand from array is incorrect compared to Test fixture.";
}