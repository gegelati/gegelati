
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

TEST(InstructionSetTest, filterSet) {
    
    Instructions::Set s;
    Instructions::AddPrimitiveType<float> iAdd; // one operand
    
    auto minus = [](double a, double b) -> double {
        return a - b;
    }; // two operands
    Instructions::LambdaInstruction<double, double, double> iMinus(minus);

    s.add(iAdd);
    s.add(iMinus);

    Data::DataType output = Data::DataType::scalar<float>();
    Data::DataType type = Data::DataType::scalar<float>();
    Instructions::Set clone = s.filterInstructionSet({type}, output);
    ASSERT_EQ(clone.getNbInstructions(), 1) << "Should contain only one instruction";
    ASSERT_TRUE(dynamic_cast<const Instructions::AddPrimitiveType<float>*>(&clone.getInstruction(0)) != nullptr) << "Should be the primitive type!";

    
    Data::DataType type2 = Data::DataType::scalar<int>();
    Instructions::Set clone2 = s.filterInstructionSet({type2}, output);
    ASSERT_EQ(clone2.getNbInstructions(), 0) << "Should be empty";

    ASSERT_THROW(s.filterInstructionSet({}, output), std::runtime_error) << "Should throw with empty list";   

    Instructions::LambdaInstruction<double[2], const double[2][3], const double[3][2]>
        iBig([](const double left[2][3], const double right[3][2]) {
            return Data::DataValue::array1d<double[2]>({0.0, 0.0});
        });

    Instructions::Set s2;
    s2.add(iBig);
    // Input check
    ASSERT_EQ(s2.filterInstructionSet({Data::DataType::scalar<double>()}, Data::DataType::array1d<double>(2)).getNbInstructions(), 0) << "Should not have keep the instruction";
    ASSERT_EQ(s2.filterInstructionSet({Data::DataType::array1d<double>(2)}, Data::DataType::array1d<double>(2)).getNbInstructions(), 0) << "Should not have keep the instruction";
    ASSERT_EQ(s2.filterInstructionSet({Data::DataType::array1d<double>(4)}, Data::DataType::array1d<double>(2)).getNbInstructions(), 0) << "Should not have keep the instruction";
    ASSERT_EQ(s2.filterInstructionSet({Data::DataType::array2d<double>(2, 2)}, Data::DataType::array1d<double>(2)).getNbInstructions(), 0) << "Should not have keep the instruction";
    ASSERT_EQ(s2.filterInstructionSet({Data::DataType::array2d<double>(3, 3)}, Data::DataType::array1d<double>(2)).getNbInstructions(), 1) << "Should have keep the instruction";
    ASSERT_EQ(s2.filterInstructionSet({Data::DataType::array2d<double>(3, 2), Data::DataType::array2d<double>(2, 3)}, Data::DataType::array1d<double>(2)).getNbInstructions(), 1) << "Should have keep the instruction";
    ASSERT_EQ(s2.filterInstructionSet({Data::DataType::scalar<float>()}, Data::DataType::array1d<double>(2)).getNbInstructions(), 0) << "Should not have keep the instruction";
    // output check
    ASSERT_EQ(s2.filterInstructionSet({Data::DataType::array2d<double>(3, 3)}, Data::DataType::scalar<double>()).getNbInstructions(), 0) << "Should not have keep the instruction";
    ASSERT_EQ(s2.filterInstructionSet({Data::DataType::array2d<double>(3, 3)}, Data::DataType::array1d<double>(3)).getNbInstructions(), 1) << "Should have keep the instruction";
    ASSERT_EQ(s2.filterInstructionSet({Data::DataType::array2d<double>(3, 3)}, Data::DataType::array1d<double>(20)).getNbInstructions(), 1) << "Should have keep the instruction";
    ASSERT_EQ(s2.filterInstructionSet({Data::DataType::array2d<double>(3, 3)}, Data::DataType::array2d<double>(2, 2)).getNbInstructions(), 1) << "Should have keep the instruction";
    ASSERT_EQ(s2.filterInstructionSet({Data::DataType::array2d<double>(3, 3)}, Data::DataType::array1d<float>(2)).getNbInstructions(), 0) << "Should not have keep the instruction";
    
}