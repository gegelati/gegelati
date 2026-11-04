#include <gtest/gtest.h>

#include "evaluation/evaluator.h"

#include "evaluation/predictionProblem.h"

#include "evaluation/reinforcementProblem.h"

#include "metrics/inputMetric.h"

#include "metrics/outputMetric.h"

#include "learn/stickGameWithOpponent.h"

#include "learn/fakeRepresentation.h"

#include "learn/dataSetExamples.h"


TEST(EvaluatorTest, Constructor)
{
    Evaluation::Evaluator* evaluator;

    ASSERT_NO_THROW(evaluator = new Evaluation::Evaluator()) << "Construction failed";

    ASSERT_NO_THROW(delete evaluator) << "Destruction failed";
}

TEST(EvaluatorTest, evaluateIndividualsOnRL)
{
    std::shared_ptr<Evaluation::ReinforcementEnvironment> le = std::make_shared<StickGameWithOpponent>();
    Evaluation::ReinforcementProblem problem(le);
    Evaluation::Evaluator evaluator;

    auto rep = std::make_shared<Representations::FakeRepresentation>(problem.getInputDimensions(), problem.getOutputDimension());
    std::unique_ptr<GraphBased::Genotype> genotype = GraphBased::Genotype::singleNodeGenotype(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0}));
    std::shared_ptr<const Individual> indiv = std::make_shared<Individual>(rep, std::move(genotype));
    ASSERT_TRUE(indiv->isValid()) << "Individual should be valid";
    const uint64_t indivID = indiv->getIndividualID();

    std::vector<std::shared_ptr<const Individual>> individuals{indiv};

    auto metricTemplate = std::make_shared<Metrics::MetricMapTemplate>();
    metricTemplate->addRequiredMetric(std::make_unique<Metrics::InputMetric>());
    metricTemplate->addRequiredMetric(std::make_unique<Metrics::OutputMetric>());

    std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>> results;
    ASSERT_NO_THROW(results = evaluator.evaluateIndividuals(individuals, problem, metricTemplate, 2, 0, Evaluation::Mode::TESTING)) << "RL evaluation failed";
    ASSERT_EQ(results.size(), 1) << "There should be one evaluation result";
    ASSERT_TRUE(results.find(indivID) != results.end()) << "The evaluated individual should be present in the results";
    ASSERT_NE(results.at(indivID), nullptr) << "The metric map should not be null";

    std::map<uint64_t, const Metrics::InputMetric*> inputMetrics = results.at(indivID)->getMetricValues<Metrics::InputMetric>(Metrics::InputMetric::staticHash());
    ASSERT_EQ(inputMetrics.size(), 2) << "There should be one input metric for each evaluation hash";

    std::map<uint64_t, const Metrics::OutputMetric*> outputMetrics = results.at(indivID)->getMetricValues<Metrics::OutputMetric>(Metrics::OutputMetric::staticHash());
    ASSERT_EQ(outputMetrics.size(), 2) << "There should be one output metric for each evaluation hash";

    std::set<uint64_t> expectedHashes = problem.computeEvaluationHashes(2, 0, Evaluation::Mode::TESTING, problem.getProblemSeed());
    auto it = expectedHashes.begin();
    uint64_t hash0 = *it; it++;
    uint64_t hash1 = *it;
    ASSERT_TRUE(inputMetrics.find(hash0) != inputMetrics.end()) << "Input metric should contain hash 0";
    ASSERT_TRUE(inputMetrics.find(hash1) != inputMetrics.end()) << "Input metric should contain hash 1";

    ASSERT_TRUE(outputMetrics.find(hash0) != outputMetrics.end()) << "Output metric should contain hash 0";
    ASSERT_TRUE(outputMetrics.find(hash1) != outputMetrics.end()) << "Output metric should contain hash 1";

    ASSERT_GT(inputMetrics.at(hash0)->getInputs().size(), 1) << "RL input metric should contain several inputs";
    ASSERT_GT(inputMetrics.at(hash1)->getInputs().size(), 1) << "RL input metric should contain several inputs";

    ASSERT_GT(outputMetrics.at(hash0)->getOutputs().size(), 1) << "RL output metric should contain several outputs";
    ASSERT_GT(outputMetrics.at(hash1)->getOutputs().size(), 1) << "RL output metric should contain several outputs";
}


