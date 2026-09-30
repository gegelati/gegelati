#include "metrics/scalarMetric.h"

double Metrics::ScalarMetric::getValue() const {
    return this->values[0];
}

const Dimensions::NumericRange<double>& Metrics::ScalarMetric::getRange() const
{
    return this->ranges[0];
}