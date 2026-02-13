

#include <gtest/gtest.h>
#include <stdexcept>

#include "learn/classificationLearningEnvironment.h"
#include "selector/classificationSelectionMetrics.h"
#include "selector/selectionMetrics.h"

#include <memory>

// Fake LearningEnvironment to test SelectionMetrics::extractMetricsEpisode
class FakedLearningEnvironment : public Learn::LearningEnvironment
{
  private:
    double m_score;
    double m_utility;

  public:
    FakedLearningEnvironment(double score, double utility)
        : Learn::LearningEnvironment(1), m_score(score), m_utility(utility)
    {
    }

    virtual void reset(size_t seed = 0,
                       Learn::LearningMode mode = Learn::LearningMode::TRAINING,
                       uint16_t iterationNumber = 0,
                       uint64_t generationNumber = 0) override
    {
    }

    virtual std::vector<std::reference_wrapper<const Data::DataHandler>>
    getDataSources() override
    {
        return {};
    }

    virtual double getScore() const override
    {
        return m_score;
    }

    virtual double getUtility() const override
    {
        return m_utility;
    }

    virtual bool isTerminal() const override
    {
        return true;
    }

    virtual bool isUsingUtility() const override
    {
        return true;
    }
};

// Fake ClassificationLearningEnvironment for testing
// ClassificationSelectionMetrics
class FakedClassificationLearningEnvironment
    : public Learn::ClassificationLearningEnvironment
{
  public:
    FakedClassificationLearningEnvironment(uint64_t nbClass)
        : Learn::ClassificationLearningEnvironment(nbClass)
    {
    }

    void setClassificationTable(const std::vector<std::vector<uint64_t>>& table)
    {
        this->classificationTable = table;
    }

    virtual void doAction(double actionID) override
    {
    }
    virtual void reset(size_t seed = 0,
                       Learn::LearningMode mode = Learn::LearningMode::TRAINING,
                       uint16_t iterationNumber = 0,
                       uint64_t generationNumber = 0) override
    {
    }

    virtual std::vector<std::reference_wrapper<const Data::DataHandler>>
    getDataSources() override
    {
        return {};
    }

    virtual bool isTerminal() const override
    {
        return true;
    }
};

TEST(SelectionMetricsTest, DefaultAndParamConstructor)
{
    // Default constructor
    Selector::SelectionMetrics defaultMetrics;
    ASSERT_DOUBLE_EQ(defaultMetrics.getScore(), 0.0);
    ASSERT_DOUBLE_EQ(defaultMetrics.getUtility(), 0.0);

    // Param constructor
    Selector::SelectionMetrics paramMetrics(1.5, 2.5);
    ASSERT_DOUBLE_EQ(paramMetrics.getScore(), 1.5);
    ASSERT_DOUBLE_EQ(paramMetrics.getUtility(), 2.5);
}

TEST(SelectionMetricsTest, ExtractMetricsEpisodeAddsScoreAndUtility)
{
    Selector::SelectionMetrics metrics(1.5, 2.5);
    FakedLearningEnvironment env(3.0, 1.0);

    // Call extraction
    const Algorithm::Agent* nullAgent = nullptr;
    metrics.extractMetricsEpisode(*nullAgent, 0, env);

    // score and utility should be incremented
    ASSERT_DOUBLE_EQ(metrics.getScore(), 4.5);
    ASSERT_DOUBLE_EQ(metrics.getUtility(), 3.5);
}

TEST(SelectionMetricsTest, WeightedSumAndTypeMismatch)
{
    auto m1 = std::make_shared<Selector::SelectionMetrics>(1.0, 2.0);
    auto m2 = std::make_shared<Selector::SelectionMetrics>(3.0, 5.0);

    // weighted sum according to current implementation
    ASSERT_NO_THROW(m1->weightedSum(m2, 2, 3));
    // score = (1*2 + 3*3) / 5 = 11/5 = 2.2
    ASSERT_EQ(m1->getScore(), 2.2);
    // utility computed by implementation (note: uses updated score in formula)
    // utility = (score * 2 + other.utility * 3) / 5 = (2.2*2 + 5*3)/5 = 19.4/5
    // = 3.88
    ASSERT_EQ(m1->getUtility(), 3.88);

    // Type mismatch should throw
    auto classMetrics =
        std::make_shared<Selector::ClassificationSelectionMetrics>(
            std::vector<double>{1.0}, std::vector<size_t>{1});
    ASSERT_THROW(m1->weightedSum(classMetrics, 1, 1), std::runtime_error);
}

