

#ifndef SCORE_METRICS_H
#define SCORE_METRICS_H

#include "evaluation/evaluationMetric.h"


namespace Evaluation {
    /**
     * \brief Class to extract any metrics from either the individual or the environment.
     */
    class ScoreMetric : public EvaluationMetric
    {
      protected:
        /**
         * Score obtained by the individual at the end of an episode
         */
        double score = 0;

      public:
        /**
         * \brief Default constructor
         */
        ScoreMetric(): EvaluationMetric() {};

        /**
         * \brief Constructor with score.
         *
         * \param[in] score the score obtained by the individual.
         */
        ScoreMetric(double score)
            : EvaluationMetric(), score{score} {};

        /**
         * \brief Method to dupplicate the current polymorphic metric, keeping the current parameter but forgetting the extractions.
         */
        virtual std::unique_ptr<EvaluationMetric> cloneEmptyUniquePtr() const override;

        /**
         * Return the score of the individual.
         */
        virtual double getScore() const;


        /**
         * \brief Extract metrics from the individual in the learning environment.
         *
         * This method is called at the end of every episode of the environment
         * evaluation.
         *
         * \param[in] individual the individual performing the run.
         * \param[in] nbStepsExecuted number of steps executed at the end of the
         * episode.
         * \param[in] learningEnvironment the learning environment in
         * which the individual is evaluated.
         */
        virtual void extractMetricsRun(
            const Evolution::Individual& individual, size_t nbStepsExecuted,
            const Learn::LearningEnvironment& learningEnvironment) override;
    };


}; // namespace Evaluation

#endif // EVALUATION_METRICS_H