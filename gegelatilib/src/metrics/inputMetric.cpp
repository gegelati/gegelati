
#include "metrics/inputMetric.h"

#include <algorithm>
#include <cstring>
#include <sstream>
#include <typeindex>


std::mutex Metrics::InputCache::mutex;
std::unordered_map<std::size_t, std::vector<std::weak_ptr<const std::vector<Data::DataValue>>>> Metrics::InputCache::cache;


std::vector<Data::DataValue> Metrics::InputCache::cloneInputs(const std::vector<Data::DataValue>& inputs)
{
    std::vector<Data::DataValue> result;
    result.reserve(inputs.size());
    for (const auto& input : inputs) {
        result.push_back(input.clone());
    }
    return result;
}

bool Metrics::InputCache::equalInputs(const std::vector<Data::DataValue>& lhs, const std::vector<Data::DataValue>& rhs)
{
    if (lhs.size() != rhs.size()) return false;
    for (std::size_t i = 0; i < lhs.size(); ++i) {
        if (lhs[i] != rhs[i]) return false;
    }
    return true;
}

std::shared_ptr<const std::vector<Data::DataValue>> Metrics::InputCache::intern(const std::vector<Data::DataValue>& inputs)
{
    const std::size_t hash = Data::DataValue::getCombinedHash(inputs);
    std::lock_guard<std::mutex> lock(mutex);
    auto& bucket = cache[hash];
    bucket.erase(std::remove_if(bucket.begin(), bucket.end(), [](const auto& entry) { return entry.expired(); }), bucket.end());
    for (const auto& entry : bucket) {
        if (auto existing = entry.lock()) {
            return existing;
        }
    }
    auto stored = std::make_shared<const std::vector<Data::DataValue>>(cloneInputs(inputs));
    bucket.emplace_back(stored);
    return stored;
}

std::unique_ptr<Metrics::Metric> Metrics::InputMetric::cloneEmptyPtr() const
{
    return std::make_unique<InputMetric>();
}

void Metrics::InputMetric::extractBeforeExecution(const uint64_t& hash, const Individual& individual, const std::vector<Data::DataValue>& inputs, const Evaluation::Problem& problem)
{
    (void)hash;
    (void)individual;
    (void)problem;
    this->inputs.push_back(InputCache::intern(inputs));
}

std::string Metrics::InputMetric::toString(std::string prefix) const
{
    std::stringstream ss;
    ss << prefix << "InputMetric with " << inputs.size() << " extractions:\n";
    for (std::size_t idx = 0; idx < inputs.size(); ++idx) {
        ss << prefix << "  Extraction " << idx << ":\n";
        for (std::size_t idxInput = 0; idxInput < inputs[idx]->size(); ++idxInput) ss << inputs[idx]->at(idxInput).toShortString(prefix + "    ") << ",\n";
    }
    return ss.str();
}

uint64_t Metrics::InputMetric::staticHash()
{
    return std::type_index(typeid(InputMetric)).hash_code();
}

const std::vector<std::shared_ptr<const std::vector<Data::DataValue>>>& Metrics::InputMetric::getInputs() const
{
    return inputs;
}

