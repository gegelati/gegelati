

#ifndef SELECTION_METRICS_H
#define SELECTION_METRICS_H

#include "learn/learningEnvironment.h"
#include "tpg/tpgGraph.h"

namespace Selector {
    /**
     * \brief Class to extract metrics from either the agent or the environment.
     *
     * This metrics can be used to specify the selection of the selector.
     * This class does not implement any metrics, it need to be override by the
     * different selection methods.
     */
    class SelectionMetrics
    {
      protected:
        /**
         * Score obtained by the agent at the end of an episode
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
         * \param[in] score the score obtained by the agent.
         * \param[in] utility the utility obtained by the agent.
         */
        SelectionMetrics(double score, double utility = 0)
            : score{score}, utility{utility} {};

        /**
         * Return the score of the agent.
         */
        virtual double getScore() const;

        /**
         * Return the utility of the agent.
         */
        virtual double getUtility() const;

        /**
         * \brief Init the metrics for the agent in the learning environment.
         *
         * This method is called at the beginning of the evaluateJob method.
         *
         * \param[in] agent the TPGVertex representing the agent.
         * \param[in] learningEnvironment the learning environment in which the
         * agent is evaluated.
         */
        virtual void initMetrics(
            const TPG::TPGVertex* agent,
            const Learn::LearningEnvironment& learningEnvironment) {
            /* Empty because sub-class does not need to inherrit from it.*/
        };

        /**
         * \brief Extract metrics from the agent in the learning environment.
         *
         * This method is called at every step of the environment evaluation.
         *
         * \param[in] agent the TPGVertex representing the agent.
         * \param[in] actionValues the action values taken by the agent.
         * \param[in] learningEnvironment the learning environment in which the
         * agent is evaluated.
         */
        virtual void extractMetricsStep(
            const TPG::TPGVertex* agent, std::vector<double> actionValues,
            const Learn::LearningEnvironment& learningEnvironment) {
            /* Empty because sub-class does not need to inherrit from it.*/
        };

        /**
         * \brief Extract metrics from the agent in the learning environment.
         *
         * This method is called at the end of every episode of the environment
         * evaluation.
         *
         * \param[in] agent the TPGVertex representing the agent.
         * \param[in] nbStepsExecuted number of steps executed at the end of the
         * episode.
         * \param[in] learningEnvironment the learning environment in
         * which the agent is evaluated.
         */
        virtual void extractMetricsEpisode(
            const TPG::TPGVertex* agent, size_t nbStepsExecuted,
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

        /// @brief Comparison function to enable sorting of SelectionMetrics
        /// with STL.
        friend bool operator<(const SelectionMetrics& lhs,
                              const SelectionMetrics& rhs)
        {
            return lhs.getScore() < rhs.getScore();
        }

        /**
         * \brief Perform a weighted sum between 2 values.
         *
         * \param[in] value the value to combine.
         * \param[in] valueOther the other value to combine.
         * \param[in] nbEvaluation the number of evaluations to obtain value.
         * \param[in] nbEvaluationOther the number of evaluations to obtain
         * valueOther.
         */
        template <class T>
        static T weightedSum(T value, T valueOther, size_t nbEvaluation,
                                  size_t nbEvaluationOther)
        {
            value = value * (T)nbEvaluation + valueOther * (T)nbEvaluationOther;
            value /= (T)(nbEvaluation + nbEvaluationOther);
            return value;
        }
    };

    /**
     * \brief Comparison function to enable sorting of SelectionMetrics with
     * STL.
     */
    bool operator<(std::shared_ptr<SelectionMetrics> a,
                   std::shared_ptr<SelectionMetrics> b);

}; // namespace Selector

#endif // SELECTION_METRICS_H