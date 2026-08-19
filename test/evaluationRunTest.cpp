
#include <gtest/gtest.h>
#include <stdexcept>

#include "evaluation/evaluationRun.h"



TEST(EvaluationRunTest, Constructor)
{
    Evaluation::EvaluationRun* eval1;
    Evaluation::EvaluationRun* eval2;
    Evaluation::EvaluationRun* eval3;

    std::unique_ptr<Evaluation::EvaluationMetric> metric1 = std::make_unique<Evaluation::EvaluationMetric>(1.0);
    std::vector<std::unique_ptr<Evaluation::EvaluationMetric>> metrics;
    metrics.push_back(std::make_unique<Evaluation::EvaluationMetric>(2.0));
    metrics.push_back(std::make_unique<Evaluation::EvaluationMetric>(3.0));

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
    metrics.push_back(std::make_unique<Evaluation::EvaluationMetric>(2.0));
    metrics.push_back(std::make_unique<Evaluation::EvaluationMetric>(3.0));

    Evaluation::EvaluationRun eval(std::move(metrics));

    ASSERT_EQ(eval.getSize(), 2) << "Size is wrong";
    ASSERT_EQ(eval.getMetrics().size(), 2.0) << "Size is wrong"; 
    ASSERT_EQ(eval.getMetricAt(0).getScore(), 2.0) << "Score is wrong";
    ASSERT_EQ(eval.getMetricAt(1).getScore(), 3.0) << "Score is wrong"; 

    ASSERT_THROW(eval.getMetricAt(2), std::runtime_error) << "Should fail out of bound";

    std::unique_ptr<Evaluation::EvaluationMetric> metric = std::make_unique<Evaluation::EvaluationMetric>(1.0);
    ASSERT_NO_THROW(eval.addMetric(std::move(metric))) << "Adding a metric failed";

    ASSERT_EQ(eval.getSize(), 3) << "Size is wrong";
    ASSERT_EQ(eval.getMetricAt(2).getScore(), 1.0) << "Score is wrong";
}