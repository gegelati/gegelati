

#ifndef SELECTION_METRICS_H
#define SELECTION_METRICS_H

#include "representation/individual.h"
#include "learn/learningEnvironment.h"
#include "evoGraph/graph.h"


namespace Selector {
    /**
     * \brief Class to extract metrics from either the individual or the environment.
     *
     * This metrics can be used to specify the selection of the selector.
     * This class does not implement any metrics, it need to be override by the
     * different selection methods.
     */
    class SelectionMetrics
    {
      protected:
        /**
         * Score obtained by the individual at the end of an episode
         */
        double score = 0;

        /**
         * Double value for the utility. Utility is used only if the
         * LearningEnvironment override the specific method. The utility
         * represent a score needed for logs but not for learning.
         *
         * It can be used for comparison with different score rules for
         * example.
         */
        double utility = 0;

      public:
        /**
         * \brief Default constructor
         */
        SelectionMetrics() = default;

        /**
         * \brief Constructor with score and utility initialization.
         *
         * \param[in] score the score obtained by the individual.
         * \param[in] utility the utility obtained by the individual.
         */
        SelectionMetrics(double score, double utility = 0)
            : score{score}, utility{utility} {};

        /**
         * Return the score of the individual.
         */
        virtual double getScore() const;

        /**
         * Return the utility of the individual.
         */
        virtual double getUtility() const;

        /**
         * \brief Init the metrics for the individual in the learning environment.
         *
         * This method is called at the beginning of the evaluateJob method.
         *
         * \param[in] individual the individual representing the individual.
         * \param[in] learningEnvironment the learning environment in which the
         * individual is evaluated.
         */
        virtual void initMetrics(
            const Representation::Individual& individual,
            const Learn::LearningEnvironment& learningEnvironment) {
            /* Empty because sub-class does not need to inherrit from it.*/
        };

        /**
         * \brief Extract metrics from the individual in the learning environment.
         *
         * This method is called at every step of the environment evaluation.
         *
         * \param[in] individual the individual representing the individual.
         * \param[in] actionValues the action values taken by the individual.
         * \param[in] learningEnvironment the learning environment in which the
         * individual is evaluated.
         */
        virtual void extractMetricsStep(
            const Representation::Individual& individual, std::vector<double> actionValues,
            const Learn::LearningEnvironment& learningEnvironment) {
            /* Empty because sub-class does not need to inherrit from it.*/
        };

        /**
         * \brief Extract metrics from the individual in the learning environment.
         *
         * This method is called at the end of every episode of the environment
         * evaluation.
         *
         * \param[in] individual the individual representing the individual.
         * \param[in] nbStepsExecuted number of steps executed at the end of the
         * episode.
         * \param[in] learningEnvironment the learning environment in
         * which the individual is evaluated.
         */
        virtual void extractMetricsEpisode(
            const Representation::Individual& individual, size_t nbStepsExecuted,
            const Learn::LearningEnvironment& learningEnvironment);

        /**
         * \brief Perform a weighted sum between this SelectionMetrics and
         * another.
         *
         * \param[in] other the other SelectionMetrics to combine with this.
         * \param[in] nbEvaluation the number of evaluation used to obtain this
         * SelectionMetrics.
         * \param[in] nbEvaluationOther the number of
         * evaluation used to obtain the other SelectionMetrics.
         */
        virtual void weightedSum(std::shared_ptr<SelectionMetrics> other,
                                 size_t nbEvaluation, size_t nbEvaluationOther);
    };

    /**
     * \brief Comparison function to enable sorting of SelectionMetrics with
     * STL.
     */
    bool operator<(std::shared_ptr<SelectionMetrics> a,
                   std::shared_ptr<SelectionMetrics> b);

}; // namespace Selector

#endif // SELECTION_METRICS_H