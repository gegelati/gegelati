#include <gtest/gtest.h>

#include "evaluation/predictionProblem.h"
#include "evaluation/reinforcementProblem.h"
#include "metrics/rewardsMetric.h"
#include "metrics/inputMetric.h"
#include "metrics/outputMetric.h"

#include "learn/stickGameWithOpponent.h"
#include "learn/fakeRepresentation.h"
#include "learn/dataSetExamples.h"


TEST(ReinforcementProblemTest, Constructor)
{
    StickGameWithOpponent environment;
    Evaluation::ReinforcementProblem* problem;

    ASSERT_NO_THROW(problem = new Evaluation::ReinforcementProblem(environment)) << "Construction failed";

    ASSERT_NO_THROW(delete problem) << "Destruction failed";
}

TEST(ReinforcementProblemTest, getEnvironment)
{
    StickGameWithOpponent environment;
    Evaluation::ReinforcementProblem problem(environment);

    ASSERT_EQ(&problem.getEnvironment(), &environment) << "Returned environment should be the environment used to construct the problem";
}

TEST(ReinforcementProblemTest, dimensions)
{
    StickGameWithOpponent environment;
    Evaluation::ReinforcementProblem problem(environment);

    ASSERT_NO_THROW(problem.getInputDimensions()) << "Getting input dimensions failed";
    ASSERT_NO_THROW(problem.getOutputDimension()) << "Getting output dimension failed";

    ASSERT_EQ(problem.getInputDimensions(), environment.getInputDimensions()) << "Input dimensions should match the environment";
    ASSERT_EQ(problem.getOutputDimension(), environment.getOutputDimension()) << "Output dimension should match the environment";
}

TEST(ReinforcementProblemTest, maxHash) 
{
    StickGameWithOpponent environment;
    Evaluation::ReinforcementProblem problem(environment);

    ASSERT_EQ(problem.maxHash(), UINT64_MAX) << "Max hash should be super high!";
}

TEST(ReinforcementProblemTest, extractNoMetric)
{
    StickGameWithOpponent environment;
    Evaluation::ReinforcementProblem problem(environment);

    Representations::FakeRepresentation rep(problem.getInputDimensions(), problem.getOutputDimension());
    std::unique_ptr<GraphBased::Genotype> genotype = GraphBased::Genotype::singleNodeGenotype(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0}));
    Individual indiv(rep, std::move(genotype));
    ASSERT_TRUE(indiv.isValid()) << "Individual should be valid";

    auto metricTemplate = std::make_shared<Metrics::MetricMapTemplate>();
    Metrics::MetricMap map(metricTemplate);

    ASSERT_NO_THROW(problem.extractMetrics(indiv, map, {0, 1})) << "Metric extraction without metrics should not throw";
}

TEST(ReinforcementProblemTest, extractOneMetric)
{
    StickGameWithOpponent environment;
    Evaluation::ReinforcementProblem problem(environment);

    Representations::FakeRepresentation rep(problem.getInputDimensions(), problem.getOutputDimension());
    std::unique_ptr<GraphBased::Genotype> genotype = GraphBased::Genotype::singleNodeGenotype(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0}));
    Individual indiv(rep, std::move(genotype));

    ASSERT_TRUE(indiv.isValid()) << "Individual should be valid";

    auto metricTemplate = std::make_shared<Metrics::MetricMapTemplate>(std::make_unique<Metrics::RewardsMetric>());
    Metrics::MetricMap map(metricTemplate);

    ASSERT_NO_THROW(problem.extractMetrics(indiv, map, {0, 1})) << "Reward metric extraction failed";

    std::map<uint64_t, const Metrics::RewardsMetric*> metrics = map.getMetricValues<Metrics::RewardsMetric>(Metrics::RewardsMetric::staticHash());

    ASSERT_EQ(metrics.size(), 2) << "Should have two reward metrics";

    ASSERT_TRUE(metrics.find(0) != metrics.end()) << "Should have hash 0";
    ASSERT_TRUE(metrics.find(1) != metrics.end()) << "Should have hash 1";

    ASSERT_NO_THROW(metrics.at(0)->getScore()) << "Getting reward metric at hash 0 failed";
    ASSERT_NO_THROW(metrics.at(1)->getScore()) << "Getting reward metric at hash 1 failed";
}


