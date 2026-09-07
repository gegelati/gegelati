#include <gtest/gtest.h>

#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "dimensions/activationFunctions.h"
#include "dimensions/requirement.h"
#include "dimensions/numericRange.h"
#include "data/dataValue.h"

namespace
{
constexpr double kTolerance = 1e-6;
}

// ============================================================================
// Tanh
// ============================================================================

TEST(ActivationFunctionsTanhTest, HasCorrectName)
{
const auto input = Dimensions::Requirement::scalar<double>();
const Dimensions::ActivationFunctions::Tanh<double> tanh(input);


EXPECT_EQ(
    tanh.name(),
    "TanH")
    << "Tanh must expose the expected function name.";


}

TEST(ActivationFunctionsTanhTest, ExposesInputDimension)
{
const auto input = Dimensions::Requirement::array1d<double>(3);
const Dimensions::ActivationFunctions::Tanh<double> tanh(input);


ASSERT_EQ(
    tanh.inputDimensions().size(),
    1u)
    << "Tanh must have exactly one input dimension.";

EXPECT_EQ(
    tanh.inputDimensions()[0],
    input)
    << "Tanh inputDimensions() must return the same requirement supplied to its constructor.";


}

TEST(ActivationFunctionsTanhTest, OutputDimensionPreservesInputDataTypeAndShape)
{
const auto input = Dimensions::Requirement::array1d<double>(3);
const Dimensions::ActivationFunctions::Tanh<double> tanh(input);


const auto expectedOutputType = Data::DataType::array1d<double>(3);

EXPECT_EQ(
    tanh.outputDimension().getDataType(),
    expectedOutputType)
    << "Tanh must preserve the input data type and shape in its output dimension.";


}

TEST(ActivationFunctionsTanhTest, OutputDimensionIsConstrainedToMinusOneAndOne)
{
const auto input = Dimensions::Requirement::scalar<double>();
const Dimensions::ActivationFunctions::Tanh<double> tanh(input);


const auto& output = tanh.outputDimension();

EXPECT_TRUE(
    output.accepts(Data::DataValue::scalar(-1.0).view()))
    << "Tanh output dimension must accept the lower bound -1.";

EXPECT_TRUE(
    output.accepts(Data::DataValue::scalar(0.0).view()))
    << "Tanh output dimension must accept zero.";

EXPECT_TRUE(
    output.accepts(Data::DataValue::scalar(1.0).view()))
    << "Tanh output dimension must accept the upper bound 1.";

EXPECT_FALSE(
    output.accepts(Data::DataValue::scalar(-1.000001).view()))
    << "Tanh output dimension must reject values below -1.";

EXPECT_FALSE(
    output.accepts(Data::DataValue::scalar(1.000001).view()))
    << "Tanh output dimension must reject values above 1.";


}

TEST(ActivationFunctionsTanhTest, ExecutesCorrectlyForScalar)
{
const auto input = Dimensions::Requirement::scalar<double>();
const Dimensions::ActivationFunctions::Tanh<double> tanh(input);


const auto result =
    tanh.execute(Data::DataValue::scalar(0.5));

ASSERT_EQ(
    result.getRank(),
    0u)
    << "Tanh applied to a scalar must return a scalar.";

EXPECT_NEAR(
    result.getData<double>()[0],
    std::tanh(0.5),
    kTolerance)
    << "Tanh must calculate the mathematically correct hyperbolic tangent for a scalar.";


}

TEST(ActivationFunctionsTanhTest, ExecutesCorrectlyForZero)
{
const auto input = Dimensions::Requirement::scalar<double>();
const Dimensions::ActivationFunctions::Tanh<double> tanh(input);


const auto result =
    tanh.execute(Data::DataValue::scalar(0.0));

EXPECT_DOUBLE_EQ(
    result.getData<double>()[0],
    0.0)
    << "The hyperbolic tangent of zero must be exactly zero.";


}

TEST(ActivationFunctionsTanhTest, ExecutesCorrectlyForPositiveValue)
{
const auto input = Dimensions::Requirement::scalar<double>();
const Dimensions::ActivationFunctions::Tanh<double> tanh(input);


const auto result =
    tanh.execute(Data::DataValue::scalar(2.0));

EXPECT_NEAR(
    result.getData<double>()[0],
    std::tanh(2.0),
    kTolerance)
    << "Tanh must correctly transform positive scalar values.";


}

