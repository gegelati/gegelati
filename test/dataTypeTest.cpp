#include <gtest/gtest.h>

#include <array>
#include <string>
#include <vector>

#include "newData/dataType.h"

TEST(DataTypeTest, DefaultConstructorCreatesInvalidDescriptor)
{
    Data::DataType descriptor;

    EXPECT_EQ(descriptor.rank, 0u) << "Default rank should be 0.";
    EXPECT_EQ(descriptor.dimensions[0], 0u) << "Default dimension 0 should be 0.";
    EXPECT_EQ(descriptor.dimensions[1], 0u) << "Default dimension 1 should be 0.";
    EXPECT_EQ(descriptor.elementType, nullptr) << "Default element type should be null.";
    EXPECT_EQ(descriptor.elementSize, 0u) << "Default element size should be 0.";
    EXPECT_EQ(descriptor.sourceRank, 0u) << "Default source rank should be 0.";
    EXPECT_EQ(descriptor.sourceDimensions[0], 0u) << "Default source dimension 0 should be 0.";
    EXPECT_EQ(descriptor.sourceDimensions[1], 0u) << "Default source dimension 1 should be 0.";
    EXPECT_EQ(descriptor.sourceOffset, 0u) << "Default source offset should be 0.";
    EXPECT_EQ(descriptor.totalElements(), 0u) << "Default total elements should be 0.";
    EXPECT_EQ(descriptor.sourceTotalElements(), 0u) << "Default source total elements should be 0.";
    EXPECT_TRUE(descriptor.canFitIn(descriptor, 0)) << "Default descriptor should fit inside itself at index 0.";
}

TEST(DataTypeTest, ScalarFactoryCreatesScalarDescriptor)
{
    const auto descriptor = Data::DataType::scalar<double>();

    EXPECT_EQ(descriptor.rank, 0u) << "Scalar rank should be 0.";
    EXPECT_EQ(descriptor.dimensions[0], 1u) << "Scalar dimension should be {1, 0}.";
    EXPECT_EQ(descriptor.dimensions[1], 0u) << "Scalar second dimension should be 0.";
    ASSERT_NE(descriptor.elementType, nullptr) << "Scalar elementType should not be null.";
    EXPECT_TRUE(*descriptor.elementType == typeid(double)) << "Scalar type should be double.";
    EXPECT_EQ(descriptor.elementSize, sizeof(double)) << "Scalar size should be sizeof(double).";
    EXPECT_EQ(descriptor.sourceRank, 0u) << "Scalar source rank should be 0.";
    EXPECT_EQ(descriptor.sourceDimensions[0], 1u) << "Scalar source size should be 1.";
    EXPECT_EQ(descriptor.sourceDimensions[1], 0u) << "Scalar source second dimension should be 0.";
    EXPECT_EQ(descriptor.sourceOffset, 0u) << "Scalar source offset should be 0.";
    EXPECT_EQ(descriptor.totalElements(), 1u) << "Scalar total elements should be 1.";
    EXPECT_EQ(descriptor.sourceTotalElements(), 1u) << "Scalar source total elements should be 1.";
    EXPECT_TRUE(descriptor.canFitIn(descriptor, 0)) << "descriptor should always fit inside itself at index 0.";
    EXPECT_TRUE(descriptor.canFitIn(Data::DataType::scalar<double>(), 0)) << "Scalar should fit inside itself at offset 0.";
    EXPECT_FALSE(descriptor.canFitIn(Data::DataType::scalar<double>(), 1)) << "Scalar should not start at offset 1 inside a scalar.";
    EXPECT_FALSE(descriptor.canFitIn(Data::DataType::array1d<double>(1), 0)) << "Scalar should not fit an array of the same type.";
    EXPECT_FALSE(descriptor.canFitIn(Data::DataType::array2d<double>(1, 1), 0)) << "Scalar should not fit an array of the same type.";
}