TEST(EvaluatorTest, evaluateIndividualsOnClassif)
{
    Evaluation::DataSet dataSet = createSmallDataSet();
    Evaluation::PredictionProblem problem(dataSet);
    Evaluation::Evaluator evaluator;

    auto rep = std::make_shared<Representations::FakeRepresentation>(problem.getInputDimensions(), problem.getOutputDimension());
    rep->output = Data::DataValue::scalar<int>(0);
    std::unique_ptr<GraphBased::Genotype> genotype = GraphBased::Genotype::singleNodeGenotype(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0}));
    std::shared_ptr<const Individual> indiv = std::make_shared<Individual>(rep, std::move(genotype));
    ASSERT_TRUE(indiv->isValid()) << "Individual should be valid";
    const uint64_t indivID = indiv->getIndividualID();

    std::vector<std::shared_ptr<const Individual>> individuals{indiv};

    auto metricTemplate = std::make_shared<Metrics::MetricMapTemplate>();
    metricTemplate->addRequiredMetric(std::make_unique<Metrics::InputMetric>());
    metricTemplate->addRequiredMetric(std::make_unique<Metrics::OutputMetric>());
    std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>> results;

    ASSERT_NO_THROW(results = evaluator.evaluateIndividuals(individuals, problem, metricTemplate, 2, 0, Evaluation::Mode::TRAINING)) << "Classification evaluation failed";
    ASSERT_EQ(results.size(), 1) << "There should be one evaluation result";
    ASSERT_TRUE(results.find(indivID) != results.end()) << "The evaluated individual should be present in the results";
    ASSERT_NE(results.at(indivID), nullptr) << "The metric map should not be null";

    std::set<uint64_t> expectedHashes = problem.computeEvaluationHashes(2, 0, Evaluation::Mode::TRAINING, problem.getProblemSeed());
    ASSERT_EQ(expectedHashes.size(), 2) << "There should be two distinct evaluation hashes";

    auto it = expectedHashes.begin();
    uint64_t hash0 = *it;
    it++;
    uint64_t hash1 = *it;

    std::map<uint64_t, const Metrics::InputMetric*> inputMetrics = results.at(indivID)->getMetricValues<Metrics::InputMetric>(Metrics::InputMetric::staticHash());
    ASSERT_EQ(inputMetrics.size(), 2) << "There should be two input metrics";
    ASSERT_TRUE(inputMetrics.find(hash0) != inputMetrics.end()) << "Input metric should contain the first evaluation hash";
    ASSERT_TRUE(inputMetrics.find(hash1) != inputMetrics.end()) << "Input metric should contain the second evaluation hash";

    std::map<uint64_t, const Metrics::OutputMetric*> outputMetrics = results.at(indivID)->getMetricValues<Metrics::OutputMetric>(Metrics::OutputMetric::staticHash());
    ASSERT_EQ(outputMetrics.size(), 2) << "There should be two output metrics";
    ASSERT_TRUE(outputMetrics.find(hash0) != outputMetrics.end()) << "Output metric should contain the first evaluation hash";
    ASSERT_TRUE(outputMetrics.find(hash1) != outputMetrics.end()) << "Output metric should contain the second evaluation hash";

    ASSERT_EQ(inputMetrics.at(hash0)->getInputs().size(), 1) << "Classification input metric should contain one input";
    ASSERT_EQ(inputMetrics.at(hash0)->getInputs().at(0).size(), 1) << "Classification input metric should contain one data source value";
    ASSERT_EQ(inputMetrics.at(hash0)->getInputs().at(0).at(0), dataSet.getInputsAt(hash0).at(0)) << "Input values should be equal";

    ASSERT_EQ(inputMetrics.at(hash1)->getInputs().size(), 1) << "Classification input metric should contain one input";
    ASSERT_EQ(inputMetrics.at(hash1)->getInputs().at(0).size(), 1) << "Classification input metric should contain one data source value";
    ASSERT_EQ(inputMetrics.at(hash1)->getInputs().at(0).at(0), dataSet.getInputsAt(hash1).at(0)) << "Input values should be equal";

    ASSERT_EQ(outputMetrics.at(hash0)->getOutputs().size(), 1) << "Classification output metric should contain one output";
    ASSERT_EQ(outputMetrics.at(hash0)->getOutputs().at(0), Data::DataValue::scalar<int>(0)) << "Output value should be equal";

    ASSERT_EQ(outputMetrics.at(hash1)->getOutputs().size(), 1) << "Classification output metric should contain one output";
    ASSERT_EQ(outputMetrics.at(hash1)->getOutputs().at(0), Data::DataValue::scalar<int>(0)) << "Output value should be equal";
}

TEST(EvaluatorTest, evaluateIndividualsInvalidArguments)
{
    Evaluation::DataSet dataSet = createSmallDataSet();
    Evaluation::PredictionProblem problem(dataSet);
    Evaluation::Evaluator evaluator;

    auto rep = std::make_shared<Representations::FakeRepresentation>(problem.getInputDimensions(), problem.getOutputDimension());
    std::unique_ptr<GraphBased::Genotype> genotype = GraphBased::Genotype::singleNodeGenotype(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0}));
    std::shared_ptr<const Individual> indiv = std::make_shared<Individual>(rep, std::move(genotype));

    std::vector<std::shared_ptr<const Individual>> individuals{indiv};

    auto emptyMetricTemplate = std::make_shared<Metrics::MetricMapTemplate>();

    ASSERT_THROW(evaluator.evaluateIndividuals(individuals, problem, emptyMetricTemplate, 2, 0, Evaluation::Mode::TESTING), std::runtime_error) << "Evaluation should fail with an empty metric template";

    auto metricTemplate = std::make_shared<Metrics::MetricMapTemplate>();
    metricTemplate->addRequiredMetric(std::make_unique<Metrics::InputMetric>());

    std::vector<std::shared_ptr<const Individual>> emptyIndividuals;

    ASSERT_THROW(evaluator.evaluateIndividuals(emptyIndividuals, problem, metricTemplate, 2, 0, Evaluation::Mode::TESTING), std::runtime_error) << "Evaluation should fail with an empty set of individuals";
}
