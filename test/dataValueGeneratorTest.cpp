#include <gtest/gtest.h>

#include <functional>
#include <string>
#include <vector>

#include "dimensions/dataValueGenerator.h"


TEST(DataValueGeneratorTest, SamplesValuesWithinRange)
{
    Dimensions::NumericUniformGenerator<int> generator(10, 20);
    RNG::RNG rng;

    for (size_t i = 0; i < 100; ++i) {
        Data::DataValue value = generator.sample(rng);

        ASSERT_EQ(value.getElementType(), typeid(int))
            << "Generated value should have type int.";

        const int sampledValue = value.getScalar<int>();

        EXPECT_GE(sampledValue, 10)
            << "Generated value should not be lower than the minimum.";

        EXPECT_LE(sampledValue, 20)
            << "Generated value should not be greater than the maximum.";
    }
}

TEST(DataValueGeneratorTest, SupportsDifferentNumericTypes)
{
    Dimensions::NumericUniformGenerator<double> generator(1.5, 3.5);
    RNG::RNG rng;

    Data::DataValue value = generator.sample(rng);

    ASSERT_EQ(value.getElementType(), typeid(double))
        << "Generated value should have type double.";

    const double sampledValue = value.getScalar<double>();

    EXPECT_GE(sampledValue, 1.5)
        << "Generated double should not be lower than the minimum.";

    EXPECT_LE(sampledValue, 3.5)
        << "Generated double should not be greater than the maximum.";
}

TEST(DataValueGeneratorTest, SamplesOnlyValuesFromList)
{
    const std::string first = "first";
    const std::string second = "second";
    const std::string third = "third";

    std::vector<std::string> values = {
        first,
        second,
        third
    };

    Dimensions::ListUniformGenerator<std::string> generator(values);
    RNG::RNG rng;

    for (size_t i = 0; i < 100; ++i) {
        Data::DataValue value = generator.sample(rng);

        ASSERT_EQ(value.getElementType(), typeid(std::string))
            << "Generated value should have type std::string.";

        const std::string& sampledValue = value.getScalar<std::string>();

        EXPECT_TRUE(
            sampledValue == first ||
            sampledValue == second ||
            sampledValue == third)
            << "Generated value should be one of the configured values.";
    }

    values.clear();
    ASSERT_THROW(generator.sample(rng), std::runtime_error) << "should fail to sampled empty list";
}

TEST(DataValueGeneratorTest, CanSampleFromSingleValueList)
{
    const int expectedValue = 42;

    const std::vector<std::shared_ptr<const int>> values = {
        std::make_shared<const int>(expectedValue)
    };

    Dimensions::ListUniformGenerator<std::shared_ptr<const int>> generator(values);
    RNG::RNG rng;

    for (size_t i = 0; i < 10; ++i) {
        Data::DataValue value = generator.sample(rng);

        ASSERT_EQ(value.getElementType(), typeid(std::shared_ptr<const int>))
            << "Generated value should have type int.";

        EXPECT_EQ(*value.getScalar<std::shared_ptr<const int>>(), expectedValue)
            << "A list containing one value should always generate that value.";
    }
}

TEST(DataValueGeneratorTest, GeneratorsCanBeUsedThroughBaseInterface)
{
    Dimensions::NumericUniformGenerator<int> generator(1, 10);
    Dimensions::DataValueGenerator& baseGenerator = generator;
    RNG::RNG rng;

    Data::DataValue value = baseGenerator.sample(rng);

    ASSERT_EQ(value.getElementType(), typeid(int))
        << "Sampling through the base interface should preserve the generated value type.";

    const int sampledValue = value.getScalar<int>();

    EXPECT_GE(sampledValue, 1)
        << "Value generated through the base interface should respect the minimum.";

    EXPECT_LE(sampledValue, 10)
        << "Value generated through the base interface should respect the maximum.";
}

TEST(DataValueGeneratorTest, Rejects)
{
    auto gen = Dimensions::NumericUniformGenerator<int>(1, 10);


    EXPECT_THROW(
        Dimensions::MultiGenerator({gen}, {1.0, 2.0}),
        std::runtime_error)
        << "MultiGenerator should reject a different number of generators and weights.";


    EXPECT_THROW(
        Dimensions::MultiGenerator({gen}, {-1.0}),
        std::runtime_error)
        << "MultiGenerator should reject negative weights.";
}

TEST(DataValueGeneratorTest, AddGeneratorExtendsAvailableGenerators)
{
    auto gen = Dimensions::NumericUniformGenerator<int>(1, 10);


    Dimensions::MultiGenerator multiGenerator({gen}, {1.0});

    multiGenerator.addGenerator(
        Dimensions::NumericUniformGenerator<int>(42, 42), 1.0);

    ASSERT_THROW(multiGenerator.addGenerator(Dimensions::NumericUniformGenerator<int>(42, 42), -1.0), std::runtime_error) << "MultiGenerator should reject negative weights.";

    RNG::RNG rng;

    bool generatedFirstValue = false;
    bool generatedSecondValue = false;

    for (size_t i = 0; i < 100; ++i) {
        const Data::DataValue value = multiGenerator.sample(rng);

        ASSERT_EQ(value.getElementType(), typeid(int))
            << "MultiGenerator should preserve the generated value type.";

        const int sampledValue = value.getScalar<int>();

        if (sampledValue <= 10) {
            generatedFirstValue = true;
        } else if (sampledValue == 42) {
            generatedSecondValue = true;
        } else {
            FAIL() << "MultiGenerator generated a value from an unconfigured generator.";
        }
    }

    EXPECT_TRUE(generatedFirstValue)
        << "MultiGenerator should be able to select the original generator.";

    EXPECT_TRUE(generatedSecondValue)
        << "MultiGenerator should be able to select a generator added with addGenerator.";
}

TEST(DataValueGeneratorTest, ZeroWeightGeneratorIsNeverSelected)
{
    Dimensions::MultiGenerator multiGenerator;
    multiGenerator.addGenerator(Dimensions::NumericUniformGenerator<int>(1, 1), 1.0);
    multiGenerator.addGenerator(Dimensions::NumericUniformGenerator<int>(42, 42), 0.0);

    RNG::RNG rng;

    for (size_t i = 0; i < 100; ++i) {
        const Data::DataValue value = multiGenerator.sample(rng);

        ASSERT_EQ(value.getElementType(), typeid(int))
            << "MultiGenerator should preserve the generated value type.";

        EXPECT_EQ(value.getScalar<int>(), 1)
            << "A generator with zero weight should never be selected.";
    }
}
