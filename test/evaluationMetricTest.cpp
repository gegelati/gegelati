

#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>

#include "evaluation/evaluationMetric.h"


// Fake LearningEnvironment to test SelectionMetrics::extractMetricsEpisode
class FakedLearningEnvironment : public Learn::LearningEnvironment
{
  private:
    double m_score;

  public:
    FakedLearningEnvironment(double score)
        : Learn::LearningEnvironment(1), m_score(score)
    {
    }

    virtual void reset(size_t seed = 0,
                       Learn::LearningMode mode = Learn::LearningMode::TRAINING,
                       uint16_t iterationNumber = 0,
                       uint64_t generationNumber = 0) override
    {
    }

    virtual std::vector<std::reference_wrapper<const Data::DataHandler>>
    getDataSources() const override
    {
        return {};
    }

    virtual double getScore() const override
    {
        return m_score;
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

TEST(EvaluationMetricTest, Constructor)
{
    // Default constructor
    Evaluation::EvaluationMetric* metric1;
    Evaluation::EvaluationMetric* metric2;

    ASSERT_NO_THROW(metric1 = new Evaluation::EvaluationMetric()) << "Constructor failed";
    ASSERT_NO_THROW(metric2 = new Evaluation::EvaluationMetric(1.5)) << "Constructor failed";

    ASSERT_NO_THROW(delete metric1) << "Destructor failed";
    ASSERT_NO_THROW(delete metric2) << "Destructor failed";
}

TEST(EvaluationMetricTest, ExtractMetricsEpisode)
{
    Evaluation::EvaluationMetric metric;
    FakedLearningEnvironment env(3.0);

    // Call extraction
    Evolution::Individual indiv;
    metric.extractMetricsRun(indiv, 0, env);

    // score should be set
    ASSERT_DOUBLE_EQ(metric.getScore(), 3.0);
}