#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include <algorithm>
#include <cstdint>
#include <vector>

#include "dimensions/numericRange.h"
#include "metrics/metricMap.h"
#include "util/named.h"

namespace Descriptors {

    /**
     * \brief Base class of the functions mapping the metrics of an individual to descriptor values,
     * the point of the behavior space used by novelty search and, later, by quality diversity methods.
     *
     * A descriptor knows the hash of the metric it reads. Its ranges are derived from the ranges of this metric,
     * so they can be unbounded. The dimension of the descriptor is the number of ranges. It depends on the metrics
     * (e.g. the number of features), so it is given for a metric map.
     */
    class Descriptor : public Util::Named {
      protected:
        /// \brief Hash of the metric the descriptor is computed from.
        uint64_t metricHash;

        /**
         * \brief Smallest range containing two ranges: a bound is missing if it is missing in one of them.
         */
        static Dimensions::NumericRange<double> hull(const Dimensions::NumericRange<double>& a, const Dimensions::NumericRange<double>& b) {
            Dimensions::NumericRange<double> result;
            if(a.minimum && b.minimum) {
                result.minimum = std::min(*a.minimum, *b.minimum);
            }
            if(a.maximum && b.maximum) {
                result.maximum = std::max(*a.maximum, *b.maximum);
            }
            return result;
        }

      public:
        /**
         * \param[in] metricHash the hash of the metric the descriptor is computed from.
         * \param[in] name name of the descriptor, only used to print it (see Util::Named).
         */
        explicit Descriptor(uint64_t metricHash, std::string name = "Descriptor") : Util::Named(std::move(name)), metricHash{metricHash} {};

        /// Default polymorphic destructor
        virtual ~Descriptor() = default;

        /// \brief Return the hash of the metric the descriptor is computed from.
        uint64_t getMetricHash() const { return this->metricHash; }

        /**
         * \brief Compute the descriptor values of one individual.
         *
         * \param[in] metrics the metrics of the individual.
         * \return the descriptor values.
         */
        virtual std::vector<double> compute(Metrics::MetricMap& metrics) const = 0;

        /**
         * \brief Return the range of each descriptor value, possibly unbounded.
         *
         * \param[in] metrics the metrics of an individual.
         */
        virtual std::vector<Dimensions::NumericRange<double>> ranges(Metrics::MetricMap& metrics) const = 0;

        /**
         * \brief Return the number of descriptor values.
         *
         * \param[in] metrics the metrics of an individual.
         */
        size_t dimension(Metrics::MetricMap& metrics) const { return this->ranges(metrics).size(); }
    };

}; // namespace Descriptors

#endif // DESCRIPTOR_H
