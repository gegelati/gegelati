

#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>

#include "evaluation/scoreMetric.h"
#include "evolution/individual.h"


// Fake LearningEnvironment to test SelectionMetrics::extractMetricsEpisode
class FakedLearningEnvironment : public Evaluation::LearningEnvironment
{
  private:
    double m_score;
    

  public:
    FakedLearningEnvironment(double score)
        : Evaluation::LearningEnvironment({}, Data::DataRequirement()), m_score(score)
    {
    }

    virtual std::vector<Data::DataView>
    getDataSources() const override
    {
        return {};
    }

    virtual double getScore() const override
    {
        return m_score;
    }
};

TEST(ScoreMetricTest, Constructor)
{
    // Default constructor
    Evaluation::ScoreMetric* metric1;
    Evaluation::ScoreMetric* metric2;

    ASSERT_NO_THROW(metric1 = new Evaluation::ScoreMetric()) << "Constructor failed";
    ASSERT_NO_THROW(metric2 = new Evaluation::ScoreMetric(1.5)) << "Constructor failed";

    ASSERT_NO_THROW(metric1->cloneEmptyUniquePtr()) << "Construction with cloning failed";

    ASSERT_NO_THROW(delete metric1) << "Destructor failed";
    ASSERT_NO_THROW(delete metric2) << "Destructor failed";
}

TEST(ScoreMetricTest, ExtractMetricsEpisode)
{
    Evaluation::ScoreMetric metric;
    FakedLearningEnvironment env(3.0);

    // Call extraction
    Evolution::Individual indiv;
    metric.extractMetricsRun(indiv, 0, env);

    // score should be set
    ASSERT_DOUBLE_EQ(metric.getScore(), 3.0);
}