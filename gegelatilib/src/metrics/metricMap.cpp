#include "metrics/metricMap.h"

void Metrics::MetricMap::resetCurrentMetrics()
{
    this->currentMetrics.clear();
    for(const auto& [hash, metric]: this->metricTemplate->emptyMetrics) {
        this->currentMetrics.insert(std::make_pair(hash, metric->cloneEmptyPtr()));
    }
}

const Metrics::MetricMapTemplate& Metrics::MetricMap::getTemplate() const
{
    return *this->metricTemplate;
}


void Metrics::MetricMap::merge(MetricMap& other)
{
    for(auto& [featureHash, otherFeatureMap]: other.metrics) {
        // Feature unknown, add the feature;
        if(this->metrics.find(featureHash) == this->metrics.end()) {
            this->metrics.insert(std::make_pair(featureHash, std::move(otherFeatureMap)));
        
        // Feature known, add only unknown metrics
        } else {
            auto& featureMap = this->metrics.at(featureHash);
            for(auto& [metricHash, metric]: otherFeatureMap) {
                // Metric unknown, add it
                if(featureMap.find(metricHash) == featureMap.end()) {
                    featureMap.insert(std::make_pair(metricHash, std::move(metric)));
                } else {
                    // Override old value (todo)
                    featureMap[metricHash] = std::move(metric);
                }
            }
        }
    }
}

void Metrics::MetricMap::extractBeforeExecution(
    const uint64_t& hash,
    const Individual& individual,
    const std::vector<Data::DataValue>& inputs,
    const Evaluation::Problem& problem)
{
    for(const auto& [key, metric]: this->currentMetrics) {
      metric->extractBeforeExecution(hash, individual, inputs, problem);
    }
}

/// Extract for all metrics
void Metrics::MetricMap::extractAfterExecution(
    const uint64_t& hash,
    const Individual& individual, 
    const Data::DataValue& output,
    const Evaluation::Problem& problem)
{
    for(const auto& [key, metric]: this->currentMetrics) {
      metric->extractAfterExecution(hash, individual, output, problem);
    }
}

/// Extract for all metrics
void Metrics::MetricMap::extractionEnd(
    const uint64_t& hash,
    const Individual& individual,
    const Evaluation::Problem& problem)
{
    for(const auto& [key, metric]: this->currentMetrics) {
      metric->extractionEnd(hash, individual, problem);
    }
    this->metrics.insert(std::make_pair(hash, std::move(this->currentMetrics)));
    this->resetCurrentMetrics();
}


std::set<uint64_t> Metrics::MetricMapTemplate::getMetricHash() const
{
    std::set<uint64_t> hashes;
    for(const auto& [hash, metric]: this->emptyMetrics) {
        hashes.insert(hash);
    }
    return hashes;
}

bool Metrics::MetricMapTemplate::hasMetricHash(uint64_t key) const
{
    return this->emptyMetrics.find(key) != this->emptyMetrics.end();
}

size_t Metrics::MetricMapTemplate::size() const
{
    return this->emptyMetrics.size();
}