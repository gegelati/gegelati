
#include <gtest/gtest.h>

#include "evaluation/predictionProblem.h"
#include "metrics/metricMap.h"
#include "metrics/metric.h"
#include "metrics/inputMetric.h"
#include "metrics/outputMetric.h"

#include "metrics/scalarMetric.h"
#include "metrics/vectorMetric.h"

#include "learn/fakeRepresentation.h"
#include "learn/dataSetExamples.h"

class FakeScalarMetric: public Metrics::ScalarMetric {
    public:
    FakeScalarMetric(double score = 0) {this->values[0] = score;};
    std::unique_ptr<Metric> cloneEmptyPtr() const override {return std::make_unique<FakeScalarMetric>(this->values[0]);};
    std::string toString(std::string prefix = "") const override {return "";};
};

class FakeVectorMetric: public Metrics::VectorMetric {
    public:
    FakeVectorMetric(std::vector<double> values = {0.0, 0.0}): Metrics::VectorMetric(2) {this->values = values;};
    std::unique_ptr<Metric> cloneEmptyPtr() const override {return std::make_unique<FakeVectorMetric>(this->values);};
    std::string toString(std::string prefix = "") const override {return "";};
};

TEST(MetricTest, Constructor) 
{
    Metrics::Metric* metric1;
    Metrics::Metric* metric2;
    FakeScalarMetric* metric3;
    FakeVectorMetric* metric4;

    ASSERT_NO_THROW(metric1 = new Metrics::InputMetric()) << "Construction failed";
    ASSERT_NO_THROW(metric1->cloneEmptyPtr()) << "Cloning failed";

    ASSERT_NO_THROW(metric2 = new Metrics::OutputMetric()) << "Construction failed";
    ASSERT_NO_THROW(metric2->cloneEmptyPtr()) << "Cloning failed";

    ASSERT_NO_THROW(metric3 = new FakeScalarMetric(2.1)) << "Construction failed";
    ASSERT_NO_THROW(metric3->cloneEmptyPtr()) << "Cloning failed";
    ASSERT_NO_THROW(metric3->getRange()) << "For coverage";
    ASSERT_NO_THROW(metric3->getValue()) << "For coverage";


    ASSERT_NO_THROW(metric4 = new FakeVectorMetric()) << "Construction failed";
    ASSERT_NO_THROW(metric4->cloneEmptyPtr()) << "Cloning failed";
    ASSERT_NO_THROW(metric4->getRanges()) << "For coverage";
    ASSERT_NO_THROW(metric4->getValues()) << "For coverage";

    ASSERT_NO_THROW(delete metric1) << "Destruction failed";
    ASSERT_NO_THROW(delete metric2) << "Destruction failed";
    ASSERT_NO_THROW(delete metric3) << "Destruction failed";
    ASSERT_NO_THROW(delete metric4) << "Destruction failed";
}

TEST(MetricTest, staticHash) 
{
    Metrics::InputMetric metric1;
    ASSERT_EQ(metric1.hash(), Metrics::InputMetric::staticHash())<< "Hash should be equals";

    Metrics::OutputMetric metric2;
    ASSERT_EQ(metric2.hash(), Metrics::OutputMetric::staticHash())<< "Hash should be equals";

    ASSERT_NE(metric1.hash(), metric2.hash()) << "Different metric should not have the same hash";

    Metrics::OutputMetric metric3;
    ASSERT_EQ(metric3.hash(), metric2.hash()) << "Different instance of same metric should have the same hash";
}


