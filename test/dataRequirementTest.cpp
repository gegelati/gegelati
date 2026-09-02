#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <sstream>
#include <vector>

#include "newData/dataRequirement.h"
#include "newData/dataValue.h"
#include "newData/numericRange.h"

TEST(DataRequirementTest, UnconstrainedRequirementChecksShapeAndType)
{
    const auto requirement = Data::DataRequirement::array1d<int>(3);
    const auto matching = Data::DataValue::array1d(std::vector<int>{1, 2, 3});
    const auto wrongSize = Data::DataValue::array1d(std::vector<int>{1, 2});
    const auto wrongType = Data::DataValue::array1d(std::vector<double>{1.0, 2.0, 3.0});

    EXPECT_TRUE(requirement.accepts(matching.view()));
    EXPECT_FALSE(requirement.accepts(wrongSize.view()));
    EXPECT_FALSE(requirement.accepts(wrongType.view()));
    EXPECT_EQ(requirement.getDataType(), matching.getType());
}

TEST(DataRequirementTest, UnconstrainedFactoriesSupportEveryShape)
{
    const auto scalar = Data::DataRequirement::scalar<int>();
    const auto array1d = Data::DataRequirement::array1d<double>(3);
    const auto array2d = Data::DataRequirement::array2d<float>(2, 4);

    EXPECT_EQ(scalar.getDataType(), Data::DataType::scalar<int>());
    EXPECT_EQ(array1d.getDataType(), Data::DataType::array1d<double>(3));
    EXPECT_EQ(array2d.getDataType(), Data::DataType::array2d<float>(2, 4));
}

TEST(DataRequirementTest, EqualityComparesDataTypeAndConstraint)
{
    const auto unconstrained = Data::DataRequirement::array1d<int>(3);
    const auto sameUnconstrained = Data::DataRequirement::array1d<int>(3);
    const auto differentShape = Data::DataRequirement::array1d<int>(4);
    const auto range = Data::DataRequirement::scalar<int>(
        Data::NumericRange<int>::between(0, 5));
    const auto sameRange = Data::DataRequirement::scalar<int>(
        Data::NumericRange<int>::between(0, 5));
    const auto differentRange = Data::DataRequirement::scalar<int>(
        Data::NumericRange<int>::between(0, 6));
    const auto otherConstraint = Data::DataRequirement::scalar<double>(
        Data::NumericRange<double>::between(0.0, 5.0));

    EXPECT_TRUE(unconstrained == sameUnconstrained);
    EXPECT_FALSE(unconstrained != sameUnconstrained);
    EXPECT_FALSE(unconstrained == differentShape);
    EXPECT_TRUE(range == sameRange);
    EXPECT_FALSE(range != sameRange);
    EXPECT_FALSE(range == differentRange);
    EXPECT_FALSE(range == otherConstraint);
    EXPECT_FALSE(range == Data::DataRequirement::scalar<int>());
}

TEST(DataRequirementTest, NumericRangePreservesTheExactBoundType)
{
    const uint64_t maximum = std::numeric_limits<uint64_t>::max();
    const auto requirement = Data::DataRequirement::scalar<uint64_t>(
        Data::NumericRange<uint64_t>::between(maximum - 1, maximum));

    const auto matching = Data::DataValue::scalar(maximum);
    const auto below = Data::DataValue::scalar(maximum - 2);

    EXPECT_TRUE(requirement.accepts(matching.view()));
    EXPECT_FALSE(requirement.accepts(below.view()));

    const auto* range = dynamic_cast<const Data::NumericRange<uint64_t>*>(&requirement.getConstraint());
    ASSERT_NE(range, nullptr);
    EXPECT_EQ(*range->minimum, maximum - 1);
    EXPECT_EQ(*range->maximum, maximum);
}

TEST(DataRequirementTest, ConstructorClonesConstraint)
{
    const auto sourceConstraint = Data::NumericRange<int>::between(0, 5);
    const Data::DataRequirement requirement(
        Data::DataType::scalar<int>(), sourceConstraint);

    EXPECT_NE(&requirement.getConstraint(), &sourceConstraint);
    EXPECT_TRUE(requirement.accepts(Data::DataValue::scalar(3).view()));
    EXPECT_FALSE(requirement.accepts(Data::DataValue::scalar(6).view()));
}