TEST(ActivationFunctionsTanhTest, ExecutesCorrectlyForNegativeValue)
{
const auto input = Dimensions::Requirement::scalar<double>();
const Dimensions::ActivationFunctions::Tanh<double> tanh(input);


const auto result =
    tanh.execute(Data::DataValue::scalar(-2.0));

EXPECT_NEAR(
    result.getData<double>()[0],
    std::tanh(-2.0),
    kTolerance)
    << "Tanh must correctly transform negative scalar values.";


}

TEST(ActivationFunctionsTanhTest, ExecutesCorrectlyForOneDimensionalArray)
{
const auto input = Dimensions::Requirement::array1d<double>(5);
const Dimensions::ActivationFunctions::Tanh<double> tanh(input);


const std::vector<double> source{
    -2.0,
    -0.5,
    0.0,
    0.5,
    2.0
};

const auto result =
    tanh.execute(Data::DataValue::array1d(source));

ASSERT_EQ(
    result.getRank(),
    1u)
    << "Tanh applied to a one-dimensional array must return a one-dimensional array.";

ASSERT_EQ(
    result.getType().totalElements(),
    source.size())
    << "Tanh must preserve the number of elements in a one-dimensional array.";

const double* values = result.getData<double>();

for (size_t i = 0; i < source.size(); ++i) {
    EXPECT_NEAR(
        values[i],
        std::tanh(source[i]),
        kTolerance)
        << "Each element of a one-dimensional array must be transformed independently by tanh.";
}


}

TEST(ActivationFunctionsTanhTest, ExecutesCorrectlyForTwoDimensionalArray)
{
const auto input = Dimensions::Requirement::array2d<double>(2, 3);
const Dimensions::ActivationFunctions::Tanh<double> tanh(input);


const std::vector<std::vector<double>> source{
    {-2.0, -0.5, 0.0},
    {0.5, 1.0, 2.0}
};

const auto result =
    tanh.execute(Data::DataValue::array2d(source));

ASSERT_EQ(
    result.getRank(),
    2u)
    << "Tanh applied to a two-dimensional array must return a two-dimensional array.";

EXPECT_EQ(
    result.getType().dimensions[0],
    2u)
    << "Tanh must preserve the first dimension of a two-dimensional array.";

EXPECT_EQ(
    result.getType().dimensions[1],
    3u)
    << "Tanh must preserve the second dimension of a two-dimensional array.";

ASSERT_EQ(
    result.getType().totalElements(),
    6u)
    << "Tanh must preserve the total number of elements in a two-dimensional array.";

const double* values = result.getData<double>();

for (size_t i = 0; i < source.size(); ++i) {
    for (size_t j = 0; j < source[i].size(); ++j) {
        const size_t index = i * source[i].size() + j;

        EXPECT_NEAR(
            values[index],
            std::tanh(source[i][j]),
            kTolerance)
            << "Each element of a two-dimensional array must be transformed independently by tanh.";
    }
}


}

TEST(ActivationFunctionsTanhTest, PreservesOutputElementType)
{
const auto input = Dimensions::Requirement::array1d<float>(3);
const Dimensions::ActivationFunctions::Tanh<float> tanh(input);


const auto result =
    tanh.execute(Data::DataValue::array1d(
        std::vector<float>{-1.0f, 0.0f, 1.0f}));

EXPECT_EQ(
    result.getType().elementType,
    Data::DataType::scalar<float>().elementType)
    << "Tanh<float> must produce float output values.";


}

TEST(ActivationFunctionsTanhTest, FloatExecutionIsNumericallyCorrect)
{
const auto input = Dimensions::Requirement::scalar<float>();
const Dimensions::ActivationFunctions::Tanh<float> tanh(input);


const float source = 0.75f;

const auto result =
    tanh.execute(Data::DataValue::scalar(source));

EXPECT_NEAR(
    result.getData<float>()[0],
    std::tanh(source),
    1e-6f)
    << "Tanh<float> must produce a numerically correct floating-point result.";


}

TEST(ActivationFunctionsTanhTest, SaturatesForLargePositiveInput)
{
const auto input = Dimensions::Requirement::scalar<double>();
const Dimensions::ActivationFunctions::Tanh<double> tanh(input);


const auto result =
    tanh.execute(Data::DataValue::scalar(1000.0));

EXPECT_NEAR(
    result.getData<double>()[0],
    1.0,
    kTolerance)
    << "Tanh must approach +1 for very large positive inputs.";


}

