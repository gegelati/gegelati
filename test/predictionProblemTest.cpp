#include <gtest/gtest.h>

#include "evaluation/predictionProblem.h"
#include "evaluation/reinforcementProblem.h"
#include "metrics/predictionMetric.h"
#include "metrics/inputMetric.h"
#include "metrics/outputMetric.h"

#include "learn/stickGameWithOpponent.h"
#include "learn/fakeRepresentation.h"
#include "learn/dataSetExamples.h"

TEST(PredictionProblemTest, Constructor) 
{
    Evaluation::PredictionProblem* p;
    Evaluation::DataSet dataSet = createSmallDataSet();

    ASSERT_NO_THROW(p = new Evaluation::PredictionProblem(dataSet)) << "Construction failed";

    ASSERT_NO_THROW(delete p) << "Destruction failed";
}

TEST(PredictionProblemTest, dimensions) 
{
    Evaluation::DataSet dataSet = createSmallDataSet();
    Evaluation::PredictionProblem problem(dataSet);

    ASSERT_NO_THROW(problem.getInputDimensions())  << "Getting dimension failed";
    ASSERT_NO_THROW(problem.getOutputDimension())  << "Getting dimension failed";

    ASSERT_EQ(problem.getInputDimensions().at(0), dataSet.getInputDimensions().at(0));
    ASSERT_EQ(problem.getInputDimensions().size(), 1);
    ASSERT_EQ(problem.getOutputDimension(), dataSet.getOutputDimension());

    ASSERT_NO_THROW(problem.summary());
}

TEST(PredictionProblemTest, seed) 
{
    Evaluation::DataSet dataSet = createSmallDataSet();
    Evaluation::PredictionProblem problem(dataSet);
    ASSERT_EQ(problem.getProblemSeed(), 0);

    Evaluation::PredictionProblem problem1(dataSet, 3);
    ASSERT_EQ(problem1.getProblemSeed(), 3);
}
TEST(PredictionMetricTest, Constructor) 
{
    Metrics::PredictionMetric* metric;

    ASSERT_NO_THROW(metric = new Metrics::PredictionMetric()) << "Construction failed";
    ASSERT_NO_THROW(metric->cloneEmptyPtr()) << "Construction failed";

    ASSERT_NO_THROW(delete metric) << "Destruction failed";
}

TEST(PredictionMetricTest, staticHash) 
{
    Metrics::PredictionMetric metric;
    ASSERT_EQ(metric.hash(), Metrics::PredictionMetric::staticHash())<< "Hash should be equals";
}

TEST(PredictionMetricTest, extractAfterExecution) {
    Evaluation::DataSet dataSet = createSmallDataSet();
    Evaluation::PredictionProblem problem(dataSet);

    Representations::FakeRepresentation rep(problem.getInputDimensions(), problem.getOutputDimension());
    std::unique_ptr<GraphBased::Genotype> genotype = GraphBased::Genotype::singleNodeGenotype(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0}));
    Individual indiv(rep, std::move(genotype));

    Metrics::PredictionMetric metric;
    ASSERT_NO_THROW(metric.extractAfterExecution(0, indiv, Data::DataValue::scalar<int>(0), problem)) << "Fail to extract metric";
    ASSERT_EQ(metric.getValue(), 1) << "Accuracy should be 1";
    ASSERT_EQ(metric.getPairs().size(), 1);

    ASSERT_NO_THROW(metric.extractAfterExecution(0, indiv, Data::DataValue::scalar<int>(1), problem)) << "Fail to extract metric";
    ASSERT_EQ(metric.getValue(), 0.5) << "Accuracy should be 1/2";

    ASSERT_NO_THROW(metric.extractAfterExecution(0, indiv, Data::DataValue::scalar<double>(0), problem)) << "Fail to extract metric";
    ASSERT_NEAR(metric.getValue(), 1.0 / 3.0, 1e-9) << "Accuracy should be 1/3";

    ASSERT_NO_THROW(metric.extractAfterExecution(1, indiv, Data::DataValue::scalar<int>(1), problem)) << "Fail to extract metric";
    ASSERT_EQ(metric.getValue(), 0.5) << "Accuracy should be 2/4";

    ASSERT_EQ(metric.getPairs().size(), 4);
    ASSERT_EQ(metric.getPairs().at(1).first, Data::DataValue::scalar<int>(1));
    ASSERT_EQ(metric.getPairs().at(1).second, Data::DataValue::scalar<int>(0));

    ASSERT_NO_THROW(metric.toString()) << "Fail to print metric";

    std::shared_ptr<Evaluation::ReinforcementEnvironment> le = std::make_shared<StickGameWithOpponent>();
    Evaluation::ReinforcementProblem fakeProblem(le);
    ASSERT_THROW(metric.extractAfterExecution(0, indiv, Data::DataValue::scalar<int>(1), fakeProblem), std::runtime_error) << "Should fail with wrong problem";

}


