#include "code_gen/LambdaPrintableInstruction.h"
#include "code_gen/PrintableInstruction.h"
#include <data/constant.h>
#include <gtest/gtest.h>

TEST(LambdaPrintableInstructionTest, ConstructorDestructor)
{
    std::function<double(double, double)> add;
    std::function<double(double, double)> sub;
    std::function<double(double, double)> mult = nullptr;
    Instructions::LambdaPrintableInstruction<double, double>* instruction;

    add = [](double a, double b) -> double { return a + b; };
    sub = [](double a, double b) -> double { return a - b; };
    ASSERT_NO_THROW(
        (instruction =
             new Instructions::LambdaPrintableInstruction<double, double>(
                 "$0 = $1 + $2;", add)));

    ASSERT_NE(instruction, nullptr) << "Construction failed";

    ASSERT_NO_THROW(delete instruction) << "Destruction failed.";

    ASSERT_NO_THROW(
        (instruction =
             new Instructions::LambdaPrintableInstruction<double, double>("",
                                                                          sub)))
        << "Construction failed with format empty";

    ASSERT_NO_THROW(delete instruction) << "Destruction failed.";

    ASSERT_NO_THROW(
        (instruction =
             new Instructions::LambdaPrintableInstruction<double, double>(
                 "$0 = $1 + $2;", mult)));
}

TEST(LambdaPrintableInstructionTest, getFormat)
{
    std::function<double(double, double)> add;
    Instructions::LambdaPrintableInstruction<double, double>* instruction;

    add = [](double a, double b) -> double { return a + b; };

    ASSERT_NO_THROW(
        (instruction =
             new Instructions::LambdaPrintableInstruction<double, double>(
                 "$0 = $1 + $2;", add)))
        << "Construction failed";

    ASSERT_EQ(instruction->getFormat(), "$0 = $1 + $2;")
        << "The format of the instruction is not the one given at the "
           "construction of the object";
}
