#include <gtest/gtest.h>

#include "data/supportedTypes.h"
#include "data/dataHandler.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/multByConstParam.h"
#include "instructions/lambdaInstruction.h"
#include "instructions/set.h"

TEST(InstructionsTest, ConstructorDestructorCall) {
	ASSERT_NO_THROW({
		Instructions::Instruction * i = new Instructions::AddPrimitiveType<double>();
	delete i;
		}) << "Call to constructor for AddPrimitiveType<double> failed.";



	ASSERT_NO_THROW({
	Instructions::Instruction * i = new Instructions::AddPrimitiveType<int>();
	delete i;
		}) << "Call to constructor for AddPrimitiveType<int> failed.";
}

TEST(InstructionsTest, OperandListAndNbParam) {
	Instructions::Instruction* i = new Instructions::AddPrimitiveType<double>();
	ASSERT_EQ(i->getNbOperands(), 2) << "Number of operands of Instructions::AddPrimitiveType<double> is different from 2";
	auto operands = i->getOperandTypes();
	ASSERT_EQ(operands.size(), 2) << "Operand list of AddPrimitiveType<double> is different from 2";
	ASSERT_STREQ(operands.at(0).get().name(), typeid(PrimitiveType<double>).name()) << "First operand of AddPrimitiveType<double> is not\"" << typeid(PrimitiveType<double>).name() << "\".";
	ASSERT_STREQ(operands.at(1).get().name(), typeid(PrimitiveType<double>).name()) << "Second operand of AddPrimitiveType<double> is not\"" << typeid(PrimitiveType<double>).name() << "\".";
	ASSERT_EQ(i->getNbParameters(), 0) << "Number of parameters of AddPrimitiveType<double> should be 0.";
	delete i;
}

TEST(InstructionsTest, CheckArgumentTypes) {
	Instructions::Instruction* i = new Instructions::AddPrimitiveType<double>();
	PrimitiveType<double> a{ 2.5 };
	PrimitiveType<double> b = 5.6;
	PrimitiveType<double> c = 3.7;
	PrimitiveType<int> d = 5;

	std::vector<std::shared_ptr<const SupportedType>> vect;
	vect.emplace_back(&a, Data::DataHandler::emptyDestructor());
	vect.emplace_back(&b, Data::DataHandler::emptyDestructor());
	ASSERT_TRUE(i->checkOperandTypes(vect)) << "Operands of valid types wrongfully classified as invalid.";
	vect.emplace_back(&c, Data::DataHandler::emptyDestructor());
	ASSERT_FALSE(i->checkOperandTypes(vect)) << "Operands list of too long size wrongfully classified as valid.";
	vect.pop_back();
	vect.pop_back();
	vect.emplace_back(&d, Data::DataHandler::emptyDestructor());
	ASSERT_FALSE(i->checkOperandTypes(vect)) << "Operands of invalid types wrongfully classified as valid";
	delete i;
}

TEST(InstructionsTest, CheckParameters) {
	Instructions::Instruction* i = new Instructions::AddPrimitiveType<int>();
	std::vector<std::reference_wrapper<const Parameter>> v;
	Parameter a = (int16_t)2;
	Parameter b = 3.2f;
	Parameter c = -2.0f; // To cover init from negative float in Parameter.
	v.push_back(a);
	v.push_back(b);
	v.push_back(c);
	ASSERT_FALSE(i->checkParameters(v)) << "Parameter list of wrong size not detected as such.";
	delete i;

	i = new Instructions::MultByConstParam<double, int16_t>();
	v.pop_back();
	v.pop_back();
	ASSERT_TRUE(i->checkParameters(v)) << "Parameter list of right size not detected as such.";
	delete i;
}

TEST(InstructionsTest, Execute) {
	Instructions::Instruction* i = new Instructions::AddPrimitiveType<double>();
	PrimitiveType<double> a{ 2.6 };
	PrimitiveType<double> b = 5.5;
	PrimitiveType<int> c = 3;

	std::vector<std::shared_ptr<const SupportedType>> vect;
	vect.emplace_back(&a, Data::DataHandler::emptyDestructor());
	vect.emplace_back(&b, Data::DataHandler::emptyDestructor());
	ASSERT_EQ(i->execute({}, vect), 8.1) << "Execute method of AddPrimitiveType<double> returns an incorrect value with valid operands.";

	vect.pop_back();
	vect.emplace_back(&c, Data::DataHandler::emptyDestructor());
	ASSERT_EQ(i->execute({}, vect), 0.0) << "Execute method of AddPrimitiveType<double> returns an incorrect value with invalid operands.";

	delete i;
	i = new Instructions::MultByConstParam<double, int16_t>();
	vect.pop_back();
	Parameter p = (int16_t)2;
	ASSERT_EQ(i->execute({ p }, vect), 5.2) << "Execute method of MultByConstParam<double,int> returns an incorrect value with valid operands.";
	ASSERT_EQ(i->execute({ }, vect), 0.0) << "Execute method of MultByConstParam<double,int> returns an incorrect value with invalid params.";
	delete i;
}

