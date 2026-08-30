#include <gtest/gtest.h>

#include <algorithm>
#include <initializer_list>
#include <memory>
#include <vector>

#include "data/dataValue.h"
#include "instructions/lambdaInstruction.h"

namespace {
}

template <typename T>
Data::DataValue makeValueArray(std::initializer_list<T> values)
{
    std::unique_ptr<T[]> data(new T[values.size()]);
    std::copy(values.begin(), values.end(), data.get());
    return Data::DataValue::array(std::move(data), values.size());
}

TEST(InstructionResultTest, InstructionCanProduceAnInteger)
{
    Instructions::TypedLambdaInstruction<int, int, int> instruction(
        [](int left, int right) { return left * right; });
    int left = 4;
    int right = 3;
    const Data::DataValue result = instruction.execute(
        {Data::DataView::scalar(left), Data::DataView::scalar(right)});

    EXPECT_EQ(result.type(), typeid(int));
    EXPECT_EQ(result.shape(), Data::DataShape({1}));
    EXPECT_EQ(result.getScalar<int>(), 12);
}

TEST(InstructionResultTest, TypedLambdaUsesBorrowedInputs)
{
    Instructions::TypedLambdaInstruction<double, int, double> instruction(
        [](int left, double right) { return left + right; });
    int left = 3;
    double right = 2.75;
    const Data::DataValue result = instruction.execute(
        {Data::DataView::scalar(left), Data::DataView::scalar(right)});
    EXPECT_DOUBLE_EQ(result.getScalar<double>(), 5.75);
}

TEST(InstructionResultTest, IntegerLambdaProducesTypedResult)
{
    Instructions::TypedLambdaInstruction<int, int, int> instruction(
        [](int left, int right) { return left * right; });
    int left = 6;
    int right = 7;
    const Data::DataValue result = instruction.execute(
        {Data::DataView::scalar(left), Data::DataView::scalar(right)});
    EXPECT_EQ(result.getScalar<int>(), 42);
}

TEST(InstructionResultTest, TypedLambdaPreservesOwnedArrayResult)
{
    Instructions::TypedLambdaInstruction<float[4], const float[4], float>
        instruction([](const float* vector, float factor) {
            return Data::DataValue::array<float>(
                std::unique_ptr<float[]>(new float[4]{vector[0] * factor,
                                                       vector[1] * factor,
                                                       vector[2] * factor,
                                                       vector[3] * factor}),
                4);
        });
    const float input[] = {1.0f, 2.0f, 3.0f, 4.0f};
    float factor = 2.0f;
    const Data::DataValue result = instruction.execute(
        {Data::DataView::array(input, Data::DataShape{4}),
         Data::DataView::scalar(factor)});
    const float* values = result.getArray<float>();
    EXPECT_EQ(result.type(), typeid(float[]));
    EXPECT_EQ(result.shape(), Data::DataShape({4}));
    EXPECT_FLOAT_EQ(values[0], 2.0f);
    EXPECT_FLOAT_EQ(values[3], 8.0f);
}


TEST(InstructionResultTest, TypedLambdaPreservesOwnedArrayResult2)
{
    Instructions::TypedLambdaInstruction<int[4], const float[4], float>
        instruction([](const float* vector, float factor) {
            return Data::DataValue::array<int>(
                std::unique_ptr<int[]>(new int[4]{int(vector[0] * factor),
                                                    int(vector[1] * factor),
                                                    int(vector[2] * factor),
                                                    int(vector[3] * factor)}),
                4);
        });
    const float input[] = {1.0f, 2.0f, 3.0f, 4.0f};
    float factor = 2.0f;
    const Data::DataValue result = instruction.execute(
        {Data::DataView::array(input, Data::DataShape{4}),
         Data::DataView::scalar(factor)});
    const int* values = result.getArray<int>();
    EXPECT_EQ(result.type(), typeid(int[]));
    EXPECT_EQ(result.shape(), Data::DataShape({4}));
    EXPECT_EQ(values[0], 2);
    EXPECT_EQ(values[3], 8);


    ASSERT_THROW(const float* values2 = result.getArray<float>(), std::runtime_error) << "Crash ?";
}

TEST(InstructionResultTest, TypedLambdaRejectsWrongScalarResultType)
{
    Instructions::TypedLambdaInstruction<int, int> instruction(
        [](int value) { return static_cast<double>(value); });
    int value = 3;

    EXPECT_THROW(instruction.execute({Data::DataView::scalar(value)}),
                 std::invalid_argument);
}