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
    double valueA = 2.6;
    double valueB = 5.5;
    Data::DataView b(valueB);

    Instructions::LambdaInstruction<double, double, double> instruction(
        [](double left, double right) { return left - right; });

    const Data::DataValue result = instruction.execute({Data::DataView(valueA), b});
    ASSERT_DOUBLE_EQ(result.getScalar<double>(), -2.9);
}

TEST(LambdaInstructionsTest, ExecuteConstant)
{
    Data::Constant constant{4};
    double valueA = 2.6;
    Instructions::LambdaInstruction<double, Data::Constant, double> instruction(
        [](Data::Constant lhs, double rhs) { return double(lhs) * rhs; });

    Data::DataView constantValue(constant);
    Data::DataView doubleValue(valueA);

    const Data::DataValue result = instruction.execute(
        {constant, doubleValue});
    ASSERT_DOUBLE_EQ(result.getScalar<double>(), 4.0 * 2.6);
}

TEST(LambdaInstructionsTest, ExecuteArray)
{
    const double first[] = {1.1, 2.2, 3.3};
    const double second[] = {6.5, 4.3, 2.1};
    Instructions::LambdaInstruction<double, const double[3], const double[3]>
        instruction([](const double left[3], const double right[3]) {
            return left[0] * right[0] + left[1] * right[1] +
                   left[2] * right[2];
        });

    const Data::DataValue result = instruction.execute(
        {Data::DataView(first, Data::DataType::array1d<double>(3)),
         Data::DataView(second, Data::DataType::array1d<double>(3))});
    ASSERT_DOUBLE_EQ(result.getScalar<double>(), 23.54);
}

TEST(LambdaInstructionsTest, ExecuteArray2D)
{
    const double first[] = {1.1, 2.2, 3.3, 4.4, 5.5, 6.6};
    const double second[] = {6.5, 4.3, 2.1, 9.8, 7.6, 5.4};
    Instructions::LambdaInstruction<double, const double[2][3], const double[2][3]>
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
        {Data::DataView(first, Data::DataType::array2d<double>(2, 3)),
         Data::DataView(second, Data::DataType::array2d<double>(2, 3))});
    ASSERT_DOUBLE_EQ(result.getScalar<double>(), 144.1);
}

TEST(LambdaInstructionsTest, ExecuteAllTypesMixed)
{
    Instructions::LambdaInstruction<double, double, double, int> instruction(
        [](double first, double second, int multiplier) {
            return (first + second) * multiplier;
        });
    double first = 1.0;
    double second = 1.1;
    int multiplier = 2;

    const Data::DataValue result = instruction.execute(
        {Data::DataView(&first, Data::DataType::scalar<double>()), Data::DataView(&second, Data::DataType::scalar<double>()),
         Data::DataView(&multiplier, Data::DataType::scalar<int>())});
    ASSERT_DOUBLE_EQ(result.getScalar<double>(), 4.2);
    
}



TEST(LambdaInstructionsTest, productFloatOutput) 
{
    // float multiplication 
    Instructions::LambdaInstruction<float, float, float>
    instruction([](float v1, float v2) {
        return v1*v2;
    });

    
    float v1 = 2.0;
    float v2 = 3.5;
    const Data::DataValue result = instruction.execute({
        Data::DataView(&v1, Data::DataType::scalar<float>()),
        Data::DataView(&v2, Data::DataType::scalar<float>())
    });
    ASSERT_EQ(result.getScalar<float>(), 7.0) << "Value mismatch";
}

TEST(LambdaInstructionsTest, executeVectoProduct) 
{
    // Vector multiplication
    Instructions::LambdaInstruction<float[4], const float[4], float>
    instruction([](const float* vector, float factor) {
        return Data::DataValue::array1d<float[4]>({vector[0] * factor, vector[1] * factor, vector[2] * factor, vector[3] * factor});
    });

    
    const float input1[] = {1.0f, 2.0f, 3.0f, 4.0f};
    float factor = 2.0f;
    const Data::DataValue result1 = instruction.execute(
        {Data::DataView(input1, Data::DataType::array1d<float>(4)),
         Data::DataView(&factor, Data::DataType::scalar<float>())});
    const float* arrayResult1 = result1.getArray<float>();
    for(size_t idx = 0; idx < 4; idx++) {
        ASSERT_EQ(arrayResult1[idx], input1[idx] * factor) << "Value mismatch";
    }

    
    const float input2[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};
    Data::DataView viewInput2(input2, Data::DataType::array1d<float>(6));
    Data::DataView subView = viewInput2.getSubView(Data::DataType::array1d<float>(4), 2);

    const Data::DataValue result2 = instruction.execute(
        {subView,
         Data::DataView(&factor, Data::DataType::scalar<float>())});
        
    const float* arrayResult2 = result2.getArray<float>();
    for(size_t idx = 0; idx < 4; idx++) {
        ASSERT_EQ(arrayResult2[idx], input2[idx+2] * factor) << "Value mismatch";
    }
}