TEST(MetricTest, extractAfterExecution) {

    Evaluation::DataSet dataSet = createSmallDataSet();
    Evaluation::PredictionProblem problem(dataSet);

    Representations::FakeRepresentation rep(problem.getInputDimensions(), problem.getOutputDimension());
    std::unique_ptr<GraphBased::Genotype> genotype = GraphBased::Genotype::singleNodeGenotype(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0}));
    Individual indiv(rep, std::move(genotype));


    Metrics::InputMetric metric1;
    ASSERT_NO_THROW(metric1.extractBeforeExecution(0, indiv, dataSet.getInputsAt(0), problem)) << "Fail to extract metric";
    ASSERT_EQ(metric1.getInputs().size(), 1);
    ASSERT_EQ(metric1.getInputs().at(0).size(), 1) << "Dataset should have only one source";
    ASSERT_EQ(metric1.getInputs().at(0).at(0), Data::DataValue::array1d<int[2]>({1, 2})) << "Values should be equal";
    
    ASSERT_NO_THROW(metric1.extractBeforeExecution(0, indiv, dataSet.getInputsAt(0), problem)) << "Fail to extract metric";
    ASSERT_NO_THROW(metric1.extractBeforeExecution(0, indiv, dataSet.getInputsAt(1), problem)) << "Fail to extract metric";

    ASSERT_EQ(metric1.getInputs().size(), 3);
    ASSERT_EQ(metric1.getInputs().at(1).size(), 1) << "Dataset should have only one source";
    ASSERT_EQ(metric1.getInputs().at(2).size(), 1) << "Dataset should have only one source";
    ASSERT_EQ(metric1.getInputs().at(1).at(0), Data::DataValue::array1d<int[2]>({1, 2})) << "Values should be equal";
    ASSERT_EQ(metric1.getInputs().at(2).at(0), Data::DataValue::array1d<int[2]>({3, 4})) << "Values should be equal";


    Metrics::OutputMetric metric2;
    ASSERT_NO_THROW(metric2.extractAfterExecution(0, indiv, dataSet.getOutputAt(0), problem)) << "Fail to extract metric";
    ASSERT_EQ(metric2.getOutputs().size(), 1);
    ASSERT_EQ(metric2.getOutputs().at(0), Data::DataValue::scalar<int>(0)) << "Values should be equal";
    
    ASSERT_NO_THROW(metric2.extractAfterExecution(0, indiv, dataSet.getOutputAt(0), problem)) << "Fail to extract metric";
    ASSERT_NO_THROW(metric2.extractAfterExecution(0, indiv, dataSet.getOutputAt(1), problem)) << "Fail to extract metric";

    ASSERT_EQ(metric2.getOutputs().size(), 3);
    ASSERT_EQ(metric2.getOutputs().at(1), Data::DataValue::scalar<int>(0)) << "Values should be equal";
    ASSERT_EQ(metric2.getOutputs().at(2), Data::DataValue::scalar<int>(1)) << "Values should be equal";

    ASSERT_NO_THROW(metric1.extractBeforeExecution(0, indiv, dataSet.getInputsAt(0), problem)) << "For coverage and check empty methods don't throw";
    ASSERT_NO_THROW(metric1.extractAfterExecution(0, indiv, dataSet.getOutputAt(0), problem)) << "For coverage and check empty methods don't throw";
    ASSERT_NO_THROW(metric1.extractionEnd(0, indiv, problem)) << "For coverage and check empty methods don't throw";

    ASSERT_NO_THROW(metric2.extractBeforeExecution(0, indiv, dataSet.getInputsAt(0), problem)) << "For coverage and check empty methods don't throw";
    ASSERT_NO_THROW(metric2.extractAfterExecution(0, indiv, dataSet.getOutputAt(0), problem)) << "For coverage and check empty methods don't throw";
    ASSERT_NO_THROW(metric2.extractionEnd(0, indiv, problem)) << "For coverage and check empty methods don't throw";

    ASSERT_NO_THROW(metric1.toString()) << "Fail to print the metric";
    ASSERT_NO_THROW(metric2.toString()) << "Fail to print the metric";

}

