
#include "evaluations/feature.h"

 std::set<size_t> Evaluations::Feature::getKeys() const
 {
    std::set<size_t> keys;
    for(const auto& pair: this->features) {
        keys.insert(pair.first);
    }
    return keys;
 }

bool Evaluations::Feature::hasKey(size_t key) const
{
    return this->features.find(key) != this->features.end();
}


const Data::DataValue& Evaluations::Feature::getFeatureAt(size_t key) const
{
    if(!this->hasKey(key)) {
        throw std::runtime_error("Evaluations::Feature::getFeatureAt: key not found in the feature map");
    }
    return this->features.at(key);
}


void Evaluations::Feature::merge(Feature& other)
{
    std::set<size_t> keys(other.getKeys());
    for (const size_t& key: keys) {
        this->features.insert(std::make_pair(key, std::move(other.features.at(key))));
    }
}

 size_t Evaluations::Feature::size() const
 {
    return this->features.size();
 }

uint64_t Evaluations::Feature::hash() const noexcept
{
    return std::type_index(typeid(*this)).hash_code();
}