TEST(DataRequirementTest, NumericRangeAppliesToEveryScalarArrayElement)
{
    const auto requirement = Data::DataRequirement::array1d<double>(
        3, Data::NumericRange<double>::between(-0.4, 0.4));

    EXPECT_TRUE(requirement.accepts(
        Data::DataValue::array1d(std::vector<double>{-0.4, 0.0, 0.4}).view()));
    EXPECT_FALSE(requirement.accepts(
        Data::DataValue::array1d(std::vector<double>{-0.4, 0.5, 0.4}).view()));
}

TEST(DataRequirementTest, NumericRangeAppliesToEveryTwoDimensionalElement)
{
    const auto requirement = Data::DataRequirement::array2d<int>(
        2, 2, Data::NumericRange<int>::between(0, 5));

    EXPECT_TRUE(requirement.accepts(
        Data::DataValue::array2d(std::vector<std::vector<int>>{{0, 5}, {1, 4}}).view()));
    EXPECT_FALSE(requirement.accepts(
        Data::DataValue::array2d(std::vector<std::vector<int>>{{0, 6}, {1, 4}}).view()));
}

TEST(DataRequirementTest, NumericRangeCanRejectFullMatrixButAcceptValidSubView)
{
    const Data::DataRequirement fullMatrixRequirement(
        Data::DataType::array2d<int>(4, 4),
        Data::NumericRange<int>::between(0, 9));
    const Data::DataRequirement subMatrixRequirement(
        Data::DataType::array2d<int>(3, 3),
        Data::NumericRange<int>::between(0, 9));
    const auto matrix = Data::DataValue::array2d(
        std::vector<std::vector<int>>{
            {10, 10, 10, 10},
            {10, 1, 2, 3},
            {10, 4, 5, 6},
            {10, 7, 8, 9}});

    EXPECT_FALSE(fullMatrixRequirement.accepts(matrix.view()));

    const Data::DataView subView = matrix.view().getSubView(
        Data::DataType::array2d<int>(3, 3), 5);
    EXPECT_TRUE(subMatrixRequirement.accepts(subView));
}

TEST(DataRequirementTest, SupportsOneSidedAndUnboundedRanges)
{
    const auto atLeast = Data::DataRequirement::scalar<int>(
        Data::NumericRange<int>::atLeast(0));
    const auto unbounded = Data::DataRequirement::scalar<double>(
        Data::NumericRange<double>::unbounded());

    EXPECT_TRUE(atLeast.accepts(Data::DataValue::scalar(0).view()));
    EXPECT_FALSE(atLeast.accepts(Data::DataValue::scalar(-1).view()));
    EXPECT_TRUE(unbounded.accepts(Data::DataValue::scalar(-1000.0).view()));
    EXPECT_TRUE(unbounded.accepts(Data::DataValue::scalar(1000.0).view()));
}

TEST(DataRequirementTest, ToStringDescribesTypeAndConstraint)
{
    const Data::DataRequirement unconstrained(
        Data::DataType::array1d<int>(3));
    const auto numeric = Data::DataRequirement::scalar<double>(
        Data::NumericRange<double>::between(-0.4, 0.4));

    std::string strUnconstrained = "DataRequirement{\n\tDataType{rank=1, dimensions=[3], elementType=i, elementSize=4, sourceRank=1, sourceDimensions=[3], sourceOffset=0},\n\tRequirement: unconstrained\n}";
    std::string strNumeric = "DataRequirement{\n\tDataType{rank=0, dimensions=[], elementType=d, elementSize=8, sourceRank=0, sourceDimensions=[], sourceOffset=0},\n\tRequirement: Numeric Range: [-0.4, 0.4]\n}";
    
    EXPECT_EQ(unconstrained.toString(), strUnconstrained);
    EXPECT_EQ(numeric.toString(), strNumeric);

    std::ostringstream output;
    output << numeric;
    EXPECT_EQ(output.str(), numeric.toString());
}

TEST(DataRequirementTest, HandlesInvalidBoundsAndMismatchedConstraintTypes)
{
    const Data::DataRequirement mismatchedRequirement(
        Data::DataType::scalar<int>(),
        Data::NumericRange<double>::between(0.0, 1.0));

    EXPECT_FALSE(mismatchedRequirement.accepts(Data::DataValue::scalar(1).view()));
    EXPECT_THROW(
        Data::NumericRange<int>::between(2, 1),
        std::invalid_argument);

    const Data::DataRequirement requirement(
        Data::DataType::scalar<int>(),
        Data::NumericRange<int>::between(0, 5));
    EXPECT_FALSE(requirement.accepts(Data::DataValue::scalar(1.0).view()));
}
