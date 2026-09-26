

#ifndef EVALUATIONS_FEAULT_CLASSIF_METRIC_H
#define EVALUATIONS_FEAULT_CLASSIF_METRIC_H

#include "evaluations/metric.h"
#include "evaluations/predictionFeature.h"




namespace Evaluations {
    /**
     * \brief Abstract class to extract any metrics from either the individual or the environment during an evaluation run.
     */
    class DefaultClassifMetric: public Metric
    {
      protected:

        /// Constant hash for the prediction feature.
        uint64_t predictionFeatureHash;

      public:
        /**
         * \brief Default constructor
         */
        DefaultClassifMetric() : predictionFeatureHash{std::make_unique<PredictionFeature>()->hash()} {};

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

#endif // EVALUATIONS_FEAULT_CLASSIF_METRIC_H