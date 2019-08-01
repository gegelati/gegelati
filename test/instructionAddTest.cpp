#include <gtest/gtest.h>
#include "instructionAdd.h"
#include "instructionMultByConstParam.h"

TEST(InstructionAdd, ConstructorDestructorCall) {
	Instruction* i = new InstructionAdd<double>();
	ASSERT_NE(i, nullptr) << "Call to constructor for InstructionAdd<double> failed.";
	delete i;
	i = new InstructionAdd<int>();
	ASSERT_NE(i, nullptr) << "Call to constructor for InstructionAdd<int> failed.";
	delete i;
}

TEST(InstructionAdd, OperandListAndNbParam) {
	Instruction* i = new InstructionAdd<double>();
	auto operands = i->getOperandTypes();
	ASSERT_EQ(operands.size(), 2) << "Operand list of InstructionAdd<double> is different from 2";
	ASSERT_STREQ(operands.at(0).get().name(), typeid(PrimitiveType<double>).name()) << "First operand of InstructionAdd<double> is not\"" << typeid(PrimitiveType<double>).name() << "\".";
	ASSERT_STREQ(operands.at(1).get().name(), typeid(PrimitiveType<double>).name()) << "Second operand of InstructionAdd<double> is not\"" << typeid(PrimitiveType<double>).name() << "\".";
	ASSERT_EQ(i->getNbParameters(), 0) << "Number of parameters of InstructionAdd<double> should be 0.";
	delete i;
}

TEST(InstructionAdd, CheckArgumentTypes) {
	Instruction* i = new InstructionAdd<double>();
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

TEST(InstructionAdd, CheckParameters) {
	Instruction* i = new InstructionAdd<int>();
	std::vector<std::reference_wrapper<Parameter>> v;
	Parameter a = 2;
	Parameter b = 3.2f;
	v.push_back(a);
	v.push_back(b);
	ASSERT_FALSE(i->checkParameters(v)) << "Parameter list of wrong size not detected as such.";
	delete i;

	i = new InstructionMultByConstParam<double, int>();
	v.pop_back();
	ASSERT_TRUE(i->checkParameters(v))  << "Parameter list of right size not detected as such.";
	delete i;
}

TEST(InstructionAdd, Execute) {
	Instruction* i = new InstructionAdd<double>();
	PrimitiveType<double> a{ 2.6 };
	PrimitiveType<double> b = 5.5;
	PrimitiveType<int> c = 3;

	std::vector<std::reference_wrapper<SupportedType>> vect;
	vect.push_back(a);
	vect.push_back(b);	
	ASSERT_EQ(i->execute({} , vect), 8.1) << "Execute method of InstructionAdd<double> returns an incorrect value with valid operands.";

	vect.pop_back();
	vect.push_back(c);
	ASSERT_EQ(i->execute({}, vect), 0.0) << "Execute method of InstructionAdd<double> returns an incorrect value with invalid operands.";

	delete i;
	i = new InstructionMultByConstParam<double, int>();
	vect.pop_back();
	Parameter p = 2;
	ASSERT_EQ(i->execute({ p }, vect), 5.2) << "Execute method of InstructionMultByConstParam<double,int> returns an incorrect value with valid operands.";
	ASSERT_EQ(i->execute({ }, vect), 0.0) << "Execute method of InstructionMultByConstParam<double,int> returns an incorrect value with invalid params.";

}