TEST(DataTypeTest, Array1dFactoryCreates1DDescriptor)
{
    const auto descriptor = Data::DataType::array1d<int>(4);

    EXPECT_EQ(descriptor.rank, 1u) << "1D rank should be 1.";
    EXPECT_EQ(descriptor.dimensions[0], 4u) << "1D size should be 4.";
    EXPECT_EQ(descriptor.dimensions[1], 0u) << "1D second dimension should be 0.";
    ASSERT_NE(descriptor.elementType, nullptr) << "1D elementType should not be null.";
    EXPECT_TRUE(*descriptor.elementType == typeid(int)) << "1D type should be int.";
    EXPECT_EQ(descriptor.elementSize, sizeof(int)) << "1D size should be sizeof(int).";
    EXPECT_EQ(descriptor.sourceRank, 1u) << "1D source rank should be 1.";
    EXPECT_EQ(descriptor.sourceDimensions[0], 4u) << "1D source size should be 4.";
    EXPECT_EQ(descriptor.sourceDimensions[1], 0u) << "1D source second dimension should be 0.";
    EXPECT_EQ(descriptor.sourceOffset, 0u) << "1D source offset should be 0.";
    EXPECT_EQ(descriptor.totalElements(), 4u) << "1D total elements should be 4.";
    EXPECT_EQ(descriptor.sourceTotalElements(), 4u) << "1D source total elements should be 4.";

    EXPECT_TRUE(descriptor.canFitIn(descriptor, 0)) << "descriptor should always fit inside itself at index 0.";

    EXPECT_TRUE(descriptor.canFitIn(Data::DataType::array1d<int>(4), 0)) << "An identical 1D descriptor should fit inside itself.";
    EXPECT_TRUE(descriptor.canFitIn(Data::DataType::array1d<int>(2), 1)) << "A valid 1D window should fit at the end of the descriptor.";
    EXPECT_FALSE(descriptor.canFitIn(Data::DataType::array1d<int>(2), 3)) << "A 1D window exceeding the end should fail.";

    EXPECT_TRUE(descriptor.canFitIn(Data::DataType::scalar<double>(), 2)) << "Scalar should fit inside itself at offset 2.";
    EXPECT_FALSE(descriptor.canFitIn(Data::DataType::scalar<double>(), 5)) << "Scalar should not fit inside itself at offset 5.";

    EXPECT_FALSE(descriptor.canFitIn(Data::DataType::array2d<double>(1, 1), 0)) << "Scalar should not fit an array of the same type.";
}

TEST(DataTypeTest, Array2dFactoryCreates2DDescriptor)
{
    const auto descriptor = Data::DataType::array2d<float>(2, 3);

    EXPECT_EQ(descriptor.rank, 2u) << "2D rank should be 2.";
    EXPECT_EQ(descriptor.dimensions[0], 2u) << "2D row count should be 2.";
    EXPECT_EQ(descriptor.dimensions[1], 3u) << "2D column count should be 3.";
    ASSERT_NE(descriptor.elementType, nullptr) << "2D elementType should not be null.";
    EXPECT_TRUE(*descriptor.elementType == typeid(float)) << "2D type should be float.";
    EXPECT_EQ(descriptor.elementSize, sizeof(float)) << "2D size should be sizeof(float).";
    EXPECT_EQ(descriptor.sourceRank, 2u) << "2D source rank should be 2.";
    EXPECT_EQ(descriptor.sourceDimensions[0], 2u) << "2D source rows should be 2.";
    EXPECT_EQ(descriptor.sourceDimensions[1], 3u) << "2D source cols should be 3.";
    EXPECT_EQ(descriptor.sourceOffset, 0u) << "2D source offset should be 0.";
    EXPECT_EQ(descriptor.totalElements(), 6u) << "2D total elements should be 6.";
    EXPECT_EQ(descriptor.sourceTotalElements(), 6u) << "2D source total elements should be 6.";
    EXPECT_TRUE(descriptor.canFitIn(descriptor, 0)) << "descriptor should always fit inside itself at index 0.";
    EXPECT_TRUE(descriptor.canFitIn(Data::DataType::array2d<float>(2, 3), 0)) << "An identical 2D descriptor should fit inside itself.";
    EXPECT_TRUE(descriptor.canFitIn(Data::DataType::array2d<float>(1, 3), 3)) << "A valid 2D window should fit when it starts on a row boundary.";
    EXPECT_FALSE(descriptor.canFitIn(Data::DataType::array2d<float>(2, 2), 2)) << "A 2D window that overflows the right edge should fail.";

    EXPECT_TRUE(descriptor.canFitIn(Data::DataType::scalar<double>(), 5)) << "Scalar should fit inside  at offset 5.";
    EXPECT_FALSE(descriptor.canFitIn(Data::DataType::scalar<double>(), 8)) << "Scalar should not fit inside  at offset 8.";

    EXPECT_TRUE(descriptor.canFitIn(Data::DataType::array1d<double>(2), 4)) << "array should fit inside  at offset 4.";
    EXPECT_FALSE(descriptor.canFitIn(Data::DataType::array1d<double>(2), 7)) << "array should not fit inside at offset 7.";
    EXPECT_FALSE(descriptor.canFitIn(Data::DataType::array1d<double>(4), 2)) << "array should not fit inside  at offset 2.";
}