TEST(ActivationFunctionsTanhTest, SaturatesForLargeNegativeInput)
{
const auto input = Dimensions::Requirement::scalar<double>();
const Dimensions::ActivationFunctions::Tanh<double> tanh(input);


const auto result =
    tanh.execute(Data::DataValue::scalar(-1000.0));

EXPECT_NEAR(
    result.getData<double>()[0],
    -1.0,
    kTolerance)
    << "Tanh must approach -1 for very large negative inputs.";


}

TEST(ActivationFunctionsTanhTest, PreservesOddFunctionSymmetry)
{
const auto input = Dimensions::Requirement::scalar<double>();
const Dimensions::ActivationFunctions::Tanh<double> tanh(input);


const auto positive =
    tanh.execute(Data::DataValue::scalar(1.25));

const auto negative =
    tanh.execute(Data::DataValue::scalar(-1.25));

EXPECT_NEAR(
    positive.getData<double>()[0],
    -negative.getData<double>()[0],
    kTolerance)
    << "Tanh is an odd function, so tanh(-x) must equal -tanh(x).";


}

TEST(ActivationFunctionsTanhTest, OutputDimensionAcceptsActualTransformedArray)
{
const auto input = Dimensions::Requirement::array1d<double>(4);
const Dimensions::ActivationFunctions::Tanh<double> tanh(input);


const auto result =
    tanh.execute(Data::DataValue::array1d(
        std::vector<double>{-100.0, -1.0, 1.0, 100.0}));

EXPECT_TRUE(
    tanh.outputDimension().accepts(result.view()))
    << "The output produced by Tanh must satisfy the function's declared output dimension.";


}

TEST(ActivationFunctionsTanhTest, InputDimensionRejectsWrongShape)
{
const auto input = Dimensions::Requirement::array1d<double>(3);
const Dimensions::ActivationFunctions::Tanh<double> tanh(input);


const auto wrongShape =
    Data::DataValue::array1d(std::vector<double>{1.0, 2.0});

EXPECT_FALSE(
    tanh.inputDimensions()[0].accepts(wrongShape.view()))
    << "Tanh input dimension must reject an array with the wrong shape.";


}

TEST(ActivationFunctionsTanhTest, InputDimensionRejectsWrongType)
{
const auto input = Dimensions::Requirement::array1d<double>(3);
const Dimensions::ActivationFunctions::Tanh<double> tanh(input);


const auto wrongType =
    Data::DataValue::array1d(std::vector<float>{1.0f, 2.0f, 3.0f});

EXPECT_FALSE(
    tanh.inputDimensions()[0].accepts(wrongType.view()))
    << "Tanh input dimension must reject data with the wrong element type.";


}

TEST(ActivationFunctionsTanhTest, SupportsPolymorphicFunctionInterface)
{
const auto input = Dimensions::Requirement::scalar<double>();


std::unique_ptr<Dimensions::ActivationFunctions::Function> function =
    std::make_unique<Dimensions::ActivationFunctions::Tanh<double>>(input);

EXPECT_EQ(
    function->name(),
    "TanH")
    << "Tanh must remain usable through the polymorphic Function interface.";

const auto result =
    function->execute(Data::DataValue::scalar(0.5));

EXPECT_NEAR(
    result.getData<double>()[0],
    std::tanh(0.5),
    kTolerance)
    << "The polymorphic Function interface must execute Tanh correctly.";


}

// ============================================================================
// ArgMax
// ============================================================================

TEST(ActivationFunctionsArgMaxTest, HasCorrectName)
{
const auto input = Dimensions::Requirement::array1d<double>(4);
const Dimensions::ActivationFunctions::ArgMax<double> argmax(input);


EXPECT_EQ(
    argmax.name(),
    "ArgMax")
    << "ArgMax must expose the expected function name.";


}

TEST(ActivationFunctionsArgMaxTest, HasExactlyOneInputDimension)
{
const auto input = Dimensions::Requirement::array1d<double>(4);
const Dimensions::ActivationFunctions::ArgMax<double> argmax(input);


EXPECT_EQ(
    argmax.inputDimensions().size(),
    1u)
    << "ArgMax must have exactly one input dimension.";


}

