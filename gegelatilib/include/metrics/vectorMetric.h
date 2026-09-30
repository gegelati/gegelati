

#ifndef VECTOR_METRIC_H
#define VECTOR_METRIC_H

#include "metrics/metric.h"
#include "dimensions/numericRange.h"

namespace Metrics {

    /**
     * \brief Abstract class of metric specifically extracting a vector of values stored as a double.
     */
    class VectorMetric: public Metric
    {
      protected:

        /// @brief Values of the metric
        std::vector<double> values;

        /// Range of the values
        std::vector<Dimensions::NumericRange<double>> ranges;

      public:
        /**
         * \brief Default constructor
         * 
         * \param[in] ranges the ranges for each of the values. (different for each)
         */
        VectorMetric(const std::vector<Dimensions::NumericRange<double>>& ranges) 
        : values(ranges.size(), 0), ranges{ranges} {};

        /**
         * \brief Default constructor
         * 
         * \param[in] nbValues the number of values in the vector
         * \param[in] range the range for each of the values. (same for each)
         */
        VectorMetric(size_t nbValues, const Dimensions::NumericRange<double>& range = Dimensions::NumericRange<double>::unbounded()) 
        : values(nbValues, 0), ranges(nbValues, range) {};

        /**
         * \brief Return the values of the metrics
         */
        virtual const std::vector<double>& getValues() const;

        /**
         * \brief Return the range of the values
         */
        virtual const std::vector<Dimensions::NumericRange<double>>& getRanges() const; 
    };

}; // namespace Metrics

#endif // VECTOR_METRIC_H