TEST(InstructionsTest, LambdaInstruction) {
	PrimitiveType<double> a{ 2.6 };
	PrimitiveType<double> b = 5.5;
	PrimitiveType<int> c = 3;

	std::vector<std::shared_ptr<const SupportedType>> vect;
	vect.emplace_back(&a, Data::DataHandler::emptyDestructor());
	vect.emplace_back(&b, Data::DataHandler::emptyDestructor());

	auto minus = [](double a, double b) {return a - b; };

	Instructions::LambdaInstruction<double>* instruction;
	ASSERT_NO_THROW(instruction = new Instructions::LambdaInstruction<double>(minus)) << "Constructing a new lambdaInstruction failed.";

	ASSERT_EQ(instruction->execute({}, vect), -2.9) << "Result returned by the instruction is not as expected.";

	// Execute with wrong types of operands.
	vect.pop_back();
	vect.emplace_back(&c, Data::DataHandler::emptyDestructor());
	ASSERT_EQ(instruction->execute({}, vect), 0.0) << "Instructions executed with wrong types of operands should return 0.0";

	ASSERT_NO_THROW(delete instruction) << "Destruction of the LambdaInstruction failed.";
}

TEST(InstructionsTest, SetAdd) {
	Instructions::Set s;

	Instructions::MultByConstParam<int, float> iMult;
	Instructions::MultByConstParam<int, float> iMult2;
	Instructions::MultByConstParam<int, int16_t> iMult3;

	ASSERT_TRUE(s.add(iMult)) << "Add of instruction to empty Instructions::Set failed.";
	// Adding equivalent instructions is no longer forbidden.
	ASSERT_TRUE(s.add(iMult2)) << "Add of instruction already present in an Instructions::Set should not fail.";
	ASSERT_TRUE(s.add(iMult3)) << "Add of instruction to non empty Instructions::Set failed. (with a template instruction with different template param than an already present one";
}

TEST(InstructionsTest, SetGetNbInstruction) {
	Instructions::Set s;

	ASSERT_EQ(s.getNbInstructions(), 0) << "Incorrect number of instructions in an empty Set.";

	Instructions::MultByConstParam<int, float> iMult;
	Instructions::MultByConstParam<int, int16_t> iMult2;
	s.add(iMult);
	s.add(iMult2);
	ASSERT_EQ(s.getNbInstructions(), 2) << "Incorrect number of instructions in a non-empty Set.";
}

TEST(InstructionsTest, SetGetInstruction) {
	Instructions::Set s;

	Instructions::AddPrimitiveType<float> iAdd;
	Instructions::MultByConstParam<double, float> iMult;
	s.add(iAdd);
	s.add(iMult);

	const Instructions::Instruction* res;
	ASSERT_NO_THROW(res = &s.getInstruction(1)) << "Exception was thrown unexpectedly when calling Set::getInstruction with a valid index.";

	// Compare that the returned reference points to the right object.
	ASSERT_EQ(res, &iMult) << "Incorrect Instruction was returned by valid Set::getInstruction.";

	// Check that exception is thrown when an invalid index is given.
	ASSERT_THROW(res = &s.getInstruction(2), std::out_of_range) << "Exception was not thrown when calling Set::getInstruction with an invalid index.";
}

TEST(InstructionsTest, SetGetNbMaxOperands) {
	Instructions::Set s;

	ASSERT_EQ(s.getMaxNbOperands(), 0) << "Max number of operands returned by the empty Instructions::Set is incorrect.";

	Instructions::AddPrimitiveType<float> iAdd; // Two operands
	Instructions::MultByConstParam<double, float> iMult; // One operand
	s.add(iAdd);
	s.add(iMult);

	ASSERT_EQ(s.getMaxNbOperands(), 2) << "Max number of operands returned by the Instructions::Set is incorrect.";
}

TEST(InstructionsTest, SetGetNbMaxParameters) {
	Instructions::Set s;

	ASSERT_EQ(s.getMaxNbParameters(), 0) << "Max number of parameters returned by the empty Instructions::Set is incorrect.";

	Instructions::AddPrimitiveType<float> iAdd; // Two operands
	Instructions::MultByConstParam<double, float> iMult; // One operand
	s.add(iAdd);
	s.add(iMult);

	ASSERT_EQ(s.getMaxNbParameters(), 1) << "Max number of parameters returned by the Instructions::Set is incorrect.";
}