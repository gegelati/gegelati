#ifndef SCALAR_LIST_DESCRIPTOR_H
#define SCALAR_LIST_DESCRIPTOR_H

#include "fitnessAssignment/noveltyDescriptors/descriptor.h"
#include "metrics/scalarMetric.h"

namespace Descriptors {

    /**
     * \brief Descriptor listing a scalar metric over the features: the dimension is the number of features.
     *
     * Example: the metric is a single value, and there are 3 features (e.g. 3 images):
     * \code
     *   feature 0: 0.2
     *   feature 1: 0.7
     *   feature 2: 0.1
     *   descriptor = [0.2, 0.7, 0.1]   // dimension 3, the number of features
     * \endcode
     * Each feature is its own dimension, so two individuals are close only if they have close values on every feature.
     * Unlike AverageVectorDescriptor, the features are not merged: the number of features must be the same for all the individuals.
     */
    class ScalarListDescriptor : public Descriptor {
      public:
        /**
         * \brief Constructor.
         *
         * \param[in] metricHash the hash of the metric the descriptor is computed from.
         * \param[in] name name of the descriptor, only used to print it (e.g. in the summary of an evolution).
         */
        explicit ScalarListDescriptor(uint64_t metricHash, std::string name = "ScalarListDescriptor") : Descriptor(metricHash, std::move(name)) {};

        virtual std::vector<double> compute(Metrics::MetricMap& metrics) const override;

        /// The ranges are the ranges of the metric of each feature.
        virtual std::vector<Dimensions::NumericRange<double>> ranges(Metrics::MetricMap& metrics) const override;
    };

}; // namespace Descriptors

#endif // SCALAR_LIST_DESCRIPTOR_H
