

#ifndef EVALUATIONS_METRIC_H
#define EVALUATIONS_METRIC_H

#include <typeindex>
#include <map>

#include "evaluations/feature.h"



/// For include
class Individual;

namespace Evaluations {
    /**
     * \brief Abstract class to extract any metrics from either the individual or the environment during an evaluation run.
     */
    class Metric
    {
      protected:

      public:
        /**
         * \brief Default constructor
         */
        Metric() {};

        /**
         * \brief returns a map of the requested features, with the corresponding hash as key
         */
        virtual std::map<size_t, std::unique_ptr<Feature>> getRequestedFeatures() const = 0;

        /**
         * \brief Extract metrics from the individual in the learning environment.
         *
         * This method is called at every step of the environment evaluation.
         *
         * \param[in] features
         */
        virtual Data::DataValue computeMetrics(const std::map<size_t, std::unique_ptr<Feature>>& features) const = 0;


        /**
         * \brief from a list of metric, get all requested metric with returning only without double values.
         */
        static std::map<size_t, std::unique_ptr<Feature>> getUniqueRequestedMetrics(const std::vector<std::shared_ptr<const Metric>>& metrics);
    };


}; // namespace Evaluation

#endif // EVALUATION_METRICS_H