TEST(ActivationFunctionsArgMaxTest, PreservesInputDimensionAsInputRequirement)
{
const auto input = Dimensions::Requirement::array1d<double>(4);
const Dimensions::ActivationFunctions::ArgMax<double> argmax(input);


ASSERT_EQ(
    argmax.inputDimensions().size(),
    1u)
    << "ArgMax must expose exactly one input requirement.";

EXPECT_EQ(
    argmax.inputDimensions()[0],
    input)
    << "ArgMax must expose the same input requirement supplied to its constructor.";


}

TEST(ActivationFunctionsArgMaxTest, OutputIsScalarSizeT)
{
const auto input = Dimensions::Requirement::array1d<double>(4);
const Dimensions::ActivationFunctions::ArgMax<double> argmax(input);


EXPECT_EQ(
    argmax.outputDimension().getDataType(),
    Data::DataType::scalar<size_t>())
    << "ArgMax must produce a scalar size_t index.";


}

TEST(ActivationFunctionsArgMaxTest, OutputRangeCoversAllValidIndices)
{
const auto input = Dimensions::Requirement::array1d<double>(4);
const Dimensions::ActivationFunctions::ArgMax<double> argmax(input);


const auto& output = argmax.outputDimension();

EXPECT_TRUE(
    output.accepts(Data::DataValue::scalar<size_t>(0).view()))
    << "ArgMax output dimension must accept index zero.";

EXPECT_TRUE(
    output.accepts(Data::DataValue::scalar<size_t>(3).view()))
    << "ArgMax output dimension must accept the last valid index.";

EXPECT_FALSE(
    output.accepts(Data::DataValue::scalar<size_t>(4).view()))
    << "ArgMax output dimension must reject an index equal to the element count.";

EXPECT_FALSE(
    output.accepts(Data::DataValue::scalar<size_t>(100).view()))
    << "ArgMax output dimension must reject indices beyond the valid range.";


}

TEST(ActivationFunctionsArgMaxTest, ReturnsIndexOfLargestElement)
{
const auto input = Dimensions::Requirement::array1d<double>(5);
const Dimensions::ActivationFunctions::ArgMax<double> argmax(input);


const auto result =
    argmax.execute(
        Data::DataValue::array1d(
            std::vector<double>{1.0, 5.0, 2.0, 9.0, 3.0}));

ASSERT_EQ(
    result.getRank(),
    0u)
    << "ArgMax must return its result as a scalar.";

EXPECT_EQ(
    result.getData<size_t>()[0],
    3u)
    << "ArgMax must return the index of the largest element.";


}

TEST(ActivationFunctionsArgMaxTest, ReturnsZeroWhenFirstElementIsLargest)
{
const auto input = Dimensions::Requirement::array1d<int>(4);
const Dimensions::ActivationFunctions::ArgMax<int> argmax(input);


const auto result =
    argmax.execute(
        Data::DataValue::array1d(
            std::vector<int>{10, 5, 3, 1}));

EXPECT_EQ(
    result.getData<size_t>()[0],
    0u)
    << "ArgMax must return index zero when the first element is the largest.";


}

TEST(ActivationFunctionsArgMaxTest, ReturnsLastIndexWhenLastElementIsLargest)
{
const auto input = Dimensions::Requirement::array1d<int>(4);
const Dimensions::ActivationFunctions::ArgMax<int> argmax(input);


const auto result =
    argmax.execute(
        Data::DataValue::array1d(
            std::vector<int>{1, 2, 3, 10}));

EXPECT_EQ(
    result.getData<size_t>()[0],
    3u)
    << "ArgMax must return the final index when the last element is the largest.";


}

TEST(ActivationFunctionsArgMaxTest, ReturnsFirstIndexWhenMaximumOccursMultipleTimes)
{
const auto input = Dimensions::Requirement::array1d<int>(5);
const Dimensions::ActivationFunctions::ArgMax<int> argmax(input);


const auto result =
    argmax.execute(
        Data::DataValue::array1d(
            std::vector<int>{5, 9, 9, 2, 9}));

EXPECT_EQ(
    result.getData<size_t>()[0],
    1u)
    << "ArgMax must return the first occurrence when the maximum value appears multiple times.";


}

TEST(ActivationFunctionsArgMaxTest, HandlesNegativeValues)
{
const auto input = Dimensions::Requirement::array1d<int>(4);
const Dimensions::ActivationFunctions::ArgMax<int> argmax(input);


const auto result =
    argmax.execute(
        Data::DataValue::array1d(
            std::vector<int>{-10, -3, -7, -5}));

EXPECT_EQ(
    result.getData<size_t>()[0],
    1u)
    << "ArgMax must correctly identify the largest value when all values are negative.";


}

