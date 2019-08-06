#include <gtest/gtest.h>
#include "instructions/addPrimitiveType.h"
#include "instructions/multByConstParam.h"
#include "instructions/set.h"

TEST(Instructions, ConstructorDestructorCall) {
	ASSERT_NO_THROW({
		Instructions::Instruction* i = new Instructions::AddPrimitiveType<double>();
	delete i;
		}) << "Call to constructor for AddPrimitiveType<double> failed.";



	ASSERT_NO_THROW({
	Instructions::Instruction* i = new Instructions::AddPrimitiveType<int>();
	delete i;
		}) << "Call to constructor for AddPrimitiveType<int> failed.";


}

TEST(Instructions, OperandListAndNbParam) {
	Instructions::Instruction* i = new Instructions::AddPrimitiveType<double>();
	ASSERT_EQ(i->getNbOperands(), 2) << "Number of operands of Instructions::AddPrimitiveType<double> is different from 2";
	auto operands = i->getOperandTypes();
	ASSERT_EQ(operands.size(), 2) << "Operand list of AddPrimitiveType<double> is different from 2";
	ASSERT_STREQ(operands.at(0).get().name(), typeid(PrimitiveType<double>).name()) << "First operand of AddPrimitiveType<double> is not\"" << typeid(PrimitiveType<double>).name() << "\".";
	ASSERT_STREQ(operands.at(1).get().name(), typeid(PrimitiveType<double>).name()) << "Second operand of AddPrimitiveType<double> is not\"" << typeid(PrimitiveType<double>).name() << "\".";
	ASSERT_EQ(i->getNbParameters(), 0) << "Number of parameters of AddPrimitiveType<double> should be 0.";
	delete i;
}

TEST(Instructions, CheckArgumentTypes) {
	Instructions::Instruction* i = new Instructions::AddPrimitiveType<double>();
	PrimitiveType<double> a{ 2.5 };
	PrimitiveType<double> b = 5.6;
	PrimitiveType<double> c = 3.7;
	PrimitiveType<int> d = 5;

	std::vector<std::reference_wrapper<SupportedType>> vect;
	vect.push_back(a);
	vect.push_back(b);
	ASSERT_TRUE(i->checkOperandTypes(vect)) << "Operands of valid types wrongfully classified as invalid.";
	vect.push_back(c);
	ASSERT_FALSE(i->checkOperandTypes(vect)) << "Operands list of too long size wrongfully classified as valid.";
	vect.pop_back();
	vect.pop_back();
	vect.push_back(d);
	ASSERT_FALSE(i->checkOperandTypes(vect)) << "Operands of invalid types wrongfully classified as valid";
	delete i;
}

TEST(Instructions, CheckParameters) {
	Instructions::Instruction* i = new Instructions::AddPrimitiveType<int>();
	std::vector<std::reference_wrapper<Parameter>> v;
	Parameter a = 2;
	Parameter b = 3.2f;
	v.push_back(a);
	v.push_back(b);
	ASSERT_FALSE(i->checkParameters(v)) << "Parameter list of wrong size not detected as such.";
	delete i;

	i = new Instructions::MultByConstParam<double, int>();
	v.pop_back();
	ASSERT_TRUE(i->checkParameters(v)) << "Parameter list of right size not detected as such.";
	delete i;
}

TEST(Instructions, Execute) {
	Instructions::Instruction* i = new Instructions::AddPrimitiveType<double>();
	PrimitiveType<double> a{ 2.6 };
	PrimitiveType<double> b = 5.5;
	PrimitiveType<int> c = 3;

	std::vector<std::reference_wrapper<SupportedType>> vect;
	vect.push_back(a);
	vect.push_back(b);
	ASSERT_EQ(i->execute({}, vect), 8.1) << "Execute method of AddPrimitiveType<double> returns an incorrect value with valid operands.";

	vect.pop_back();
	vect.push_back(c);
	ASSERT_EQ(i->execute({}, vect), 0.0) << "Execute method of AddPrimitiveType<double> returns an incorrect value with invalid operands.";

	delete i;
	i = new Instructions::MultByConstParam<double, int>();
	vect.pop_back();
	Parameter p = 2;
	ASSERT_EQ(i->execute({ p }, vect), 5.2) << "Execute method of MultByConstParam<double,int> returns an incorrect value with valid operands.";
	ASSERT_EQ(i->execute({ }, vect), 0.0) << "Execute method of MultByConstParam<double,int> returns an incorrect value with invalid params.";
	delete i;
}

TEST(Instructions, SetAdd) {
	Instructions::Set s;

	Instructions::MultByConstParam<int, float> iMult;
	Instructions::MultByConstParam<int, float> iMult2;
	Instructions::MultByConstParam<int, int> iMult3;

	ASSERT_TRUE(s.add(iMult)) << "Add of instruction to empty Instructions::Set failed.";
	ASSERT_FALSE(s.add(iMult2)) << "Add of instruction already present in an Instructions::Set did not fail.";
	ASSERT_TRUE(s.add(iMult3)) << "Add of instruction to non empty Instructions::Set failed. (with a template instruction with different template param than an already present one";
}

TEST(Instructions, SetGetNbInstruction) {
	Instructions::Set s;

	ASSERT_EQ(s.getNbInstructions(), 0) << "Incorrect number of instructions in an empty Set.";

	Instructions::MultByConstParam<int, float> iMult;
	Instructions::MultByConstParam<int, int> iMult2;
	s.add(iMult);
	s.add(iMult2);
	ASSERT_EQ(s.getNbInstructions(), 2) << "Incorrect number of instructions in a non-empty Set.";
}

TEST(Instructions, SetGetInstruction) {
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

TEST(Instructions, SetGetNbMaxOperands) {
	Instructions::Set s;

	ASSERT_EQ(s.getMaxNbOperands(), 0) << "Max number of operands returned by the empty Instructions::Set is incorrect.";

	Instructions::AddPrimitiveType<float> iAdd; // Two operands
	Instructions::MultByConstParam<double, float> iMult; // One operand
	s.add(iAdd);
	s.add(iMult);

	ASSERT_EQ(s.getMaxNbOperands(), 2) << "Max number of operands returned by the Instructions::Set is incorrect.";
}

TEST(Instructions, SetGetNbMaxParameters) {
	Instructions::Set s;

	ASSERT_EQ(s.getMaxNbParameters(), 0) << "Max number of parameters returned by the empty Instructions::Set is incorrect.";

	Instructions::AddPrimitiveType<float> iAdd; // Two operands
	Instructions::MultByConstParam<double, float> iMult; // One operand
	s.add(iAdd);
	s.add(iMult);

	ASSERT_EQ(s.getMaxNbParameters(), 1) << "Max number of parameters returned by the Instructions::Set is incorrect.";
}