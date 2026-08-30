#include <gtest/gtest.h>

#include <array>
#include <functional>

#include "data/constant.h"
#include "data/dataValue.h"
#include "instructions/lambdaInstruction.h"

#include <chrono>
#include <cmath>
#include <iostream>

TEST(LambdaInstructionsTest, ExecutePrimitiveType)
{
    double a = 2.6;
    double b = 5.5;
    Instructions::LambdaInstruction<double, double> instruction(
        [](double left, double right) { return left - right; });

    const Data::DataValue result = instruction.execute(
        {Data::DataView::scalar(a), Data::DataView::scalar(b)});
    ASSERT_DOUBLE_EQ(result.getScalar<double>(), -2.9);
}

TEST(LambdaInstructionsTest, ExecuteConstant)
{
    Data::Constant constant{4};
    double value = 2.6;
    Instructions::LambdaInstruction<Data::Constant, double> instruction(
        [](Data::Constant lhs, double rhs) { return double(lhs) * rhs; });

    const Data::DataValue result = instruction.execute(
        {Data::DataView::scalar(constant), Data::DataView::scalar(value)});
    ASSERT_DOUBLE_EQ(result.getScalar<double>(), 4.0 * 2.6);
}

TEST(LambdaInstructionsTest, ExecuteArray)
{
    const double first[] = {1.1, 2.2, 3.3};
    const double second[] = {6.5, 4.3, 2.1};
    Instructions::LambdaInstruction<const double[3], const double[3]>
        instruction([](const double left[3], const double right[3]) {
            return left[0] * right[0] + left[1] * right[1] +
                   left[2] * right[2];
        });

    const Data::DataValue result = instruction.execute(
        {Data::DataView::array(first, Data::DataShape{3}),
         Data::DataView::array(second, Data::DataShape{3})});
    ASSERT_DOUBLE_EQ(result.getScalar<double>(), 23.54);
}

TEST(LambdaInstructionsTest, ExecuteArray2D)
{
    const double first[] = {1.1, 2.2, 3.3, 4.4, 5.5, 6.6};
    const double second[] = {6.5, 4.3, 2.1, 9.8, 7.6, 5.4};
    Instructions::LambdaInstruction<const double[2][3], const double[2][3]>
        instruction([](const double left[2][3], const double right[2][3]) {
            double result = 0.0;
            for (size_t row = 0; row < 2; ++row) {
                for (size_t column = 0; column < 3; ++column) {
                    result += left[row][column] * right[row][column];
                }
            }
            return result;
        });

    const Data::DataValue result = instruction.execute(
        {Data::DataView::array(first, Data::DataShape{2, 3}),
         Data::DataView::array(second, Data::DataShape{2, 3})});
    ASSERT_DOUBLE_EQ(result.getScalar<double>(), 144.1);
}

TEST(LambdaInstructionsTest, ExecuteAllTypesMixed)
{
    Instructions::LambdaInstruction<double, double, int> instruction(
        [](double first, double second, int multiplier) {
            return (first + second) * multiplier;
        });
    double first = 1.0;
    double second = 1.1;
    int multiplier = 2;

    const Data::DataValue result = instruction.execute(
        {Data::DataView::scalar(first), Data::DataView::scalar(second),
         Data::DataView::scalar(multiplier)});
    ASSERT_DOUBLE_EQ(result.getScalar<double>(), 4.2);
}

#ifdef CODE_GENERATION
TEST(LambdaInstructionsTest, PrintConstructor)
{
    std::function<double(double, double)> minus =
        [](double left, double right) { return left - right; };
    Instructions::LambdaInstruction<double, double> instruction(
        minus, "$0 = $1 - $2;");
    SUCCEED();
}
#endif
