

#ifndef EVALUATIONS_TOTAL_REWARD_METRIC_H
#define EVALUATIONS_TOTAL_REWARD_METRIC_H

#include "evaluations/metric.h"
#include "evaluations/rewardFeature.h"




namespace Evaluations {
    /**
     * \brief Abstract class to extract any metrics from either the individual or the environment during an evaluation run.
     */
    class TotalRewardMetric: public Metric
    {
      protected:

        /// Constant hash for the reward feature.
        uint64_t rewardFeatureHash;

      public:
        /**
         * \brief Default constructor
         */
        TotalRewardMetric() : rewardFeatureHash{std::make_unique<RewardFeature>()->hash()} {};

        /**
         * \brief returns a map of the requested features, with the corresponding hash as key
         */
        virtual std::map<size_t, std::unique_ptr<Feature>> getRequestedFeatures() const override;

        /**
         * \brief Extract metrics from the individual in the learning environment.
         *
         * This method is called at every step of the environment evaluation.
         *
         * \param[in] features
         */
        virtual Data::DataValue computeMetrics(const std::map<size_t, std::unique_ptr<Feature>>& features) const override;
    };

}; // namespace Evaluation

#endif // EVALUATIONS_TOTAL_REWARD_METRIC_H