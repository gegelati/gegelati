

#ifndef EVALUATION_METRICS_H
#define EVALUATION_METRICS_H

#include "evolution/individual.h"
#include "learn/learningEnvironment.h"


namespace Evaluation {
    /**
     * \brief Class to extract any metrics from either the individual or the environment.
     */
    class EvaluationMetric
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
        EvaluationMetric() = default;

        /**
         * \brief Constructor with score.
         *
         * \param[in] score the score obtained by the individual.
         */
        EvaluationMetric(double score)
            : score{score} {};

        /**
         * \brief Method to dupplicate the current polymorphic metric, keeping the current parameter but forgetting the extractions.
         */
        std::unique_ptr<EvaluationMetric> cloneEmptyUniquePtr() const;

        /**
         * Return the score of the individual.
         */
        virtual double getScore() const;

        /**
         * \brief Init the metrics for the individual in the learning environment.
         *
         * This method is called at the beginning of the evaluateJob method.
         *
         * \param[in] individual the individual representing the individual.
         * \param[in] learningEnvironment the learning environment in which the
         * individual is evaluated.
         * \param[in] seed used to reset the learningEnvironment.
         */
        virtual void initMetrics(
            const Evolution::Individual& individual,
            const Learn::LearningEnvironment& learningEnvironment,
            size_t seed) {
            /* Empty because sub-class does not need to inherrit from it.*/
        };

        /**
         * \brief Extract metrics from the individual in the learning environment.
         *
         * This method is called at every step of the environment evaluation.
         *
         * \param[in] individual the individual performing a step.
         * \param[in] actionValues the action values taken by the individual.
         * \param[in] learningEnvironment the learning environment in which the
         * individual is evaluated.
         */
        virtual void extractMetricsStep(
            const Evolution::Individual& individual, std::vector<double> actionValues,
            const Learn::LearningEnvironment& learningEnvironment) {
            /* Empty because sub-class does not need to inherrit from it.*/
        };

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
            const Learn::LearningEnvironment& learningEnvironment);
    };


}; // namespace Evaluation

#endif // EVALUATION_METRICS_H