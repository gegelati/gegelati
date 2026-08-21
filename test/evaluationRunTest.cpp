
#include <gtest/gtest.h>
#include <stdexcept>

#include "evaluation/evaluationRun.h"
#include "evaluation/scoreMetric.h"


TEST(EvaluationRunTest, Constructor)
{
    Evaluation::EvaluationRun* eval1;
    Evaluation::EvaluationRun* eval2;
    Evaluation::EvaluationRun* eval3;

    std::unique_ptr<Evaluation::ScoreMetric> metric1 = std::make_unique<Evaluation::ScoreMetric>(1.0);
    std::vector<std::unique_ptr<Evaluation::EvaluationMetric>> metrics;
    metrics.push_back(std::make_unique<Evaluation::ScoreMetric>(2.0));
    metrics.push_back(std::make_unique<Evaluation::ScoreMetric>(3.0));

    ASSERT_NO_THROW(eval1 = new Evaluation::EvaluationRun())
        << "Building an EvaluationRun failed unexpectedly.";
    
    ASSERT_NO_THROW(eval2 = new Evaluation::EvaluationRun(std::move(metric1)))
        << "Building an EvaluationRun failed unexpectedly.";

    ASSERT_NO_THROW(eval3 = new Evaluation::EvaluationRun(std::move(metrics)))
        << "Building an EvaluationRun failed unexpectedly.";

    ASSERT_NO_THROW(delete eval1);
    ASSERT_NO_THROW(delete eval2);
    ASSERT_NO_THROW(delete eval3);
}

TEST(EvaluationRunTest, addMetric)
{
    std::vector<std::unique_ptr<Evaluation::EvaluationMetric>> metrics;
    metrics.push_back(std::make_unique<Evaluation::ScoreMetric>(2.0));
    metrics.push_back(std::make_unique<Evaluation::ScoreMetric>(3.0));

    Evaluation::EvaluationRun eval(std::move(metrics));

    ASSERT_EQ(eval.getSize(), 2) << "Size is wrong";
    ASSERT_EQ(eval.getMetrics().size(), 2.0) << "Size is wrong";
    ASSERT_TRUE(dynamic_cast<const Evaluation::ScoreMetric*>(&eval.getMetricAt(0)) != nullptr) << "Metric should be scoreMetric";
    ASSERT_TRUE(dynamic_cast<const Evaluation::ScoreMetric*>(&eval.getMetricAt(1)) != nullptr) << "Metric should be scoreMetric";
    const Evaluation::ScoreMetric* scoreMetric1 = dynamic_cast<const Evaluation::ScoreMetric*>(&eval.getMetricAt(0));
    const Evaluation::ScoreMetric* scoreMetric2 = dynamic_cast<const Evaluation::ScoreMetric*>(&eval.getMetricAt(1));
    ASSERT_EQ(scoreMetric1->getScore(), 2.0) << "Score is wrong";
    ASSERT_EQ(scoreMetric2->getScore(), 3.0) << "Score is wrong"; 

    ASSERT_THROW(eval.getMetricAt(2), std::runtime_error) << "Should fail out of bound";

    std::unique_ptr<Evaluation::ScoreMetric> metric = std::make_unique<Evaluation::ScoreMetric>(1.0);
    ASSERT_NO_THROW(eval.addMetric(std::move(metric))) << "Adding a metric failed";

    ASSERT_EQ(eval.getSize(), 3) << "Size is wrong";
    ASSERT_TRUE(dynamic_cast<const Evaluation::ScoreMetric*>(&eval.getMetricAt(2)) != nullptr) << "Metric should be scoreMetric";
    const Evaluation::ScoreMetric* scoreMetric3 = dynamic_cast<const Evaluation::ScoreMetric*>(&eval.getMetricAt(2));
    ASSERT_EQ(scoreMetric3->getScore(), 1.0) << "Score is wrong";
}