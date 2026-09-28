
#include "metrics/metric.h"

uint64_t Metrics::Metric::hash() const noexcept
{
    return std::type_index(typeid(*this)).hash_code();
}