TEST(MetricMapTest, ConstructorTemplate) 
{
    Metrics::MetricMapTemplate* mapTemplate1;
    Metrics::MetricMapTemplate* mapTemplate2;
    Metrics::MetricMapTemplate* mapTemplate3;

    ASSERT_NO_THROW(mapTemplate1 = new Metrics::MetricMapTemplate()) << "Construction failed";

    std::unique_ptr<Metrics::Metric> metric1 = std::make_unique<Metrics::InputMetric>();
    ASSERT_NO_THROW(mapTemplate2 = new Metrics::MetricMapTemplate(std::move(metric1))) << "Construction failed";

    std::vector<std::unique_ptr<Metrics::Metric>> metrics;
    metrics.push_back(std::make_unique<Metrics::InputMetric>());
    metrics.push_back(std::make_unique<Metrics::OutputMetric>());
    ASSERT_NO_THROW(mapTemplate3 = new Metrics::MetricMapTemplate(metrics)) << "Construction failed";

    ASSERT_NO_THROW(delete mapTemplate1) << "Destruction failed";
    ASSERT_NO_THROW(delete mapTemplate2) << "Destruction failed";
    ASSERT_NO_THROW(delete mapTemplate3) << "Destruction failed";
}

TEST(MetricMapTest, addMetricTemplate) 
{
    Metrics::MetricMapTemplate mapTemplate;

    ASSERT_EQ(mapTemplate.size(), 0) << "Size should be 0";

    ASSERT_NO_THROW(mapTemplate.addRequiredMetric(std::make_unique<Metrics::InputMetric>())) << "Fail to add metric";

    ASSERT_EQ(mapTemplate.size(), 1) << "Size should be 1";
    ASSERT_NO_THROW(mapTemplate.addRequiredMetric(std::make_unique<Metrics::InputMetric>())) << "Fail to add metric";
    ASSERT_EQ(mapTemplate.size(), 1) << "Size should still be 1";

    ASSERT_TRUE(mapTemplate.hasMetricHash(Metrics::InputMetric::staticHash())) << "Should have hash";
    ASSERT_FALSE(mapTemplate.hasMetricHash(Metrics::OutputMetric::staticHash())) << "Should not have output hash";
    
    ASSERT_NO_THROW(mapTemplate.addRequiredMetric(std::make_unique<Metrics::OutputMetric>())) << "Fail to add metric";
    ASSERT_EQ(mapTemplate.size(), 2) << "Size should now be 2";

    std::set<uint64_t> hashes = {Metrics::InputMetric::staticHash(), Metrics::OutputMetric::staticHash()};
    ASSERT_EQ(mapTemplate.getMetricHash(), hashes) << "Hashes should be the same";
}


TEST(MetricMapTest, Constructor)
{
    std::shared_ptr<const Metrics::MetricMapTemplate> mapTemplate = std::make_shared<Metrics::MetricMapTemplate>();
    Metrics::MetricMap* map;

    ASSERT_NO_THROW(map = new Metrics::MetricMap(mapTemplate)) << "Construction failed";
    map->getTemplate(); // For coverage
    ASSERT_NO_THROW(delete map) << "Destruction failed";
}


