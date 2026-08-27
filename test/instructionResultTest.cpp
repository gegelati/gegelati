#include <gtest/gtest.h>

#include <algorithm>
#include <functional>
#include <initializer_list>
#include <vector>

#include "data/primitiveTypeArray.h"
#include "data/primitiveTypeArray2D.h"
#include "instructions/lambdaInstruction.h"

namespace {
template <typename T>
Data::UntypedSharedPtr makeScalar(T value)
{
    return Data::UntypedSharedPtr{new T(value)};
}

template <typename T>
Data::UntypedSharedPtr makeArray(std::initializer_list<T> values,
                                 Data::DataShape shape = Data::DataShape{1})
{
    T* data = new T[values.size()];
    std::copy(values.begin(), values.end(), data);
    return Data::UntypedSharedPtr{
        std::make_shared<Data::UntypedSharedPtr::Model<const T[]>>(data),
        shape};
}

}

TEST(InstructionResultTest, InstructionCanProduceAnInteger)
{
    Instructions::TypedLambdaInstruction<int, int, int> instruction(
        [](int left, int right) { return left * right; });
    const std::vector<Data::UntypedSharedPtr> args{
        makeScalar(4), makeScalar(3)};
    const Data::UntypedSharedPtr result =
        instruction.executeResult(args);

    EXPECT_EQ(result.getType(), typeid(int));
    EXPECT_EQ(result.getShape(), Data::DataShape({1}));
    EXPECT_EQ(*result.getSharedPointer<const int>(), 12);
}

TEST(InstructionResultTest, DoubleAdditionWritesDoubleRegister)
{
    Instructions::TypedLambdaInstruction<double, double, double> instruction(
        [](double left, double right) { return left + right; });
    const std::vector<Data::UntypedSharedPtr> args{
        makeScalar(1.25), makeScalar(2.75)};
    const Data::UntypedSharedPtr result =
        instruction.executeResult(args);
    Data::PrimitiveTypeArray<double> registers(4);

    registers.setDataAt(typeid(double), 2, result);

    EXPECT_DOUBLE_EQ(
        *registers.getDataAt(typeid(double), 2)
             .getSharedPointer<const double>(),
        4.0);
}

TEST(InstructionResultTest, IntegerMultiplicationWritesIntRegister)
{
    Instructions::TypedLambdaInstruction<int, int, int> instruction(
        [](int left, int right) { return left * right; });
    const std::vector<Data::UntypedSharedPtr> args{
        makeScalar(6), makeScalar(7)};
    const Data::UntypedSharedPtr result =
        instruction.executeResult(args);
    Data::PrimitiveTypeArray<int> registers(4);

    registers.setDataAt(typeid(int), 1, result);

    EXPECT_EQ(*registers.getDataAt(typeid(int), 1)
                   .getSharedPointer<const int>(),
              42);
}

TEST(InstructionResultTest, FloatVectorMultiplicationWritesVectorRegister)
{
    Instructions::TypedLambdaInstruction<float[4], const float[4], float>
        instruction([](const float* vector, float factor) {
            return makeArray<float>({vector[0] * factor, vector[1] * factor,
                                     vector[2] * factor, vector[3] * factor},
                                    {4});
        });
    const std::vector<Data::UntypedSharedPtr> args{
        makeArray<float>({1.0f, 2.0f, 3.0f, 4.0f}), makeScalar(2.0f)};
    const Data::UntypedSharedPtr result =
        instruction.executeResult(args);
    Data::PrimitiveTypeArray2D<float> registers(4, 7);

    registers.setDataAt(typeid(float[4]), 3, result);
    EXPECT_EQ(result.getShape(), Data::DataShape({4}));

    const auto written = registers.getDataAt(typeid(float[4]), 3)
                             .getSharedPointer<const float[]>();
    EXPECT_FLOAT_EQ(written.get()[0], 2.0f);
    EXPECT_FLOAT_EQ(written.get()[1], 4.0f);
    EXPECT_FLOAT_EQ(written.get()[2], 6.0f);
    EXPECT_FLOAT_EQ(written.get()[3], 8.0f);

    const auto untouched = registers.getDataAt(typeid(float[4]), 2)
                               .getSharedPointer<const float[]>();
    EXPECT_FLOAT_EQ(untouched.get()[0], 0.0f);
    EXPECT_FLOAT_EQ(untouched.get()[3], 0.0f);
}

TEST(InstructionResultTest, UntypedSharedPtrCarriesTypeAndShape)
{
    Data::UntypedSharedPtr result = makeArray<double>(
        {1.0, 2.0, 3.0, 4.0}, {2, 2});

    EXPECT_EQ(result.getType(), typeid(double[]));
    EXPECT_EQ(result.getShape(), Data::DataShape({2, 2}));
}

TEST(InstructionResultTest, ScalarRegisterTypeIsFixedByOwningArray)
{
    Data::PrimitiveTypeArray<double> doubleRegisters(8);
    Data::PrimitiveTypeArray<int> intRegisters(6);

    EXPECT_EQ(doubleRegisters.getAddressSpace(typeid(double)), 8);
    EXPECT_EQ(doubleRegisters.getAddressSpace(typeid(int)), 0);
    EXPECT_EQ(intRegisters.getAddressSpace(typeid(int)), 6);
    EXPECT_EQ(intRegisters.getAddressSpace(typeid(double)), 0);
}

TEST(InstructionResultTest, VectorRegisterTypeAndShapeAreFixedByTwoDArray)
{
    constexpr size_t vectorLength = 4;
    constexpr size_t registerCount = 7;
    Data::PrimitiveTypeArray2D<float> registers(vectorLength, registerCount);

    EXPECT_EQ(registers.getAddressSpace(typeid(float[vectorLength])),
              registerCount);
    EXPECT_EQ(registers.getAddressSpace(typeid(float[3])), registerCount * 2);
    EXPECT_EQ(registers.getAddressSpace(typeid(double[vectorLength])), 0);
}