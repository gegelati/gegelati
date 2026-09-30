

#ifndef REWARD_METRICS_H
#define REWARD_METRICS_H

#include "metrics/scalarMetric.h"
#include "evaluation/reinforcementProblem.h"



namespace Metrics {

    /**
     * \brief Abstract class to extract any metrics from either the individual or the environment during an evaluation run.
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
         * \brief if currentHash == Hash, extract reward, else set currentHash to hash and return (we don't extract reward at step 0)
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
         * \brief return the static hash of a prediction metric (equal for all prediction metric since there is not hyperparameters)
         */
        static uint64_t staticHash();
    };


}; // namespace Metrics

#endif // REWARD_METRICS_H