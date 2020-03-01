#include <gtest/gtest.h>
#include <vector>

#include "instructions/set.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/multByConstParam.h"
#include "data/untypedSharedPtr.h"
#include "data/dataHandler.h"
#include "data/primitiveType.h"
#include "data/primitiveTypeArray.h"
#include "program/program.h"
#include "program/line.h"
#include "program/programExecutionEngine.h"

class ProgramExecutionEngineTest : public ::testing::Test {
protected:
	const size_t size1{ 24 };
	const size_t size2{ 32 };
	const double value0{ 2.3 };
	const float value1{ 0.2f };
	std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
	Instructions::Set set;
	Environment* e;
	Program::Program* p;

	virtual void SetUp() {
		vect.push_back(*(new Data::PrimitiveTypeArray<int>((unsigned int)size1)));
		vect.push_back(*(new Data::PrimitiveTypeArray<double>((unsigned int)size2)));

		((Data::PrimitiveTypeArray<double>&)vect.at(1).get()).setDataAt(typeid(Data::PrimitiveType<double>), 25, value0);

		set.add(*(new Instructions::AddPrimitiveType<double>()));
		set.add(*(new Instructions::MultByConstParam<double, float>()));

		e = new Environment(set, vect, 8);
		p = new Program::Program(*e);

		Program::Line& l0 = p->addNewLine();
		l0.setInstructionIndex(0); // Instruction is addPrimitiveType<double>.
		l0.setOperand(0, 0, 5); // 1st operand: 6th register.
		l0.setOperand(1, 2, 25); // 2nd operand: 26th double in the PrimitiveTypeArray of double.
		l0.setDestinationIndex(1); // Destination is resgister at index 1

		// Intron line
		Program::Line& l1 = p->addNewLine();
		l1.setInstructionIndex(1); // Instruction is MultByConstParam<double, float>.
		l1.setOperand(0, 0, 3); // 1st operand: 3rd register.
		l1.setParameter(0, (float)value0); // Parameter is set to value1 (=2.3f)
		l1.setDestinationIndex(0); // Destination is register at index 0

		Program::Line& l2 = p->addNewLine();
		l2.setInstructionIndex(1); // Instruction is MultByConstParam<double, float>.
		l2.setOperand(0, 0, 1); // 1st operand: 1th register.
		l2.setParameter(0, value1); // Parameter is set to value1 (=0.2f)
		l2.setDestinationIndex(0); // Destination is register at index 0

		// Mark intron lines
		ASSERT_EQ(p->identifyIntrons(), 1);
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

	std::vector<std::reference_wrapper<Data::DataHandler>> vect2;
	vect2.push_back(*vect.at(0).get().clone());
	ASSERT_THROW(progExecEng = new Program::ProgramExecutionEngine(*p, vect2), std::runtime_error) << "Construction should faile with data sources differing in number from those of the Environment.";
	vect2.push_back(*vect.at(1).get().clone());

	ASSERT_NO_THROW(progExecEng = new Program::ProgramExecutionEngine(*p)) << "Construction failed with a perfect copy of the environment data source.";
	ASSERT_NO_THROW(delete progExecEng) << "Destruction failed.";

	// Push a new dataHandler instead.
	// Because its id is different, it will not be accepted by the PEE.
	delete (&(vect2.at(1).get()));
	vect2.pop_back();
	vect2.push_back(*(new Data::PrimitiveTypeArray<double>((unsigned int)size2)));
	ASSERT_THROW(progExecEng = new Program::ProgramExecutionEngine(*p, vect2), std::runtime_error) << "Construction should fail with data sources differing in id from those of the Environment.";

	delete (&(vect2.at(0).get()));
	delete (&(vect2.at(1).get()));
}

TEST_F(ProgramExecutionEngineTest, next) {
	Program::ProgramExecutionEngine progExecEng(*p);

	ASSERT_TRUE(progExecEng.next()) << "Program has two line so going to the next line after initialization should succeed.";
	ASSERT_FALSE(progExecEng.next()) << "Program has two line so going to the next line twice after initialization should not succeed.";
}

TEST_F(ProgramExecutionEngineTest, getCurrentLine) {
	Program::ProgramExecutionEngine progExecEng(*p);

	// Valid since the program has more than 0 line and program counter is initialized to 0.
	ASSERT_EQ(&progExecEng.getCurrentLine(), &p->getLine(0)) << "First line of the Program not accessible from the ProgramExecutionEngine.";
	progExecEng.next(); // Skips the intron automatically
	ASSERT_EQ(&progExecEng.getCurrentLine(), &p->getLine(2)) << "Second line of the Program not accessible from the ProgramExecutionEngine.";
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
	std::vector<Data::UntypedSharedPtr> operands;
	// From Fixture:
	// Program line 0
	// Instruction is AddPrimitiveType<double>.
	// Operands are: 6th (index = 5) register and 26th (index = 25) element of an double array.
	ASSERT_NO_THROW(progExecEng.fetchCurrentOperands(operands)) << "Fetching the operands of a valid Program from fixtures failed.";
	// Check number of operands
	ASSERT_EQ(operands.size(), 2) << "Incorrect number of operands were fetched by previous call.";
	// Check operand value. Register is 0.0, array element is value0: 2.3
	ASSERT_EQ((double)*((operands.at(0)).getSharedPointer<const Data::PrimitiveType<double>>()), 0.0) << "Value of fetched operand from register is incorrect.";
	ASSERT_EQ((double)*((operands.at(1)).getSharedPointer<const Data::PrimitiveType<double>>()), value0) << "Value of fetched operand from array is incorrect compared to Test fixture.";
}

TEST_F(ProgramExecutionEngineTest, fetchParameters) {
	Program::ProgramExecutionEngine progExecEng(*p);
	std::vector<std::reference_wrapper<const Parameter>> parameters;

	// First line of fixture has no parameters. Just check that nothing is thrown.
	ASSERT_NO_THROW(progExecEng.fetchCurrentParameters(parameters)) << "Fetching the parameters of a valid Program from fixtures failed.";
	ASSERT_EQ(parameters.size(), 0) << "Since first line of the Program refers to an instruction using no Parameter, the vector should remain empty.";
	progExecEng.next();

	ASSERT_NO_THROW(progExecEng.fetchCurrentParameters(parameters)) << "Fetching the parameters of a valid Program from fixtures failed.";
	// Check number of parameters
	ASSERT_EQ(parameters.size(), 1) << "Incorrect number of operands were fetched by previous call.";
	// Check parameter value (set in fixture). value1: 0.2f (+/- the parameter floating precision)
	ASSERT_NEAR((float)parameters.at(0).get(), value1, PARAM_FLOAT_PRECISION) << "Value of fetched parameter is incorrect.";
}

TEST_F(ProgramExecutionEngineTest, executeCurrentLine) {
	Program::ProgramExecutionEngine progExecEng(*p);

	ASSERT_NO_THROW(progExecEng.executeCurrentLine()) << "Execution of the first line of the program from Fixture should not fail.";
	progExecEng.next(); // Skips the intron automatically
	ASSERT_NO_THROW(progExecEng.executeCurrentLine()) << "Execution of the second line of the program from Fixture should not fail.";
	progExecEng.next();
	ASSERT_THROW(progExecEng.executeCurrentLine(), std::out_of_range) << "Execution of a non-existing line of the program should fail.";

}

TEST_F(ProgramExecutionEngineTest, setProgram) {
	Program::ProgramExecutionEngine progExecEng(*p);

	// Create a new program
	Program::Program p2(*e);

	ASSERT_NO_THROW(progExecEng.setProgram(p2)) << "Setting a new Program with a valid Environment for a ProgramExecutionEngine failed.";

	// Create a new incompatible program
	std::vector<std::reference_wrapper<const Data::DataHandler>> otherVect;
	otherVect.push_back(*(new Data::PrimitiveTypeArray<int>((unsigned int)size2)));
	Environment otherE(set, otherVect, 2);
	Program::Program p3(otherE);

	ASSERT_THROW(progExecEng.setProgram(p3), std::runtime_error) << "Setting a Program with an incompatible Environment should not be possible.";

	// Clean up
	delete& otherVect.at(0).get();
}

TEST_F(ProgramExecutionEngineTest, setDataSources) {
	Program::ProgramExecutionEngine progExecEng(*p);

	// Create a new compatible set of dataSources
	std::vector<std::reference_wrapper<const Data::DataHandler>> otherVect;
	otherVect.push_back(*vect.at(0).get().clone());
	otherVect.push_back(*vect.at(1).get().clone());

	ASSERT_NO_THROW(progExecEng.setDataSources(otherVect)) << "Setting a new valid set of Data Sources failed.";

	// Clean up
	delete& otherVect.at(0).get();
	delete& otherVect.at(1).get();
	otherVect.pop_back();
	otherVect.pop_back();

	// Create a new incompatible set of dataSources
	// although it has the same type and size of data, id of the
	// data handlers are different, which currently breaks the comparison.
	otherVect.push_back(*(new Data::PrimitiveTypeArray<int>((unsigned int)size1)));
	otherVect.push_back(*(new Data::PrimitiveTypeArray<double>((unsigned int)size2)));

	ASSERT_THROW(progExecEng.setDataSources(otherVect), std::runtime_error) << "Setting a new invalid set of Data Sources should fail.";

	// Clean up
	delete& otherVect.at(0).get();
	delete& otherVect.at(1).get();
}

TEST_F(ProgramExecutionEngineTest, execute) {
	Program::ProgramExecutionEngine progExecEng(*p);
	double result;

	ASSERT_NO_THROW(result = progExecEng.executeProgram()) << "Program from fixture failed to execute. (Indivitual execution of its line in executeCurrentLine test).";
	ASSERT_EQ(result, (value0 + 0) * (Parameter(value1)).operator float()) << "Result of the program from Fixture is not as expected.";

	// Introduce a new line in the program to test the throw
	Program::Line& l2 = p->addNewLine();
	// Instruction 2 does not exist. Must deactivate checks to write this instruction
	l2.setInstructionIndex(2, false);
	ASSERT_THROW(progExecEng.executeProgram(), std::out_of_range) << "Program line using a incorrect Instruction index should throw an exception.";

	// Now ignoring the exceptions
	ASSERT_NO_THROW(result = progExecEng.executeProgram(true)) << "Program line using a incorrect Instruction index should not interrupt the Execution when ignored.";
	ASSERT_EQ(result, (value0 + 0) * Parameter(value1).operator float()) << "Result of the program from Fixture, with an additional ignored line, is not as expected.";
}