#ifndef AVERAGE_VECTOR_DESCRIPTOR_H
#define AVERAGE_VECTOR_DESCRIPTOR_H

#include "fitnessAssignment/noveltyDescriptors/descriptor.h"
#include "metrics/vectorMetric.h"

namespace Descriptors {

    /**
     * \brief Descriptor averaging a vector metric over the features: the descriptor has the dimension of the vector.
     *
     * All the features must have a vector of the same size.
     *
     * Example: the metric is a vector of 2 values (e.g. an OutputMetric with 2 scalar outputs per feature), and there are 3 features
     * (e.g. 3 images):
     * \code
     *   feature 0: [1.0, 4.0]
     *   feature 1: [3.0, 2.0]
     *   feature 2: [5.0, 0.0]
     *   descriptor = [(1+3+5)/3, (4+2+0)/3] = [3.0, 2.0]   // dimension 2, the size of the vector
     * \endcode
     * The features are merged: the dimension does not depend on their number. With a metric of a single value, the descriptor is the
     * mean of this value over the features (dimension 1).
     */
    class AverageVectorDescriptor : public Descriptor {
      public:
        /// \brief Constructor, see Descriptor.
        explicit AverageVectorDescriptor(uint64_t metricHash) : Descriptor(metricHash) {};

        virtual std::vector<double> compute(Metrics::MetricMap& metrics) const override;

        /// The range of a value is the smallest range containing its range in every feature.
        virtual std::vector<Dimensions::NumericRange<double>> ranges(Metrics::MetricMap& metrics) const override;
    };

}; // namespace Descriptors

#endif // AVERAGE_VECTOR_DESCRIPTOR_H
