

#ifndef EVALUATIONS_REWARD_FEATURE_H
#define EVALUATIONS_REWARD_FEATURE_H

#include "evaluations/feature.h"
#include "reinforcementProblem.h"



namespace Evaluations {

    /**
     * \brief Abstract class to extract any metrics from either the individual or the environment during an evaluation run.
     */
    class RewardFeature: public Feature
    {
      protected:
        /// @brief List of current rewards to store an episode
        std::vector<double> currentRewards;

        /// @brief BOolean indicating if a new episode is reached.
        bool newEpisode = true;

      public:
        /**
         * \brief Default constructor
         */
        RewardFeature() {};

        /**
         * \brief Extract metrics from the individual in the problem.
         *
         * This method is called at every step of the evaluation, before execution.
         *
         * \param[in] hash unique hash identifying the evaluation.
         * \param[in] individual the individual performing a step.
         * \param[in] inputs inputs of the problem
         * \param[in] problem the problem in which the individual is evaluated.
         */
        virtual void extractBeforeExecution(
            const uint64_t& hash,
            const Individual& individual,
            const std::vector<Data::DataView>& inputs,
            const Problem& problem) override;

        /**
         * \brief Print the content of the metric.
         */
        virtual std::string toString(std::string prefix = "") const override;
    };


}; // namespace Evaluation

#endif // EVALUATIONS_REWARD_FEATURE_H