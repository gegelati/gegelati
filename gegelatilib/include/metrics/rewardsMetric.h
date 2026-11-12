

#ifndef REWARD_METRICS_H
#define REWARD_METRICS_H

#include "metrics/scalarMetric.h"
#include "evaluation/reinforcementProblem.h"



namespace Metrics {

    /**
     * \brief Metric summing the rewards given by a reinforcement environment during an evaluation.
     */
    class RewardsMetric: public ScalarMetric
    {
      protected:

      public:
        /**
         * \brief Default constructor
         */
        RewardsMetric() {};

        /**
         * \brief clone returning RewardsMetric
         */
        virtual std::unique_ptr<Metric> cloneEmptyPtr() const override;

        /**
         * \brief Add the reward of the last action, given by the environment before each step.
         *
         * \param[in] hash unique hash identifying the evaluation.
         * \param[in] individual the individual performing a step.
         * \param[in] inputs inputs of the problem
         * \param[in] problem the problem in which the individual is evaluated.
         */
        virtual void extractBeforeExecution(
            const uint64_t& hash,
            const Individual& individual,
            const std::vector<Data::DataValue>& inputs,
            const Evaluation::Problem& problem) override;

        /**
         * \brief Extract the final reward and insert the score.
         *
         * \param[in] hash unique hash identifying the evaluation.
         * \param[in] individual the individual performing a step.
         * \param[in] problem the problem in which the individual is evaluated.
         */
        virtual void extractionEnd(
            const uint64_t& hash,
            const Individual& individual,
            const Evaluation::Problem& problem) override;

        /**
         * \brief Print the content of the metric.
         */
        virtual std::string toString(std::string prefix = "") const override;

        /**
         * \brief return the static hash of a rewards metric (equal for all rewards metrics since there is no hyperparameter)
         */
        static uint64_t staticHash();
    };


}; // namespace Metrics

#endif // REWARD_METRICS_H