TEST(ReinforcementProblemTest, extractMultiMetric)
{
    StickGameWithOpponent environment;
    Evaluation::ReinforcementProblem problem(environment);

    Representations::FakeRepresentation rep(problem.getInputDimensions(), problem.getOutputDimension());
    std::unique_ptr<GraphBased::Genotype> genotype = GraphBased::Genotype::singleNodeGenotype(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0}));
    Individual indiv(rep, std::move(genotype));

    ASSERT_TRUE(indiv.isValid()) << "Individual should be valid";

    auto metricTemplate = std::make_shared<Metrics::MetricMapTemplate>();
    metricTemplate->addRequiredMetric(std::make_unique<Metrics::RewardsMetric>());
    metricTemplate->addRequiredMetric(std::make_unique<Metrics::InputMetric>());
    metricTemplate->addRequiredMetric(std::make_unique<Metrics::OutputMetric>());
    Metrics::MetricMap map(metricTemplate);

    ASSERT_NO_THROW(problem.extractMetrics(indiv, map, {0, 1})) << "Reward metric extraction failed";

    std::map<uint64_t, const Metrics::RewardsMetric*> metrics = map.getMetricValues<Metrics::RewardsMetric>(Metrics::RewardsMetric::staticHash());

    ASSERT_EQ(metrics.size(), 2) << "Should have two reward metrics";

    ASSERT_TRUE(metrics.find(0) != metrics.end()) << "Should have hash 0";
    ASSERT_TRUE(metrics.find(1) != metrics.end()) << "Should have hash 1";

    ASSERT_NO_THROW(metrics.at(0)->getScore()) << "Getting reward metric at hash 0 failed";
    ASSERT_NO_THROW(metrics.at(1)->getScore()) << "Getting reward metric at hash 1 failed";
    
    std::map<uint64_t, const Metrics::OutputMetric*> outputMetrics = map.getMetricValues<Metrics::OutputMetric>(Metrics::OutputMetric::staticHash());
    ASSERT_EQ(outputMetrics.size(), 2) << "Should have two metrics with two hash";

    ASSERT_TRUE(outputMetrics.find(0) != outputMetrics.end()) << "Should have hash 0";
    ASSERT_TRUE(outputMetrics.find(1) != outputMetrics.end()) << "Should have hash 1";

    ASSERT_GT(outputMetrics.at(0)->getOutputs().size(), 1) << "Size of output should be more than 1 for reinforcement";
    ASSERT_GT(outputMetrics.at(1)->getOutputs().size(), 1) << "Size of output should be more than 1 for reinforcement";
    
    ASSERT_EQ(outputMetrics.at(0)->getOutputs().at(0), Data::DataValue::scalar<double>(1)) << "Values should be equal";
    ASSERT_EQ(outputMetrics.at(1)->getOutputs().at(0), Data::DataValue::scalar<double>(1)) << "Values should be equal";
    
    std::map<uint64_t, const Metrics::InputMetric*> inputMetrics = map.getMetricValues<Metrics::InputMetric>(Metrics::InputMetric::staticHash());
    ASSERT_EQ(inputMetrics.size(), 2) << "Should have two metrics with two hash";

    ASSERT_TRUE(inputMetrics.find(0) != inputMetrics.end()) << "Should have hash 0";
    ASSERT_TRUE(inputMetrics.find(1) != inputMetrics.end()) << "Should have hash 1";

    ASSERT_GT(inputMetrics.at(0)->getInputs().size(), 1) << "Size of input should be more than 1 for reinforcement";
    ASSERT_GT(inputMetrics.at(1)->getInputs().size(), 1) << "Size of input should be more than 1 for reinforcement";

    ASSERT_EQ(inputMetrics.at(0)->getInputs().at(0).size(), 2) << "Dataset should have only one source";
    ASSERT_EQ(inputMetrics.at(1)->getInputs().at(0).size(), 2) << "Dataset should have only one source";

    ASSERT_EQ(inputMetrics.at(0)->getInputs().at(0).at(0), Data::DataValue::array1d<int[3]>({1, 2, 3})) << "Values should be equal";
    ASSERT_EQ(inputMetrics.at(0)->getInputs().at(0).at(1), Data::DataValue::scalar<double>(21.0)) << "Values should be equal";
    ASSERT_EQ(inputMetrics.at(1)->getInputs().at(0).at(0), Data::DataValue::array1d<int[3]>({1, 2, 3})) << "Values should be equal";
    ASSERT_EQ(inputMetrics.at(1)->getInputs().at(0).at(1), Data::DataValue::scalar<double>(21.0)) << "Values should be equal";
}

TEST(RewardsMetricTest, Constructor)
{
    Metrics::RewardsMetric* metric;

    ASSERT_NO_THROW(metric = new Metrics::RewardsMetric()) << "Construction failed";
    ASSERT_NO_THROW(metric->cloneEmptyPtr()) << "Cloning empty metric failed";

    ASSERT_NO_THROW(delete metric) << "Destruction failed";
}

TEST(RewardsMetricTest, staticHash)
{
    Metrics::RewardsMetric metric;
    ASSERT_EQ(metric.hash(), Metrics::RewardsMetric::staticHash()) << "Hash should be equal to staticHash";
}

TEST(RewardsMetricTest, extract)
{
    StickGameWithOpponent environment;
    Evaluation::ReinforcementProblem problem(environment);
    Metrics::RewardsMetric metric;


    Representations::FakeRepresentation rep(problem.getInputDimensions(), problem.getOutputDimension());
    std::unique_ptr<GraphBased::Genotype> genotype = GraphBased::Genotype::singleNodeGenotype(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0}));
    Individual indiv(rep, std::move(genotype));

    environment.reset();

    double expectedReward = environment.getLastReward();

    ASSERT_NO_THROW(metric.extractBeforeExecution(0, indiv, environment.getDataSources(), problem)) << "Extracting the reward before execution failed";
    ASSERT_EQ(metric.getScore(), expectedReward) << "Score should contain the environment reward";

    ASSERT_NO_THROW(metric.extractBeforeExecution(0, indiv, environment.getDataSources(), problem)) << "Extracting the reward before execution failed";
    ASSERT_EQ(metric.getScore(), expectedReward * 2) << "Score should contain the environment reward";

    
    ASSERT_NO_THROW(metric.extractionEnd(0, indiv, problem)) << "Extracting the reward at the end of the episode failed";
    ASSERT_EQ(metric.getScore(), expectedReward * 3) << "Score should contain the environment reward";

    
    Evaluation::DataSet dataSet = createSmallDataSet();
    Evaluation::PredictionProblem wrongProblem(dataSet);

    ASSERT_THROW(metric.extractBeforeExecution(0, indiv, wrongProblem.getDataSet().getInputsAt(0), wrongProblem), std::runtime_error) << "Extracting before execution should fail with a non-reinforcement problem";

    ASSERT_THROW(metric.extractionEnd(0, indiv, wrongProblem), std::runtime_error) << "Extraction at the end should fail with a non-reinforcement problem";
}

TEST(RewardsMetricTest, toString)
{
    Metrics::RewardsMetric metric;

    ASSERT_NO_THROW(metric.toString()) << "Converting metric to string failed";
}