TEST(ClassificationSelectionMetricsTest, Constructor)
{
    std::shared_ptr<Selector::SelectionMetrics> metrics;

    ASSERT_NO_THROW(
        metrics = std::make_shared<Selector::ClassificationSelectionMetrics>(
            std::vector<double>{1.0, 2.0}, std::vector<size_t>{2, 3}))
        << "Building a ClassificationSelectionMetrics failed unexpectedly.";

    ASSERT_THROW(
        metrics = std::make_shared<Selector::ClassificationSelectionMetrics>(
            std::vector<double>{1.0, 2.0}, std::vector<size_t>{2, 3, 5}),
        std::runtime_error)
        << "Building a ClassificationSelectionMetrics with vectors of "
           "different sizes should fail.";
}

TEST(ClassificationSelectionMetricsTest, GetScore)
{
    auto metrics = std::make_shared<Selector::ClassificationSelectionMetrics>(
        std::vector<double>{1.0, 2.0}, std::vector<size_t>{2, 3});

    ASSERT_EQ(metrics->getScore(), (1.0 + 2.0) / 2.0)
        << "Getter returned an unexpected value.";
}

TEST(ClassificationSelectionMetricsTest, GetScorePerClass)
{
    auto metrics = std::make_shared<Selector::ClassificationSelectionMetrics>(
        std::vector<double>{1.0, 2.0}, std::vector<size_t>{2, 3});

    ASSERT_EQ(metrics->getScorePerClass().size(), 2)
        << "Getter returned an unexpected value.";
    ASSERT_EQ(metrics->getScorePerClass().at(0), 1.0)
        << "Getter returned an unexpected value.";
    ASSERT_EQ(metrics->getScorePerClass().at(1), 2.0)
        << "Getter returned an unexpected value.";
    ASSERT_EQ(metrics->getScorePerClass().at(1), 2.0)
        << "Getter returned an unexpected value.";
}

TEST(ClassificationSelectionMetricsTest, GetNbEvaluationPerClass)
{
    auto metrics = std::make_shared<Selector::ClassificationSelectionMetrics>(
        std::vector<double>{1.0, 2.0}, std::vector<size_t>{2, 3});

    ASSERT_EQ(metrics->getNbEvalPerClassPerClass().size(), 2)
        << "Getter returned an unexpected value.";
    ASSERT_EQ(metrics->getNbEvalPerClassPerClass().at(0), 2)
        << "Getter returned an unexpected value.";
    ASSERT_EQ(metrics->getNbEvalPerClassPerClass().at(1), 3)
        << "Getter returned an unexpected value.";
}

TEST(ClassificationSelectionMetricsTest, WeightedSum)
{
    auto metrics1 = std::make_shared<Selector::ClassificationSelectionMetrics>(
        std::vector<double>{1.0, 2.0}, std::vector<size_t>{2, 3});
    auto metrics2 = std::make_shared<Selector::ClassificationSelectionMetrics>(
        std::vector<double>{2.0, 3.0}, std::vector<size_t>{2, 2});

    ASSERT_NO_THROW(metrics1->weightedSum(metrics2, 5, 4))
        << "Call to weightedSum failed unexpectedly.";

    auto metrics3 = std::make_shared<Selector::ClassificationSelectionMetrics>(
        std::vector<double>{3.0, 4.0, 5.0}, std::vector<size_t>{2, 3, 4});
    ASSERT_THROW(metrics1->weightedSum(metrics3, 5, 9), std::runtime_error)
        << "Call to weightedSum should not work with mismatched "
           "number of classes.";
}

TEST(ClassificationSelectionMetricsTest, InitAndExtractEpisode)
{
    Selector::ClassificationSelectionMetrics metrics;
    FakedClassificationLearningEnvironment env(2);
    const Algorithm::Agent* nullAgent = nullptr;

    // Prepare classification table: class 0 guessed [2,1], class 1 guessed
    // [0,3]
    env.setClassificationTable(
        std::vector<std::vector<uint64_t>>{{2, 1}, {0, 3}});

    // init should resize internal vectors
    metrics.initMetrics(*nullAgent, env);
    ASSERT_EQ(metrics.getScorePerClass().size(), 2);
    ASSERT_EQ(metrics.getNbEvalPerClassPerClass().size(), 2);

    // extract should compute f-scores per class and number of evals
    metrics.extractMetricsEpisode(*nullAgent, 0, env);

    // class 0: TP=2 FN=1 FP=0 => precision=1 recall=2/3 => fscore=4/5 = 0.8
    ASSERT_EQ(metrics.getScorePerClass().at(0), 0.8);
    // class 1: TP=3 FN=0 FP=1 => precision=3/4 recall=1 => fscore = 6/7
    ASSERT_EQ(metrics.getScorePerClass().at(1), 6.0 / 7.0);

    ASSERT_EQ(metrics.getNbEvalPerClassPerClass().at(0), 3);
    ASSERT_EQ(metrics.getNbEvalPerClassPerClass().at(1), 3);
}