TEST(MetricMapTest, extract)
{
    auto mapTemplate = std::make_shared<Metrics::MetricMapTemplate>();
    mapTemplate->addRequiredMetric(std::make_unique<Metrics::InputMetric>());
    mapTemplate->addRequiredMetric(std::make_unique<Metrics::OutputMetric>());
    Metrics::MetricMap map(mapTemplate);

    Evaluation::DataSet dataSet = createSmallDataSet();
    Evaluation::PredictionProblem problem(dataSet);
    Representations::FakeRepresentation rep(problem.getInputDimensions(), problem.getOutputDimension());
    std::unique_ptr<GraphBased::Genotype> genotype = GraphBased::Genotype::singleNodeGenotype(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0}));
    Individual indiv(rep, std::move(genotype));

    ASSERT_NO_THROW(map.extractBeforeExecution(0, indiv, dataSet.getInputsAt(0), problem)) << "Failed to extract inputs for first evaluation";
    ASSERT_NO_THROW(map.extractAfterExecution(0, indiv, dataSet.getOutputAt(0), problem)) << "Failed to extract output for first evaluation";
    ASSERT_NO_THROW(map.extractionEnd(0, indiv, problem)) << "Failed to finish extraction for first evaluation";

    ASSERT_NO_THROW(map.extractBeforeExecution(1, indiv, dataSet.getInputsAt(1), problem)) << "Failed to extract inputs for second evaluation";
    ASSERT_NO_THROW(map.extractAfterExecution(1, indiv, dataSet.getOutputAt(1), problem)) << "Failed to extract output for second evaluation";
    ASSERT_NO_THROW(map.extractionEnd(1, indiv, problem)) << "Failed to finish extraction for second evaluation";

    auto inputMetrics = map.getMetricValues<Metrics::InputMetric>(Metrics::InputMetric::staticHash());
    ASSERT_EQ(inputMetrics.size(), 2) << "Should contain input metrics for two evaluations";
    ASSERT_TRUE(inputMetrics.find(0) != inputMetrics.end()) << "Input metric for first evaluation should exist";
    ASSERT_TRUE(inputMetrics.find(1) != inputMetrics.end()) << "Input metric for second evaluation should exist";

    ASSERT_EQ(inputMetrics.at(0)->getInputs().size(), 1) << "First input metric should contain one input extraction";
    ASSERT_EQ(inputMetrics.at(1)->getInputs().size(), 1) << "Second input metric should contain one input extraction";
    ASSERT_EQ(inputMetrics.at(0)->getInputs().at(0).at(0), Data::DataValue::array1d<int[2]>({1, 2})) << "First input metric should contain the expected input values";
    ASSERT_EQ(inputMetrics.at(1)->getInputs().at(0).at(0), Data::DataValue::array1d<int[2]>({3, 4})) << "Second input metric should contain the expected input values";

    auto outputMetrics = map.getMetricValues<Metrics::OutputMetric>(Metrics::OutputMetric::staticHash());
    ASSERT_EQ(outputMetrics.size(), 2) << "Should contain output metrics for two evaluations";
    ASSERT_TRUE(outputMetrics.find(0) != outputMetrics.end()) << "Output metric for first evaluation should exist";
    ASSERT_TRUE(outputMetrics.find(1) != outputMetrics.end()) << "Output metric for second evaluation should exist";

    ASSERT_EQ(outputMetrics.at(0)->getOutputs().size(), 1) << "First output metric should contain one output extraction";
    ASSERT_EQ(outputMetrics.at(1)->getOutputs().size(), 1) << "Second output metric should contain one output extraction";
    ASSERT_EQ(outputMetrics.at(0)->getOutputs().at(0), Data::DataValue::scalar<int>(0)) << "First output metric should contain the expected output value";
    ASSERT_EQ(outputMetrics.at(1)->getOutputs().at(0), Data::DataValue::scalar<int>(1)) << "Second output metric should contain the expected output value";
}