TEST(LambdaInstructionsTest, executeMatrixProduct) 
{
    // Matrix multiplication
    Instructions::LambdaInstruction<float[2], const float[2][2], const float[2]> instruction([](const float m[2][2], const float v[2]){
        return Data::DataValue::array1d<float[2]>({(m[0][0] * v[0]) + (m[0][1] * v[1]), (m[1][0] * v[0]) + (m[1][1] * v[1])});
    });

    const float matrix[2][2] = {{1, 2}, {4, 3}};
    const float vector[2] = {1, -1};
    /**
     * r0 = 1 * 1 + 2 * -1 = -1
     * r1 = 4 * 1 + 3 * -1 = 1
     */
    const Data::DataValue result1 = instruction.execute(
        {Data::DataView(matrix, Data::DataType::array2d<float>(2, 2)),
         Data::DataView(vector, Data::DataType::array1d<float>(2))});
    
    const float expected[2] = {-1, 1};
    const float* arrayResult1 = result1.getArray<float>();
    for(size_t idx = 0; idx < 2; idx++) {
        ASSERT_EQ(arrayResult1[idx], expected[idx]) << "Value mismatch";
    }

    const float matrixBig[4][4] = {
        {0, 0, 0, 0},
        {0, 1, 2, 0},
        {0, 4, 3, 0},
        {0, 0, 0, 0}
    };
    Data::DataView viewMatrix(matrixBig, Data::DataType::array2d<float>(4, 4));
    Data::DataView subViewMatrix = viewMatrix.getSubView(Data::DataType::array2d<float>(2, 2), 5);

    const Data::DataValue result2 = instruction.execute(
        {subViewMatrix,
         Data::DataView(vector, Data::DataType::array1d<float>(2))});

    const float* arrayResult2 = result2.getArray<float>();
    for(size_t idx = 0; idx < 2; idx++) {
        ASSERT_EQ(arrayResult2[idx], expected[idx]) << "Value mismatch";
    }

}


TEST(LambdaInstructionsTest, executeMatrixProductMixTypes) 
{
    Instructions::LambdaInstruction<float[2][2], const double[2][2], const int[2][2]> instruction(
        [](const double m1[2][2], const int m2[2][2]){
        return Data::DataValue::array2d<float[2][2]>(
            {{float(m1[0][0] * m2[0][0]) + float(m1[0][1] * m2[1][0]), float(m1[0][0] * m2[0][1]) + float(m1[0][1] * m2[1][1])},
             {float(m1[1][0] * m2[0][0]) + float(m1[1][1] * m2[1][0]), float(m1[1][0] * m2[0][1]) + float(m1[1][1] * m2[1][1])}});
    });

    const double matrix1[3][4] = {
        {1, 0.5, -0.5, -2},
        {0,   1,   -2, 4},
        {0,  -4,    3, 3},};

    const int matrix2[4][2] = {
        {1, 0},
        {5, 1},
        {4, 4},
        {0, 0}};

    Data::DataView matrix1View(matrix1);
    Data::DataView matrix2View(matrix2);

    Data::DataView matrix1SubView = matrix1View.getSubView(Data::DataType::array2d<double>(2, 2), 5);
    Data::DataView matrix2SubView = matrix2View.getSubView(Data::DataType::array2d<int>(2, 2), 2);

    /**
     * M1 = | 1, -2| -- M2 = |5, 1|
     *      |-4,  3|         |4, 4|
     * 
     * R  = | 1*5 - 2*4,  1*1 - 2*4|  =  | -3, -7|
     *      |-4*5 + 3*4, -4*1 + 3*4|     | -8,  8|
     */
    float expected[4] = {-3, -7, -8, 8};
    Data::DataValue result = instruction.execute({matrix1SubView, matrix2SubView});
    const float* array = result.getArray<float>();
    for (size_t idx = 0; idx < 4; idx++) {
        ASSERT_EQ(array[idx], expected[idx]) << "Value mismatch";
    }

}

#ifdef CODE_GENERATION
TEST(LambdaInstructionsTest, PrintConstructor)
{
    std::function<double(double, double)> minus =
        [](double left, double right) { return left - right; };
    Instructions::LambdaInstruction<double, double, double> instruction(
        minus, "$0 = $1 - $2;");
    SUCCEED();
}
#endif
