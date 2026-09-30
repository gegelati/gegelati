#include "metrics/vectorMetric.h"

const std::vector<double>& Metrics::VectorMetric::getValues() const {
    return this->values;
}

const std::vector<Dimensions::NumericRange<double>>& Metrics::VectorMetric::getRanges() const
{
    return this->ranges;
}