TEST(MetricMapTest, getMetrics)
{
    auto mapTemplate = std::make_shared<Metrics::MetricMapTemplate>();
    mapTemplate->addRequiredMetric(std::make_unique<Metrics::InputMetric>());
    mapTemplate->addRequiredMetric(std::make_unique<Metrics::OutputMetric>());
    Metrics::MetricMap map(mapTemplate);

    Evaluation::DataSet dataSet = createSmallDataSet();
    Evaluation::PredictionProblem problem(dataSet);
    Representations::FakeRepresentation rep(problem.getInputDimensions(), problem.getOutputDimension());
    std::unique_ptr<GraphBased::Genotype> genotype = GraphBased::Genotype::singleNodeGenotype(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0}));
    Individual indiv(rep, std::move(genotype));

    ASSERT_NO_THROW(map.extractBeforeExecution(42, indiv, dataSet.getInputsAt(0), problem)) << "Failed to extract inputs";
    ASSERT_NO_THROW(map.extractAfterExecution(42, indiv, dataSet.getOutputAt(0), problem)) << "Failed to extract output";
    ASSERT_NO_THROW(map.extractionEnd(42, indiv, problem)) << "Failed to finish extraction";

    auto inputMetrics = map.getMetricValues<Metrics::InputMetric>(Metrics::InputMetric::staticHash());
    ASSERT_EQ(inputMetrics.size(), 1) << "Should contain one input metric";
    ASSERT_TRUE(inputMetrics.find(42) != inputMetrics.end()) << "Input metric for hash 42 should exist";
    ASSERT_NE(inputMetrics.at(42), nullptr) << "Input metric should not be null";
    ASSERT_EQ(inputMetrics.at(42)->getInputs().size(), 1) << "Input metric should contain one input extraction";
    ASSERT_EQ(inputMetrics.at(42)->getInputs().at(0).size(), 1) << "Input extraction should contain one source";
    ASSERT_EQ(inputMetrics.at(42)->getInputs().at(0).at(0), Data::DataValue::array1d<int[2]>({1, 2})) << "Input metric should contain the expected input values";

    auto outputMetrics = map.getMetricValues<Metrics::OutputMetric>(Metrics::OutputMetric::staticHash());
    ASSERT_EQ(outputMetrics.size(), 1) << "Should contain one output metric";
    ASSERT_TRUE(outputMetrics.find(42) != outputMetrics.end()) << "Output metric for hash 42 should exist";
    ASSERT_NE(outputMetrics.at(42), nullptr) << "Output metric should not be null";
    ASSERT_EQ(outputMetrics.at(42)->getOutputs().size(), 1) << "Output metric should contain one output extraction";
    ASSERT_EQ(outputMetrics.at(42)->getOutputs().at(0), Data::DataValue::scalar<int>(0)) << "Output metric should contain the expected output value";
}


