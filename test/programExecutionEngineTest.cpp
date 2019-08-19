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
	std::vector<std::reference_wrapper<DataHandlers::DataHandler>> vect;
	Instructions::Set set;
	Environment* e;
	Program::Program* p;

	virtual void SetUp() {
		vect.push_back(*(new DataHandlers::PrimitiveTypeArray<double>((unsigned int)size1)));
		vect.push_back(*(new DataHandlers::PrimitiveTypeArray<int>((unsigned int)size2)));

		set.add(*(new Instructions::AddPrimitiveType<float>()));
		set.add(*(new Instructions::MultByConstParam<double, float>()));

		e = new Environment(set, vect, 8);
		p = new Program::Program(*e);

		p->addNewLine();
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
	// First Instruction of the set (from Fixture) is Instructions::AddPrimitiveType<float>.
	// Since the Line was initialized to 0, its instruction index is 0.
	ASSERT_EQ(typeid(instr), typeid(Instructions::AddPrimitiveType<float>)) << "The type of the instruction does not correspond to the Set of the Environment.";
}