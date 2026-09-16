
#include <gtest/gtest.h>

#include <functional>

#include "oldData/dataValue.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/lambdaInstruction.h"
#include "instructions/multByConstant.h"
#include "instructions/set.h"

TEST(InstructionSetTest, SetAdd)
{
    Instructions::Set s;

    Instructions::AddPrimitiveType<double> i1;
    Instructions::AddPrimitiveType<double> i2;
    Instructions::AddPrimitiveType<float> i3;

    ASSERT_TRUE(s.add(i1))
        << "Add of instruction to empty Instructions::Set failed.";
    // Adding equivalent instructions is no longer forbidden.
    ASSERT_TRUE(s.add(i2)) << "Add of instruction already present in an "
                              "Instructions::Set should not fail.";
    ASSERT_TRUE(s.add(i3))
        << "Add of instruction to non empty Instructions::Set failed. (with a "
           "template instruction with different template param than an already "
           "present one";
}

TEST(InstructionSetTest, SetGetNbInstruction)
{
    Instructions::Set s;

    ASSERT_EQ(s.getNbInstructions(), 0)
        << "Incorrect number of instructions in an empty Set.";

    Instructions::AddPrimitiveType<double> i1;
    Instructions::AddPrimitiveType<float> i2;
    s.add(i1);
    s.add(i2);
    ASSERT_EQ(s.getNbInstructions(), 2)
        << "Incorrect number of instructions in a non-empty Set.";
}

TEST(InstructionSetTest, SetGetInstruction)
{
    Instructions::Set s;

    Instructions::AddPrimitiveType<float> iAdd;
    Instructions::AddPrimitiveType<double> iAddDouble;
    s.add(iAdd);
    s.add(iAddDouble);

    const Instructions::Instruction* res;
    ASSERT_NO_THROW(res = &s.getInstruction(1))
        << "Exception was thrown unexpectedly when calling Set::getInstruction "
           "with a valid index.";

    // Compare that the returned reference points to the right object.
    ASSERT_EQ(res, &iAddDouble)
        << "Incorrect Instruction was returned by valid Set::getInstruction.";

    // Check that exception is thrown when an invalid index is given.
    ASSERT_THROW(res = &s.getInstruction(2), std::out_of_range)
        << "Exception was not thrown when calling Set::getInstruction with an "
           "invalid index.";
}

TEST(InstructionSetTest, SetGetNbMaxOperands)
{
    Instructions::Set s;

    ASSERT_EQ(s.getMaxNbOperands(), 0)
        << "Max number of operands returned by the empty Instructions::Set is "
           "incorrect.";

    Instructions::AddPrimitiveType<float> iAdd; // one operand
    auto minus = [](double a, double b) -> double {
        return a - b;
    }; // two operands

    s.add(iAdd);
    s.add(Instructions::LambdaInstruction<double, double, double>(minus));

    ASSERT_EQ(s.getMaxNbOperands(), 2) << "Max number of operands returned by "
                                          "the Instructions::Set is incorrect.";
}