TEST(MetricMapTest, getMetricsWrongType)
{
    auto mapTemplate = std::make_shared<Metrics::MetricMapTemplate>();
    mapTemplate->addRequiredMetric(std::make_unique<Metrics::InputMetric>());
    Metrics::MetricMap map(mapTemplate);

    Evaluation::DataSet dataSet = createSmallDataSet();
    Evaluation::PredictionProblem problem(dataSet);
    Representations::FakeRepresentation rep(problem.getInputDimensions(), problem.getOutputDimension());
    std::unique_ptr<GraphBased::Genotype> genotype = GraphBased::Genotype::singleNodeGenotype(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0}));
    Individual indiv(rep, std::move(genotype));

    ASSERT_NO_THROW(map.extractBeforeExecution(0, indiv, dataSet.getInputsAt(0), problem)) << "Failed to extract inputs";
    ASSERT_NO_THROW(map.extractionEnd(0, indiv, problem)) << "Failed to finish extraction";

    ASSERT_THROW(map.getMetricValues<Metrics::OutputMetric>(Metrics::InputMetric::staticHash()), std::runtime_error) << "Requesting an incompatible metric type should throw";
}
TEST(MetricMapTest, merge)
{
    auto mapTemplate = std::make_shared<Metrics::MetricMapTemplate>();
    mapTemplate->addRequiredMetric(std::make_unique<Metrics::InputMetric>());
    mapTemplate->addRequiredMetric(std::make_unique<Metrics::OutputMetric>());

    Metrics::MetricMap map1(mapTemplate);
    Metrics::MetricMap map2(mapTemplate);

    Evaluation::DataSet dataSet = createSmallDataSet();
    Evaluation::PredictionProblem problem(dataSet);
    Representations::FakeRepresentation rep(problem.getInputDimensions(), problem.getOutputDimension());
    std::unique_ptr<GraphBased::Genotype> genotype = GraphBased::Genotype::singleNodeGenotype(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0}));
    Individual indiv(rep, std::move(genotype));

    ASSERT_NO_THROW(map1.extractBeforeExecution(0, indiv, dataSet.getInputsAt(0), problem)) << "Failed to extract inputs into first metric map";
    ASSERT_NO_THROW(map1.extractAfterExecution(0, indiv, dataSet.getOutputAt(0), problem)) << "Failed to extract output into first metric map";
    ASSERT_NO_THROW(map1.extractionEnd(0, indiv, problem)) << "Failed to finish extraction in first metric map";

    ASSERT_NO_THROW(map2.extractBeforeExecution(1, indiv, dataSet.getInputsAt(1), problem)) << "Failed to extract inputs into second metric map";
    ASSERT_NO_THROW(map2.extractAfterExecution(1, indiv, dataSet.getOutputAt(1), problem)) << "Failed to extract output into second metric map";
    ASSERT_NO_THROW(map2.extractionEnd(1, indiv, problem)) << "Failed to finish extraction in second metric map";

    ASSERT_EQ(map1.getMetricValues<Metrics::InputMetric>(Metrics::InputMetric::staticHash()).size(), 1) << "First metric map should contain one input metric";
    ASSERT_EQ(map1.getMetricValues<Metrics::OutputMetric>(Metrics::OutputMetric::staticHash()).size(), 1) << "First metric map should contain one output metric";
    ASSERT_EQ(map2.getMetricValues<Metrics::InputMetric>(Metrics::InputMetric::staticHash()).size(), 1) << "Second metric map should contain one input metric";
    ASSERT_EQ(map2.getMetricValues<Metrics::OutputMetric>(Metrics::OutputMetric::staticHash()).size(), 1) << "Second metric map should contain one output metric";

    ASSERT_NO_THROW(map1.merge(map2)) << "Failed to merge metric maps";

    auto inputMetrics = map1.getMetricValues<Metrics::InputMetric>(Metrics::InputMetric::staticHash());
    auto outputMetrics = map1.getMetricValues<Metrics::OutputMetric>(Metrics::OutputMetric::staticHash());

    ASSERT_EQ(inputMetrics.size(), 2) << "Merged map should contain two input metrics";
    ASSERT_EQ(outputMetrics.size(), 2) << "Merged map should contain two output metrics";
    ASSERT_TRUE(inputMetrics.find(0) != inputMetrics.end()) << "Merged map should contain input metric for first evaluation";
    ASSERT_TRUE(inputMetrics.find(1) != inputMetrics.end()) << "Merged map should contain input metric for second evaluation";
    ASSERT_TRUE(outputMetrics.find(0) != outputMetrics.end()) << "Merged map should contain output metric for first evaluation";
    ASSERT_TRUE(outputMetrics.find(1) != outputMetrics.end()) << "Merged map should contain output metric for second evaluation";

    ASSERT_EQ(inputMetrics.at(0)->getInputs().at(0).at(0), Data::DataValue::array1d<int[2]>({1, 2})) << "First merged input metric should contain the expected input values";
    ASSERT_EQ(inputMetrics.at(1)->getInputs().at(0).at(0), Data::DataValue::array1d<int[2]>({3, 4})) << "Second merged input metric should contain the expected input values";

    ASSERT_EQ(outputMetrics.at(0)->getOutputs().size(), 1) << "First merged output metric should contain one output";
    ASSERT_EQ(outputMetrics.at(1)->getOutputs().size(), 1) << "Second merged output metric should contain one output";
    ASSERT_EQ(outputMetrics.at(0)->getOutputs().at(0), dataSet.getOutputAt(0)) << "First merged output metric should contain the expected output";
    ASSERT_EQ(outputMetrics.at(1)->getOutputs().at(0), dataSet.getOutputAt(1)) << "Second merged output metric should contain the expected output";

    auto inputOnlyTemplate = std::make_shared<Metrics::MetricMapTemplate>();
    inputOnlyTemplate->addRequiredMetric(std::make_unique<Metrics::InputMetric>());

    auto outputOnlyTemplate = std::make_shared<Metrics::MetricMapTemplate>();
    outputOnlyTemplate->addRequiredMetric(std::make_unique<Metrics::OutputMetric>());

    Metrics::MetricMap inputOnlyMap(inputOnlyTemplate);
    Metrics::MetricMap outputOnlyMap(outputOnlyTemplate);

    ASSERT_NO_THROW(inputOnlyMap.extractBeforeExecution(0, indiv, dataSet.getInputsAt(0), problem)) << "Failed to extract input for unknown metric test";
    ASSERT_NO_THROW(inputOnlyMap.extractAfterExecution(0, indiv, dataSet.getOutputAt(0), problem)) << "Failed to extract output for unknown metric test";
    ASSERT_NO_THROW(inputOnlyMap.extractionEnd(0, indiv, problem)) << "Failed to finish extraction for unknown metric test";

    ASSERT_NO_THROW(outputOnlyMap.extractBeforeExecution(0, indiv, dataSet.getInputsAt(0), problem)) << "Failed to extract input for additional metric test";
    ASSERT_NO_THROW(outputOnlyMap.extractAfterExecution(0, indiv, dataSet.getOutputAt(0), problem)) << "Failed to extract output for additional metric test";
    ASSERT_NO_THROW(outputOnlyMap.extractionEnd(0, indiv, problem)) << "Failed to finish extraction for additional metric test";

    ASSERT_EQ(inputOnlyMap.getMetricValues<Metrics::InputMetric>(Metrics::InputMetric::staticHash()).size(), 1) << "Input-only map should contain one input metric";
    ASSERT_EQ(inputOnlyMap.getMetricValues<Metrics::OutputMetric>(Metrics::OutputMetric::staticHash()).size(), 0) << "Input-only map should not contain an output metric";
    ASSERT_EQ(outputOnlyMap.getMetricValues<Metrics::InputMetric>(Metrics::InputMetric::staticHash()).size(), 0) << "Output-only map should not contain an input metric";
    ASSERT_EQ(outputOnlyMap.getMetricValues<Metrics::OutputMetric>(Metrics::OutputMetric::staticHash()).size(), 1) << "Output-only map should contain one output metric";

    ASSERT_NO_THROW(inputOnlyMap.merge(outputOnlyMap)) << "Failed to add an unknown metric to a known feature";

    auto mergedInputMetrics = inputOnlyMap.getMetricValues<Metrics::InputMetric>(Metrics::InputMetric::staticHash());
    auto mergedOutputMetrics = inputOnlyMap.getMetricValues<Metrics::OutputMetric>(Metrics::OutputMetric::staticHash());

    ASSERT_EQ(mergedInputMetrics.size(), 1) << "Merged map should still contain one input metric";
    ASSERT_EQ(mergedOutputMetrics.size(), 1) << "Merged map should contain the newly added output metric";
    ASSERT_TRUE(mergedInputMetrics.find(0) != mergedInputMetrics.end()) << "Merged map should contain the existing input metric";
    ASSERT_TRUE(mergedOutputMetrics.find(0) != mergedOutputMetrics.end()) << "Merged map should contain the newly added output metric";
    ASSERT_EQ(mergedOutputMetrics.at(0)->getOutputs().size(), 1) << "Newly added output metric should contain one output";
    ASSERT_EQ(mergedOutputMetrics.at(0)->getOutputs().at(0), dataSet.getOutputAt(0)) << "Newly added output metric should contain the expected output";

    Metrics::MetricMap overrideMap1(mapTemplate);
    Metrics::MetricMap overrideMap2(mapTemplate);

    ASSERT_NO_THROW(overrideMap1.extractBeforeExecution(0, indiv, dataSet.getInputsAt(0), problem)) << "Failed to extract initial input for override test";
    ASSERT_NO_THROW(overrideMap1.extractAfterExecution(0, indiv, dataSet.getOutputAt(0), problem)) << "Failed to extract initial output for override test";
    ASSERT_NO_THROW(overrideMap1.extractionEnd(0, indiv, problem)) << "Failed to finish initial extraction for override test";

    ASSERT_NO_THROW(overrideMap2.extractBeforeExecution(0, indiv, dataSet.getInputsAt(2), problem)) << "Failed to extract replacement input for override test";
    ASSERT_NO_THROW(overrideMap2.extractAfterExecution(0, indiv, dataSet.getOutputAt(2), problem)) << "Failed to extract replacement output for override test";
    ASSERT_NO_THROW(overrideMap2.extractionEnd(0, indiv, problem)) << "Failed to finish replacement extraction for override test";

    ASSERT_NO_THROW(overrideMap1.merge(overrideMap2)) << "Failed to override existing metrics";

    auto overriddenInputMetrics = overrideMap1.getMetricValues<Metrics::InputMetric>(Metrics::InputMetric::staticHash());
    auto overriddenOutputMetrics = overrideMap1.getMetricValues<Metrics::OutputMetric>(Metrics::OutputMetric::staticHash());

    ASSERT_EQ(overriddenInputMetrics.size(), 1) << "Override merge should keep one input metric for the same evaluation";
    ASSERT_EQ(overriddenOutputMetrics.size(), 1) << "Override merge should keep one output metric for the same evaluation";
    ASSERT_EQ(overriddenInputMetrics.at(0)->getInputs().size(), 1) << "Overridden input metric should contain one input";
    ASSERT_EQ(overriddenOutputMetrics.at(0)->getOutputs().size(), 1) << "Overridden output metric should contain one output";
    ASSERT_EQ(overriddenInputMetrics.at(0)->getInputs().at(0).at(0), dataSet.getInputsAt(2).at(0)) << "Existing input metric should be overridden";
    ASSERT_EQ(overriddenOutputMetrics.at(0)->getOutputs().at(0), dataSet.getOutputAt(2)) << "Existing output metric should be overridden";
}


