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

TEST(PredictionProblemTest, maxHash) 
{
    Evaluation::DataSet dataSet = createSmallDataSet();

    Evaluation::PredictionProblem problem(dataSet);
    ASSERT_EQ(problem.maxHash(), problem.getDataSet().size()) << "Max hash should be 2";
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
    ASSERT_EQ(metric.getValue(), 1) << "Score should be successful (1)";

    ASSERT_NO_THROW(metric.extractAfterExecution(0, indiv, Data::DataValue::scalar<int>(1), problem)) << "Fail to extract metric";
    ASSERT_EQ(metric.getValue(), 0) << "Score should be unsuccessful (0)";

    ASSERT_NO_THROW(metric.extractAfterExecution(0, indiv, Data::DataValue::scalar<double>(0), problem)) << "Fail to extract metric";
    ASSERT_EQ(metric.getValue(), 0) << "Score should be unsuccessful (0)";

    ASSERT_NO_THROW(metric.extractAfterExecution(1, indiv, Data::DataValue::scalar<int>(0), problem)) << "Fail to extract metric";
    ASSERT_EQ(metric.getValue(), 0) << "Score should be unsuccessful (0)";

    ASSERT_NO_THROW(metric.extractAfterExecution(1, indiv, Data::DataValue::scalar<int>(1), problem)) << "Fail to extract metric";
    ASSERT_EQ(metric.getValue(), 1) << "Score should be successful (1)";

    
    ASSERT_NO_THROW(metric.toString()) << "Fail to print metric";

    StickGameWithOpponent le;
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
    ASSERT_EQ(metrics.size(), 2) << "Should have two metrics with two hash";

    ASSERT_TRUE(metrics.find(0) != metrics.end()) << "Should have hash 0";
    ASSERT_TRUE(metrics.find(1) != metrics.end()) << "Should have hash 1";

    ASSERT_EQ(metrics.at(0)->getValue(), 1.0) << "Score of metric at 0 should be 1";
    ASSERT_EQ(metrics.at(1)->getValue(), 0.0) << "Score of metric at 1 should be 0";
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
    ASSERT_EQ(predictionMetrics.size(), 2) << "Should have two metrics with two hash";

    ASSERT_TRUE(predictionMetrics.find(0) != predictionMetrics.end()) << "Should have hash 0";
    ASSERT_TRUE(predictionMetrics.find(1) != predictionMetrics.end()) << "Should have hash 1";

    ASSERT_EQ(predictionMetrics.at(0)->getValue(), 1.0) << "Score of metric at 0 should be 1";
    ASSERT_EQ(predictionMetrics.at(1)->getValue(), 0.0) << "Score of metric at 1 should be 0";
    
    std::map<uint64_t, const Metrics::OutputMetric*> outputMetrics = map.getMetricValues<Metrics::OutputMetric>(Metrics::OutputMetric::staticHash());
    ASSERT_EQ(outputMetrics.size(), 2) << "Should have two metrics with two hash";

    ASSERT_TRUE(outputMetrics.find(0) != outputMetrics.end()) << "Should have hash 0";
    ASSERT_TRUE(outputMetrics.find(1) != outputMetrics.end()) << "Should have hash 1";

    ASSERT_EQ(outputMetrics.at(0)->getOutputs().size(), 1) << "Size of output should be 1 for classification";
    ASSERT_EQ(outputMetrics.at(1)->getOutputs().size(), 1) << "Size of output should be 1 for classification";
    
    ASSERT_EQ(outputMetrics.at(0)->getOutputs().at(0), Data::DataValue::scalar<int>(0)) << "Values should be equal";
    ASSERT_EQ(outputMetrics.at(1)->getOutputs().at(0), Data::DataValue::scalar<int>(0)) << "Values should be equal";
    
    std::map<uint64_t, const Metrics::InputMetric*> inputMetrics = map.getMetricValues<Metrics::InputMetric>(Metrics::InputMetric::staticHash());
    ASSERT_EQ(inputMetrics.size(), 2) << "Should have two metrics with two hash";

    ASSERT_TRUE(inputMetrics.find(0) != inputMetrics.end()) << "Should have hash 0";
    ASSERT_TRUE(inputMetrics.find(1) != inputMetrics.end()) << "Should have hash 1";

    ASSERT_EQ(inputMetrics.at(0)->getInputs().size(), 1) << "Size of input should be 1 for classification";
    ASSERT_EQ(inputMetrics.at(1)->getInputs().size(), 1) << "Size of input should be 1 for classification";

    ASSERT_EQ(inputMetrics.at(0)->getInputs().at(0).size(), 1) << "Dataset should have only one source";
    ASSERT_EQ(inputMetrics.at(1)->getInputs().at(0).size(), 1) << "Dataset should have only one source";

    ASSERT_EQ(inputMetrics.at(0)->getInputs().at(0).at(0), Data::DataValue::array1d<int[2]>({1, 2})) << "Values should be equal";
    ASSERT_EQ(inputMetrics.at(1)->getInputs().at(0).at(0), Data::DataValue::array1d<int[2]>({3, 4})) << "Values should be equal";
}