TEST(PredictionProblemTest, extractNoMetric) {
    Evaluation::DataSet dataSet = createSmallDataSet();
    Evaluation::PredictionProblem problem(dataSet);

    Representations::FakeRepresentation rep(problem.getInputDimensions(), problem.getOutputDimension());
    rep.output = Data::DataValue::scalar<int>(0);
    std::unique_ptr<GraphBased::Genotype> genotype = GraphBased::Genotype::singleNodeGenotype(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0}));
    Individual indiv(rep, std::move(genotype));
    ASSERT_TRUE(indiv.isValid()) << "Should be valid";

    auto metricTemplate = std::make_shared<Metrics::MetricMapTemplate>();
    Metrics::MetricMap map(metricTemplate);

    ASSERT_NO_THROW(problem.extractMetrics(indiv, map, {0, 1}));
}

TEST(PredictionProblemTest, extractOneMetric) {

    Evaluation::DataSet dataSet = createSmallDataSet();
    Evaluation::PredictionProblem problem(dataSet);

    Representations::FakeRepresentation rep(problem.getInputDimensions(), problem.getOutputDimension());
    rep.output = Data::DataValue::scalar<int>(0);
    std::unique_ptr<GraphBased::Genotype> genotype = GraphBased::Genotype::singleNodeGenotype(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0}));
    Individual indiv(rep, std::move(genotype));
    ASSERT_TRUE(indiv.isValid()) << "Should be valid";

    auto metricTemplate = std::make_shared<Metrics::MetricMapTemplate>(std::make_unique<Metrics::PredictionMetric>());
    Metrics::MetricMap map(metricTemplate);

    ASSERT_NO_THROW(problem.extractMetrics(indiv, map, {0, 1}));
    
    std::map<uint64_t, const Metrics::PredictionMetric*> metrics = map.getMetricValues<Metrics::PredictionMetric>(Metrics::PredictionMetric::staticHash());
    ASSERT_EQ(metrics.size(), 1) << "Should have a single metric for the whole loop";

    const Metrics::PredictionMetric* metric = metrics.begin()->second;
    ASSERT_EQ(metric->getValue(), 0.5) << "Accuracy should be 0.5";
    ASSERT_EQ(metric->getPairs().size(), 2);
}

TEST(PredictionProblemTest, extractMultiMetrics) {

    Evaluation::DataSet dataSet = createSmallDataSet();

    Evaluation::PredictionProblem problem(dataSet);

    Representations::FakeRepresentation rep(problem.getInputDimensions(), problem.getOutputDimension());
    rep.output = Data::DataValue::scalar<int>(0);
    std::unique_ptr<GraphBased::Genotype> genotype = GraphBased::Genotype::singleNodeGenotype(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0}));
    Individual indiv(rep, std::move(genotype));
    ASSERT_TRUE(indiv.isValid()) << "Should be balid";

    auto metricTemplate = std::make_shared<Metrics::MetricMapTemplate>();
    metricTemplate->addRequiredMetric(std::make_unique<Metrics::PredictionMetric>());
    metricTemplate->addRequiredMetric(std::make_unique<Metrics::InputMetric>());
    metricTemplate->addRequiredMetric(std::make_unique<Metrics::OutputMetric>());
    Metrics::MetricMap map(metricTemplate);

    ASSERT_NO_THROW(problem.extractMetrics(indiv, map, {0, 1}));
    
    std::map<uint64_t, const Metrics::PredictionMetric*> predictionMetrics = map.getMetricValues<Metrics::PredictionMetric>(Metrics::PredictionMetric::staticHash());
    ASSERT_EQ(predictionMetrics.size(), 1) << "Should have a single metric for the whole loop";
    ASSERT_EQ(predictionMetrics.begin()->second->getValue(), 0.5) << "Accuracy should be 0.5";
    ASSERT_EQ(predictionMetrics.begin()->second->getPairs().size(), 2);
    
    std::map<uint64_t, const Metrics::OutputMetric*> outputMetrics = map.getMetricValues<Metrics::OutputMetric>(Metrics::OutputMetric::staticHash());
    ASSERT_EQ(outputMetrics.size(), 1) << "Should have a single metric for the whole loop";
    const auto& outputs = outputMetrics.begin()->second->getOutputs();
    ASSERT_EQ(outputs.size(), 2) << "One output per hash";
    ASSERT_EQ(outputs.at(0), Data::DataValue::scalar<int>(0)) << "Values should be equal";
    ASSERT_EQ(outputs.at(1), Data::DataValue::scalar<int>(0)) << "Values should be equal";
    
    std::map<uint64_t, const Metrics::InputMetric*> inputMetrics = map.getMetricValues<Metrics::InputMetric>(Metrics::InputMetric::staticHash());
    ASSERT_EQ(inputMetrics.size(), 1) << "Should have a single metric for the whole loop";
    const auto& inputs = inputMetrics.begin()->second->getInputs();
    ASSERT_EQ(inputs.size(), 2) << "One input set per hash";
    ASSERT_EQ(inputs.at(0)->size(), 1) << "Dataset should have only one source";
    ASSERT_EQ(inputs.at(0)->at(0), Data::DataValue::array1d<int[2]>({1, 2})) << "Values should be equal";
    ASSERT_EQ(inputs.at(1)->at(0), Data::DataValue::array1d<int[2]>({3, 4})) << "Values should be equal";
}