TEST(MetricMapTest, toString)
{
    auto mapTemplate = std::make_shared<Metrics::MetricMapTemplate>();
    mapTemplate->addRequiredMetric(std::make_unique<Metrics::InputMetric>());
    mapTemplate->addRequiredMetric(std::make_unique<Metrics::OutputMetric>());
    Metrics::MetricMap map(mapTemplate);

    Evaluation::DataSet dataSet = createSmallDataSet();
    Evaluation::PredictionProblem problem(dataSet);
    Representations::FakeRepresentation rep(problem.getInputDimensions(), problem.getOutputDimension());
    std::unique_ptr<GraphBased::Genotype> genotype = GraphBased::Genotype::singleNodeGenotype(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0}));
    Individual indiv(rep, std::move(genotype));

    ASSERT_NO_THROW(map.extractBeforeExecution(0, indiv, dataSet.getInputsAt(0), problem)) << "Failed to extract inputs for first evaluation";
    ASSERT_NO_THROW(map.extractAfterExecution(0, indiv, dataSet.getOutputAt(0), problem)) << "Failed to extract output for first evaluation";
    ASSERT_NO_THROW(map.extractionEnd(0, indiv, problem)) << "Failed to finish extraction for first evaluation";

    ASSERT_NO_THROW(map.extractBeforeExecution(1, indiv, dataSet.getInputsAt(1), problem)) << "Failed to extract inputs for second evaluation";
    ASSERT_NO_THROW(map.extractAfterExecution(1, indiv, dataSet.getOutputAt(1), problem)) << "Failed to extract output for second evaluation";
    ASSERT_NO_THROW(map.extractionEnd(1, indiv, problem)) << "Failed to finish extraction for second evaluation";

    ASSERT_NO_THROW(map.toString())<< "Should not fail";
    
    std::stringstream ss;
    ss << map << std::endl; // For coverage
}