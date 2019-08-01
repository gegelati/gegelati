#include <gtest/gtest.h>
#include "instructionAdd.h"
#include "instructionMultByConstParam.h"

TEST(Instructions, ConstructorDestructorCall) {
	Instructions::Instruction* i = new Instructions::AddPrimitiveType<double>();
	ASSERT_NE(i, nullptr) << "Call to constructor for AddPrimitiveType<double> failed.";
	delete i;
	i = new Instructions::AddPrimitiveType<int>();
	ASSERT_NE(i, nullptr) << "Call to constructor for AddPrimitiveType<int> failed.";
	delete i;
}

TEST(Instructions, OperandListAndNbParam) {
	Instructions::Instruction* i = new Instructions::AddPrimitiveType<double>();
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
	ASSERT_TRUE(i->checkParameters(v))  << "Parameter list of right size not detected as such.";
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
	ASSERT_EQ(i->execute({} , vect), 8.1) << "Execute method of AddPrimitiveType<double> returns an incorrect value with valid operands.";

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