TEST(ActivationFunctionsArgMaxTest, HandlesAllEqualValues)
{
const auto input = Dimensions::Requirement::array1d<int>(4);
const Dimensions::ActivationFunctions::ArgMax<int> argmax(input);


const auto result =
    argmax.execute(
        Data::DataValue::array1d(
            std::vector<int>{7, 7, 7, 7}));

EXPECT_EQ(
    result.getData<size_t>()[0],
    0u)
    << "ArgMax must return the first index when every element has the same value.";


}

TEST(ActivationFunctionsArgMaxTest, WorksWithFloatingPointValues)
{
const auto input = Dimensions::Requirement::array1d<double>(4);
const Dimensions::ActivationFunctions::ArgMax<double> argmax(input);


const auto result =
    argmax.execute(
        Data::DataValue::array1d(
            std::vector<double>{0.1, 0.9, 0.3, 0.7}));

EXPECT_EQ(
    result.getData<size_t>()[0],
    1u)
    << "ArgMax must work correctly with double-precision floating-point values.";


}

TEST(ActivationFunctionsArgMaxTest, WorksWithFloatValues)
{
const auto input = Dimensions::Requirement::array1d<float>(4);
const Dimensions::ActivationFunctions::ArgMax<float> argmax(input);


const auto result =
    argmax.execute(
        Data::DataValue::array1d(
            std::vector<float>{0.1f, 0.9f, 0.3f, 0.7f}));

EXPECT_EQ(
    result.getData<size_t>()[0],
    1u)
    << "ArgMax must work correctly with single-precision floating-point values.";


}

TEST(ActivationFunctionsArgMaxTest, WorksWithUnsignedIntegerValues)
{
const auto input = Dimensions::Requirement::array1d<uint32_t>(4);
const Dimensions::ActivationFunctions::ArgMax<uint32_t> argmax(input);


const auto result =
    argmax.execute(
        Data::DataValue::array1d(
            std::vector<uint32_t>{1u, 100u, 50u, 25u}));

EXPECT_EQ(
    result.getData<size_t>()[0],
    1u)
    << "ArgMax must work correctly with unsigned integer values.";


}

TEST(ActivationFunctionsArgMaxTest, WorksWithSignedIntegerValues)
{
const auto input = Dimensions::Requirement::array1d<int32_t>(4);
const Dimensions::ActivationFunctions::ArgMax<int32_t> argmax(input);


const auto result =
    argmax.execute(
        Data::DataValue::array1d(
            std::vector<int32_t>{-100, -20, -50, -30}));

EXPECT_EQ(
    result.getData<size_t>()[0],
    1u)
    << "ArgMax must work correctly with signed integer values.";


}

TEST(ActivationFunctionsArgMaxTest, WorksWithTwoDimensionalArray)
{
const auto input = Dimensions::Requirement::array2d<int>(2, 3);
const Dimensions::ActivationFunctions::ArgMax<int> argmax(input);


const auto result =
    argmax.execute(
        Data::DataValue::array2d(
            std::vector<std::vector<int>>{
                {1, 9, 3},
                {4, 5, 2}
            }));

ASSERT_EQ(
    result.getRank(),
    0u)
    << "ArgMax must return a scalar even when the input is two-dimensional.";

EXPECT_EQ(
    result.getData<size_t>()[0],
    1u)
    << "ArgMax must flatten the input storage and return the index of the largest element.";


}

TEST(ActivationFunctionsArgMaxTest, TwoDimensionalArrayUsesFlattenedIndex)
{
const auto input = Dimensions::Requirement::array2d<int>(2, 3);
const Dimensions::ActivationFunctions::ArgMax<int> argmax(input);


const auto result =
    argmax.execute(
        Data::DataValue::array2d(
            std::vector<std::vector<int>>{
                {1, 2, 3},
                {4, 10, 6}
            }));

EXPECT_EQ(
    result.getData<size_t>()[0],
    4u)
    << "ArgMax must return the flattened storage index rather than a row or column index.";


}

