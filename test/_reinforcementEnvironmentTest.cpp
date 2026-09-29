#include <gtest/gtest.h>

#include "evaluation/reinforcementEnvironment.h"


class FakeReinforcementEnvironment : public Evaluation::ReinforcementEnvironment
{
  public:
    FakeReinforcementEnvironment() : ReinforcementEnvironment({Dimensions::Requirement::array1d<int>(3)}, Dimensions::Requirement::scalar<double>(), 42) {};

    void reset(size_t seed = 0) override
    {
        this->dataSources.clear();
        this->dataSources.push_back(Data::DataValue::array1d<int[3]>({static_cast<int>(seed), 2, 3}));
    }

    void setWrongSource(const Data::DataValue& value) {
        this->dataSources.push_back(value.clone());
    }

    double getLastReward() const override
    {
        return 1.5;
    }

    bool isTerminal() const override
    {
        return false;
    }
};


TEST(ReinforcementEnvironmentTest, Constructor)
{
    Evaluation::ReinforcementEnvironment* environment;

    ASSERT_NO_THROW(environment = new FakeReinforcementEnvironment()) << "Construction failed";

    ASSERT_NO_THROW(delete environment) << "Destruction failed";
}


TEST(ReinforcementEnvironmentTest, Clonable)
{
    FakeReinforcementEnvironment environment;

    ASSERT_FALSE(environment.isCopyable()) << "Default behavior of isCopyable should be false";
    ASSERT_EQ(environment.cloneUniquePtr(), nullptr) << "Default behavior of cloneUniquePtr should return nullptr";
}


TEST(ReinforcementEnvironmentTest, getDimensions)
{
    FakeReinforcementEnvironment environment;

    ASSERT_EQ(environment.getInputDimensions().size(), 1) << "Environment should have one input dimension";
    ASSERT_EQ(environment.getInputDimensions().at(0), Dimensions::Requirement::array1d<int>(3)) << "Input dimension should match the expected requirement";
    ASSERT_EQ(environment.getOutputDimension(), Dimensions::Requirement::scalar<double>()) << "Output dimension should match the expected requirement";
}


TEST(ReinforcementEnvironmentTest, getDataSources)
{
    FakeReinforcementEnvironment environment;
    ASSERT_THROW(environment.getDataSources(), std::runtime_error) << "Should throw before reset (and setting datasources)";

    ASSERT_NO_THROW(environment.reset()) << "Reset should initialize the data sources";
    ASSERT_NO_THROW(environment.getDataSources()) << "Getting valid data sources should not throw";
    ASSERT_EQ(environment.getDataSources().size(), 1) << "Environment should contain one data source";
    ASSERT_EQ(environment.getDataSources().at(0), Data::DataValue::array1d<int[3]>({0, 2, 3})) << "Data source should contain the expected values";

    environment.setWrongSource(Data::DataValue::scalar<float>(0.2f));
    ASSERT_THROW(environment.getDataSources(), std::runtime_error) << "Should throw with wrong sources";
}


TEST(ReinforcementEnvironmentTest, getMaxSteps)
{
    FakeReinforcementEnvironment environment;

    ASSERT_EQ(environment.getMaxSteps(), 42) << "Maximum number of steps should match the value provided to the constructor";
}


TEST(ReinforcementEnvironmentTest, doAction)
{
    FakeReinforcementEnvironment environment;

    ASSERT_NO_THROW(environment.doAction(Data::DataValue::scalar<double>(1.0))) << "A valid action should not throw";
    ASSERT_THROW(environment.doAction(Data::DataValue::scalar<int>(1)), std::runtime_error) << "An action with an invalid type should throw";
    ASSERT_THROW(environment.doAction(Data::DataValue::array1d<int[3]>({1, 2, 3})), std::runtime_error) << "An action with an invalid requirement should throw";
}


TEST(ReinforcementEnvironmentTest, reset)
{
    FakeReinforcementEnvironment environment;

    ASSERT_NO_THROW(environment.reset()) << "Reset should not throw";
    ASSERT_EQ(environment.getDataSources().at(0), Data::DataValue::array1d<int[3]>({0, 2, 3})) << "Reset with default seed should initialize the expected data source";

    ASSERT_NO_THROW(environment.reset(10)) << "Reset with a seed should not throw";
    ASSERT_EQ(environment.getDataSources().at(0), Data::DataValue::array1d<int[3]>({10, 2, 3})) << "Reset should use the provided seed";
}


TEST(ReinforcementEnvironmentTest, getLastReward)
{
    FakeReinforcementEnvironment environment;

    ASSERT_EQ(environment.getLastReward(), 1.5) << "Last reward should match the expected value";
}


TEST(ReinforcementEnvironmentTest, isTerminal)
{
    FakeReinforcementEnvironment environment;

    ASSERT_FALSE(environment.isTerminal()) << "Fake environment should not be terminal";
}