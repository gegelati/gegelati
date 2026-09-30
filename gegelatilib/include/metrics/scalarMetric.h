

#ifndef SCALAR_METRICS_H
#define SCALAR_METRICS_H

#include "metrics/vectorMetric.h"

namespace Metrics {

    /**
     * \brief Abstract class of metric specifically extracting a single scalar stored as a double.
     */
    class ScalarMetric: public VectorMetric
    {
      private:
        /// Hiding getValues and getRanges when using specifically a scalarMetric. However it is still accessible if doing something like VectorMetric& metric = scalarMetric.
        using VectorMetric::getValues;
        using VectorMetric::getRanges;

      public:
        /**
         * \brief Default constructor
         */
        ScalarMetric(const Dimensions::NumericRange<double>& range = Dimensions::NumericRange<double>::unbounded()) : VectorMetric(1, range) {};

        /**
         * \brief Return the score of the metric.s
         */
        virtual double getValue() const;

        /**
         * \brief Return the range of the value
         */
        virtual const Dimensions::NumericRange<double>& getRange() const; 
    };


}; // namespace Metrics

#endif // SCALAR_METRICS_H