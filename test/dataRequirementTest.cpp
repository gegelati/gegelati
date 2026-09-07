#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <sstream>
#include <vector>

#include "dimensions/requirement.h"
#include "data/dataValue.h"
#include "dimensions/numericRange.h"
#include "dimensions/dimensionFlow.h"

TEST(DataRequirementTest, UnconstrainedRequirementChecksShapeAndType)
{
    const auto requirement = Dimensions::Requirement::array1d<int>(3);
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
    const auto scalar = Dimensions::Requirement::scalar<int>();
    const auto array1d = Dimensions::Requirement::array1d<double>(3);
    const auto array2d = Dimensions::Requirement::array2d<float>(2, 4);

    EXPECT_EQ(scalar.getDataType(), Data::DataType::scalar<int>());
    EXPECT_EQ(array1d.getDataType(), Data::DataType::array1d<double>(3));
    EXPECT_EQ(array2d.getDataType(), Data::DataType::array2d<float>(2, 4));
}

TEST(DataRequirementTest, EqualityComparesDataTypeAndConstraint)
{
    const auto unconstrained = Dimensions::Requirement::array1d<int>(3);
    const auto sameUnconstrained = Dimensions::Requirement::array1d<int>(3);
    const auto differentShape = Dimensions::Requirement::array1d<int>(4);
    const auto range = Dimensions::Requirement::scalar<int>(
        Dimensions::NumericRange<int>::between(0, 5));
    const auto sameRange = Dimensions::Requirement::scalar<int>(
        Dimensions::NumericRange<int>::between(0, 5));
    const auto differentRange = Dimensions::Requirement::scalar<int>(
        Dimensions::NumericRange<int>::between(0, 6));
    const auto otherConstraint = Dimensions::Requirement::scalar<double>(
        Dimensions::NumericRange<double>::between(0.0, 5.0));

    EXPECT_TRUE(unconstrained == sameUnconstrained);
    EXPECT_FALSE(unconstrained != sameUnconstrained);
    EXPECT_FALSE(unconstrained == differentShape);
    EXPECT_TRUE(range == sameRange);
    EXPECT_FALSE(range != sameRange);
    EXPECT_FALSE(range == differentRange);
    EXPECT_FALSE(range == otherConstraint);
    EXPECT_FALSE(range == Dimensions::Requirement::scalar<int>());
}

TEST(DataRequirementTest, NumericRangePreservesTheExactBoundType)
{
    const uint64_t maximum = std::numeric_limits<uint64_t>::max();
    const auto requirement = Dimensions::Requirement::scalar<uint64_t>(
        Dimensions::NumericRange<uint64_t>::between(maximum - 1, maximum));

    const auto matching = Data::DataValue::scalar(maximum);
    const auto below = Data::DataValue::scalar(maximum - 2);

    EXPECT_TRUE(requirement.accepts(matching.view()));
    EXPECT_FALSE(requirement.accepts(below.view()));

    const auto* range = dynamic_cast<const Dimensions::NumericRange<uint64_t>*>(&requirement.getConstraint());
    ASSERT_NE(range, nullptr);
    EXPECT_EQ(*range->minimum, maximum - 1);
    EXPECT_EQ(*range->maximum, maximum);
}

TEST(DataRequirementTest, ConstructorClonesConstraint)
{
    const auto sourceConstraint = Dimensions::NumericRange<int>::between(0, 5);
    const Dimensions::Requirement requirement(
        Data::DataType::scalar<int>(), sourceConstraint);

    EXPECT_NE(&requirement.getConstraint(), &sourceConstraint);
    EXPECT_TRUE(requirement.accepts(Data::DataValue::scalar(3).view()));
    EXPECT_FALSE(requirement.accepts(Data::DataValue::scalar(6).view()));
}

TEST(DataRequirementTest, NumericRangeAppliesToEveryScalarArrayElement)
{
    const auto requirement = Dimensions::Requirement::array1d<double>(
        3, Dimensions::NumericRange<double>::between(-0.4, 0.4));

    EXPECT_TRUE(requirement.accepts(
        Data::DataValue::array1d(std::vector<double>{-0.4, 0.0, 0.4}).view()));
    EXPECT_FALSE(requirement.accepts(
        Data::DataValue::array1d(std::vector<double>{-0.4, 0.5, 0.4}).view()));
}

TEST(DataRequirementTest, NumericRangeAppliesToEveryTwoDimensionalElement)
{
    const auto requirement = Dimensions::Requirement::array2d<int>(
        2, 2, Dimensions::NumericRange<int>::between(0, 5));

    EXPECT_TRUE(requirement.accepts(
        Data::DataValue::array2d(std::vector<std::vector<int>>{{0, 5}, {1, 4}}).view()));
    EXPECT_FALSE(requirement.accepts(
        Data::DataValue::array2d(std::vector<std::vector<int>>{{0, 6}, {1, 4}}).view()));
}

