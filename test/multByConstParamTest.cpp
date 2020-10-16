
#include <gtest/gtest.h>

#include <array>

#include "data/constantHandler.h"
#include "data/dataHandler.h"
#include "data/untypedSharedPtr.h"
#include "instructions/multByConstant.h"
#include "instructions/set.h"

TEST(MultByConstParamTest, ExecutePrimitiveType)
{
    int a{2};
    Data::Constant b{5};
    double c = 4.04;
    std::vector<Data::UntypedSharedPtr> vect;
    vect.emplace_back(&a, Data::UntypedSharedPtr::emptyDestructor<int>());
    vect.emplace_back(
        &b, Data::UntypedSharedPtr::emptyDestructor<Data::Constant>());

    Instructions::MultByConstant<int>* instruction;

    ASSERT_NO_THROW(instruction = new Instructions::MultByConstant<int>())
        << "Constructing a new multByConstParam Instruction failed.";

    ASSERT_EQ(instruction->getNbOperands(), 2)
        << "The multByConstant Instruction should use two operands.";
    ASSERT_EQ(instruction->execute(vect), 10)
        << "Result returned by the instruction is not as expected.";

    // Execute with wrong types of operands.
    vect.pop_back();
    vect.emplace_back(&c, Data::UntypedSharedPtr::emptyDestructor<double>());
#ifndef NDEBUG
    ASSERT_EQ(instruction->execute(vect), 0.0)
        << "Instructions executed with wrong types of operands should return "
           "0.0";
#else
    ASSERT_THROW(instruction->execute(vect), std::runtime_error)
        << "In NDEBUG mode, execution of a LambdaInstruction with wrong "
           "argument types should fail.";
#endif

    ASSERT_NO_THROW(delete instruction)
        << "Destruction of the MultByConstParam instruction failed.";
}