TEST(DataTypeTest, FromDeductionMatchesFactoryLiterals)
{
    const int scalarValue = 42;
    int values[4] = {1, 2, 3, 4};
    double matrix[2][3] = {{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};

    const auto scalarDescriptor = Data::DataType::from(scalarValue);
    EXPECT_EQ(scalarDescriptor.rank, 0u) << "Scalar from() should create rank 0.";
    EXPECT_TRUE(*scalarDescriptor.elementType == typeid(int)) << "from(scalar) should resolve int.";
    EXPECT_EQ(scalarDescriptor.dimensions[0], 1u) << "Scalar from() dimension should be 1.";

    const auto array1dDescriptor = Data::DataType::from(values);
    EXPECT_EQ(array1dDescriptor.rank, 1u) << "from(array1d) should create rank 1.";
    EXPECT_EQ(array1dDescriptor.dimensions[0], 4u) << "from(array1d) should keep the array size.";
    EXPECT_TRUE(*array1dDescriptor.elementType == typeid(int)) << "from(array1d) should resolve int.";

    const auto array2dDescriptor = Data::DataType::from(matrix);
    EXPECT_EQ(array2dDescriptor.rank, 2u) << "from(array2d) should create rank 2.";
    EXPECT_EQ(array2dDescriptor.dimensions[0], 2u) << "from(array2d) should keep row count.";
    EXPECT_EQ(array2dDescriptor.dimensions[1], 3u) << "from(array2d) should keep column count.";
    EXPECT_TRUE(*array2dDescriptor.elementType == typeid(double)) << "from(array2d) should resolve double.";
}

TEST(DataTypeTest, SubViewPreservesSourceMetadata)
{
    const auto source = Data::DataType::array2d<double>(3, 4);
    const auto sub = Data::DataType::subView(Data::DataType::array2d<double>(2, 3), source, 5);

    EXPECT_EQ(sub.rank, 2u) << "Sub-view rank should match requested rank.";
    EXPECT_EQ(sub.dimensions[0], 2u) << "Sub-view row count should be 2.";
    EXPECT_EQ(sub.dimensions[1], 3u) << "Sub-view col count should be 3.";
    EXPECT_TRUE(*sub.elementType == typeid(double)) << "Sub-view element type should stay double.";
    EXPECT_EQ(sub.sourceRank, source.sourceRank) << "Sub-view should keep the original source rank.";
    EXPECT_EQ(sub.sourceDimensions[0], 3u) << "Sub-view should keep original source rows.";
    EXPECT_EQ(sub.sourceDimensions[1], 4u) << "Sub-view should keep original source cols.";
    EXPECT_EQ(sub.sourceOffset, 5u) << "Sub-view should keep the correct source offset.";
    EXPECT_EQ(sub.totalElements(), 6u) << "Sub-view total elements should be rows * cols.";
    EXPECT_EQ(sub.sourceTotalElements(), 12u) << "Source total elements should remain the original buffer size.";
}

TEST(DataTypeTest, CanFitInHandlesScalar1DAnd2DCases)
{
    const auto scalar = Data::DataType::scalar<int>();
    const auto oneD = Data::DataType::array1d<int>(5);
    const auto twoD = Data::DataType::array2d<int>(3, 4);

    EXPECT_TRUE(scalar.canFitIn(Data::DataType::scalar<int>(), 0)) << "Scalar into scalar should fit.";
    EXPECT_FALSE(scalar.canFitIn(Data::DataType::array1d<int>(1), 0)) << "Scalar should not accept a 1D shape.";
    EXPECT_FALSE(scalar.canFitIn(Data::DataType::array2d<int>(1, 1), 0)) << "Scalar should not accept a 2D shape.";

    EXPECT_TRUE(oneD.canFitIn(Data::DataType::scalar<int>(), 0)) << "A 1D view can contain a scalar at its first element.";
    EXPECT_TRUE(oneD.canFitIn(Data::DataType::array1d<int>(2), 1)) << "A valid 1D sub-view should fit.";
    EXPECT_FALSE(oneD.canFitIn(Data::DataType::array1d<int>(3), 3)) << "Out-of-range 1D sub-view should fail.";
    EXPECT_FALSE(oneD.canFitIn(Data::DataType::array2d<int>(1, 2), 1)) << "1D descriptor should reject a 2D requested shape.";

    EXPECT_TRUE(twoD.canFitIn(Data::DataType::scalar<int>(), 0)) << "A 2D view can contain a scalar at the origin.";
    EXPECT_TRUE(twoD.canFitIn(Data::DataType::array1d<int>(2), 1)) << "A valid 1D window in a 2D descriptor should fit.";
    EXPECT_TRUE(twoD.canFitIn(Data::DataType::array2d<int>(1, 2), 1)) << "A valid 2D window should fit.";
    EXPECT_FALSE(twoD.canFitIn(Data::DataType::array2d<int>(2, 3), 2)) << "An out-of-bounds 2D window should fail.";
}

TEST(DataTypeTest, CanFitInUsesLinearOffsetRelativeToCurrentDescriptor)
{
    const auto source = Data::DataType::array2d<int>(3, 4);
    const auto window = Data::DataType::subView(Data::DataType::array2d<int>(2, 2), source, 5);

    EXPECT_TRUE(window.canFitIn(Data::DataType::array2d<int>(2, 2), 0)) << "The exact same 2x2 window should fit inside itself.";
    EXPECT_FALSE(window.canFitIn(Data::DataType::array2d<int>(2, 2), 1)) << "A shifted 2x2 window should not fit if it leaves its own descriptor.";
    EXPECT_TRUE(source.canFitIn(Data::DataType::array2d<int>(2, 2), 5)) << "A source descriptor can fit a valid 2D window at offset 5.";
    EXPECT_FALSE(source.canFitIn(Data::DataType::array2d<int>(2, 2), 10)) << "An offset past the valid bounds should fail.";
}

TEST(DataTypeTest, EqualityAndSourceAwareComparisonBehaveAsExpected)
{
    const auto a = Data::DataType::array2d<int>(2, 3);
    const auto b = Data::DataType::array2d<int>(2, 3);
    const auto c = Data::DataType::subView(Data::DataType::array2d<int>(2, 3), Data::DataType::array2d<int>(4, 5), 7);

    EXPECT_TRUE(a == b) << "Same logical shape and type should compare equal.";
    EXPECT_FALSE(a != b) << "Inequality should be false for identical descriptors.";
    EXPECT_TRUE(a.equalsWithSource(a)) << "A descriptor should compare equal to itself including source context.";
    EXPECT_FALSE(a.equalsWithSource(c)) << "Different source provenance should make the source-aware comparison fail.";
    EXPECT_TRUE(a == c) << "Equality intentionally ignores source context.";
}

TEST(DataTypeTest, ToStringContainsShapeAndSourceMetadata)
{
    const auto descriptor = Data::DataType::subView(
        Data::DataType::array2d<double>(2, 2),
        Data::DataType::array2d<double>(3, 4),
        5);

    const std::string text = descriptor.toString();

    EXPECT_NE(text.find("DataType{"), std::string::npos) << "String should contain the type header.";
    EXPECT_NE(text.find("rank=2"), std::string::npos) << "Rank should be included in the string.";
    EXPECT_NE(text.find("dimensions=[2, 2]"), std::string::npos) << "Requested dimensions should be included.";
    EXPECT_NE(text.find("sourceRank=2"), std::string::npos) << "Source rank should be included.";
    EXPECT_NE(text.find("sourceDimensions=[3, 4]"), std::string::npos) << "Source dimensions should be included.";
    EXPECT_NE(text.find("sourceOffset=5"), std::string::npos) << "Source offset should be included.";
}