TEST(ActivationFunctionsArgMaxTest, OutputDimensionAcceptsActualResult)
{
const auto input = Dimensions::Requirement::array1d<double>(5);
const Dimensions::ActivationFunctions::ArgMax<double> argmax(input);


const auto result =
    argmax.execute(
        Data::DataValue::array1d(
            std::vector<double>{1.0, 4.0, 2.0, 3.0, 0.0}));

EXPECT_TRUE(
    argmax.outputDimension().accepts(result.view()))
    << "The scalar index produced by ArgMax must satisfy its declared output dimension.";


}

TEST(ActivationFunctionsArgMaxTest, OutputDimensionRejectsWrongType)
{
const auto input = Dimensions::Requirement::array1d<double>(3);
const Dimensions::ActivationFunctions::ArgMax<double> argmax(input);


const auto wrongType =
    Data::DataValue::scalar<int>(1);

EXPECT_FALSE(
    argmax.outputDimension().accepts(wrongType.view()))
    << "ArgMax output dimension must reject an index represented by the wrong data type.";


}

TEST(ActivationFunctionsArgMaxTest, InputDimensionRejectsWrongShape)
{
const auto input = Dimensions::Requirement::array1d<double>(4);
const Dimensions::ActivationFunctions::ArgMax<double> argmax(input);


const auto wrongShape =
    Data::DataValue::array1d(
        std::vector<double>{1.0, 2.0, 3.0});

EXPECT_FALSE(
    argmax.inputDimensions()[0].accepts(wrongShape.view()))
    << "ArgMax input dimension must reject an array with the wrong number of elements.";


}

TEST(ActivationFunctionsArgMaxTest, InputDimensionRejectsWrongType)
{
const auto input = Dimensions::Requirement::array1d<double>(4);
const Dimensions::ActivationFunctions::ArgMax<double> argmax(input);


const auto wrongType =
    Data::DataValue::array1d(
        std::vector<float>{1.0f, 2.0f, 3.0f, 4.0f});

EXPECT_FALSE(
    argmax.inputDimensions()[0].accepts(wrongType.view()))
    << "ArgMax input dimension must reject data with the wrong element type.";


}

TEST(ActivationFunctionsArgMaxTest, SupportsPolymorphicFunctionInterface)
{
const auto input = Dimensions::Requirement::array1d<double>(4);


std::unique_ptr<Dimensions::ActivationFunctions::Function> function =
    std::make_unique<Dimensions::ActivationFunctions::ArgMax<double>>(input);

EXPECT_EQ(
    function->name(),
    "ArgMax")
    << "ArgMax must remain usable through the polymorphic Function interface.";

const auto result =
    function->execute(
        Data::DataValue::array1d(
            std::vector<double>{1.0, 5.0, 2.0, 3.0}));

EXPECT_EQ(
    result.getData<size_t>()[0],
    1u)
    << "The polymorphic Function interface must execute ArgMax correctly.";


}

// ============================================================================
// Cross-function / contract tests
// ============================================================================

TEST(ActivationFunctionsTest, TanhAndArgMaxHaveDifferentOutputContracts)
{
const auto tanhInput =
Dimensions::Requirement::array1d<double>(4);


const auto argmaxInput =
    Dimensions::Requirement::array1d<double>(4);

const Dimensions::ActivationFunctions::Tanh<double> tanh(tanhInput);
const Dimensions::ActivationFunctions::ArgMax<double> argmax(argmaxInput);

EXPECT_NE(
    tanh.outputDimension().getDataType(),
    argmax.outputDimension().getDataType())
    << "Tanh and ArgMax must expose different output data types because Tanh returns transformed values while ArgMax returns an index.";


}

TEST(ActivationFunctionsTest, BothFunctionsExposeTheirInputRequirements)
{
const auto input =
Dimensions::Requirement::array1d<double>(4);


const Dimensions::ActivationFunctions::Tanh<double> tanh(input);
const Dimensions::ActivationFunctions::ArgMax<double> argmax(input);

ASSERT_EQ(
    tanh.inputDimensions().size(),
    1u)
    << "Tanh must expose exactly one input requirement.";

ASSERT_EQ(
    argmax.inputDimensions().size(),
    1u)
    << "ArgMax must expose exactly one input requirement.";

EXPECT_EQ(
    tanh.inputDimensions()[0],
    input)
    << "Tanh must preserve its supplied input requirement.";

EXPECT_EQ(
    argmax.inputDimensions()[0],
    input)
    << "ArgMax must preserve its supplied input requirement.";

}
