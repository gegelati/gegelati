
#include "evaluation/metric.h"

 std::set<uint64_t> Evaluation::Metric::getKeys() const
 {
    std::set<uint64_t> keys;
    for(const auto& pair: this->metrics) {
        keys.insert(pair.first);
    }
    return keys;
 }

bool Evaluation::Metric::hasKey(size_t key) const
{
    return this->metrics.find(key) != this->metrics.end();
}


const Data::DataValue& Evaluation::Metric::getMetricAt(size_t key) const
{
    if(!this->hasKey(key)) {
        throw std::runtime_error("Evaluation::Metric::getMetricAt: key not found in the metric map");
    }
    return this->metrics.at(key);
}


void Evaluation::Metric::merge(Metric& other)
{
    std::set<uint64_t> keys(other.getKeys());
    for (const uint64_t& key: keys) {
        this->metrics.insert(std::make_pair(key, std::move(other.metrics.at(key))));
    }
}

 size_t Evaluation::Metric::size() const
 {
    return this->metrics.size();
 }

uint64_t Evaluation::Metric::hash() const noexcept
{
    return std::type_index(typeid(*this)).hash_code();
}

void Evaluation::MetricMap::merge(MetricMap& other)
{
    for(auto& [key, metric]: other.metrics) {
        if(this->metrics.find(key) == this->metrics.end()) {
            this->metrics.insert(std::make_pair(key, std::move(metric)));
        } else {
            this->metrics.at(key)->merge(*metric);
        }
    }
}

/// Clone the metric map by cloning the contained metrics
std::unique_ptr<Evaluation::MetricMap> Evaluation::MetricMap::clone() const
{
    std::map<uint64_t, std::unique_ptr<Metric>> mapCopy;
    for(auto& [key, metric]: this->metrics) { 
        mapCopy.insert(std::make_pair(key, metric->cloneEmptyPtr()));
    }
    return std::make_unique<MetricMap>(mapCopy);
}

void Evaluation::MetricMap::extractBeforeExecution(
    const uint64_t& hash,
    const Individual& individual,
    const std::vector<Data::DataView>& inputs,
    const Problem& problem)
{
    for(const auto& [key, metric]: this->metrics) {
      metric->extractBeforeExecution(hash, individual, inputs, problem);
    }
}

/// Extract for all metrics
void Evaluation::MetricMap::extractAfterExecution(
    const uint64_t& hash,
    const Individual& individual, 
    const Data::DataValue& output,
    const Problem& problem)
{
    for(const auto& [key, metric]: this->metrics) {
      metric->extractAfterExecution(hash, individual, output, problem);
    }
}

/// Extract for all metrics
void Evaluation::MetricMap::extractionEndAndComputeMetric(
    const uint64_t& hash,
    const Individual& individual,
    const Problem& problem)
{
    for(const auto& [key, metric]: this->metrics) {
      metric->extractionEndAndComputeMetric(hash, individual, problem);
    }
}