TEST(DataRequirementTest, NumericRangeCanRejectFullMatrixButAcceptValidSubView)
{
    const Dimensions::Requirement fullMatrixRequirement(
        Data::DataType::array2d<int>(4, 4),
        Dimensions::NumericRange<int>::between(0, 9));
    const Dimensions::Requirement subMatrixRequirement(
        Data::DataType::array2d<int>(3, 3),
        Dimensions::NumericRange<int>::between(0, 9));
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
    const auto atLeast = Dimensions::Requirement::scalar<int>(
        Dimensions::NumericRange<int>::atLeast(0));
    const auto unbounded = Dimensions::Requirement::scalar<double>(
        Dimensions::NumericRange<double>::unbounded());

    EXPECT_TRUE(atLeast.accepts(Data::DataValue::scalar(0).view()));
    EXPECT_FALSE(atLeast.accepts(Data::DataValue::scalar(-1).view()));
    EXPECT_TRUE(unbounded.accepts(Data::DataValue::scalar(-1000.0).view()));
    EXPECT_TRUE(unbounded.accepts(Data::DataValue::scalar(1000.0).view()));
}

TEST(DataRequirementTest, CompatibilityUsesRangeContainment)
{
    const auto producer = Dimensions::Requirement::scalar<double>(
        Dimensions::NumericRange<double>::between(-0.5, 0.5));
    const auto consumer = Dimensions::Requirement::scalar<double>(
        Dimensions::NumericRange<double>::between(-1.0, 1.0));
    const auto narrowerConsumer = Dimensions::Requirement::scalar<double>(
        Dimensions::NumericRange<double>::between(-0.25, 0.25));
    const auto unconstrained = Dimensions::Requirement::scalar<double>(Dimensions::UnconstrainedData());

    EXPECT_TRUE(producer.isCompatibleWith(consumer));
    EXPECT_FALSE(producer.isCompatibleWith(narrowerConsumer));
    EXPECT_TRUE(producer.isCompatibleWith(unconstrained));
    EXPECT_FALSE(unconstrained.isCompatibleWith(consumer));
}

TEST(DataRequirementTest, DimensionFlowChecksFinalProducerOutput)
{
    const auto input = Dimensions::Requirement::array1d<double>(4);
    const auto producerOutput = Dimensions::Requirement::scalar<double>(
        Dimensions::NumericRange<double>::between(-1.0, 1.0));
    const auto consumerOutput = Dimensions::Requirement::scalar<double>(
        Dimensions::NumericRange<double>::between(-2.0, 2.0));
    const auto incompatibleOutput = Dimensions::Requirement::scalar<double>(
        Dimensions::NumericRange<double>::between(-0.5, 0.5));

    Dimensions::DimensionFlow producer({input});
    producer.addLayer("producer", {input}, producerOutput);
    Dimensions::DimensionFlow consumer({input});
    consumer.addLayer("consumer", {input}, consumerOutput);
    Dimensions::DimensionFlow incompatible({input});
    incompatible.addLayer("incompatible", {input}, incompatibleOutput);

    EXPECT_TRUE(producer.isCompatibleWith(consumer.getOutputDimension()));
    EXPECT_FALSE(producer.isCompatibleWith(incompatible.getOutputDimension()));
}

TEST(DataRequirementTest, ToStringDescribesTypeAndConstraint)
{
    const Dimensions::Requirement unconstrained(
        Data::DataType::array1d<int>(3));
    const auto numeric = Dimensions::Requirement::scalar<double>(
        Dimensions::NumericRange<double>::between(-0.4, 0.4));

    std::string strUnconstrained = "Requirement{\n\tDataType{rank=1, dimensions=[3], elementType=int, elementSize=4, sourceRank=1, sourceDimensions=[3], sourceOffset=0},\n\tRequirement: unconstrained\n}";
    std::string strNumeric = "Requirement{\n\tDataType{rank=0, dimensions=[], elementType=double, elementSize=8, sourceRank=0, sourceDimensions=[], sourceOffset=0},\n\tRequirement: Numeric Range: [-0.4, 0.4]\n}";
    
    EXPECT_EQ(unconstrained.toString(), strUnconstrained);
    EXPECT_EQ(numeric.toString(), strNumeric);

    std::ostringstream output;
    output << numeric;
    EXPECT_EQ(output.str(), numeric.toString());
}

TEST(DataRequirementTest, HandlesInvalidBoundsAndMismatchedConstraintTypes)
{
    const Dimensions::Requirement mismatchedRequirement(
        Data::DataType::scalar<int>(),
        Dimensions::NumericRange<double>::between(0.0, 1.0));

    EXPECT_FALSE(mismatchedRequirement.accepts(Data::DataValue::scalar(1).view()));
    EXPECT_THROW(
        Dimensions::NumericRange<int>::between(2, 1),
        std::invalid_argument);

    const Dimensions::Requirement requirement(
        Data::DataType::scalar<int>(),
        Dimensions::NumericRange<int>::between(0, 5));
    EXPECT_FALSE(requirement.accepts(Data::DataValue::scalar(1.0).view()));
}
