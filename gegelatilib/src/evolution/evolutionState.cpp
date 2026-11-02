#include "evolution/evolutionState.h"

#include "selection/selector.h"

std::unordered_map<uint64_t, std::shared_ptr<const Individual>> Evolution::makeIndex(const std::vector<std::shared_ptr<const Individual>>& individuals)
{
    std::unordered_map<uint64_t, std::shared_ptr<const Individual>> index;
    index.reserve(individuals.size());
    for(const std::shared_ptr<const Individual>& individual: individuals) {
        index.emplace(individual->getIndividualID(), individual);
    }
    return index;
}

void Evolution::EvolutionState::reset(uint64_t seed)
{
    this->generation = initGeneration;
    this->nbGenerationsDone = 0;
    this->elapsedSeconds = 0.0;
    this->rng.setSeed(seed);
    this->stepTimes.clear();
    this->entries.clear();
}

std::vector<std::shared_ptr<const Individual>> Evolution::EvolutionState::getIndividuals(const std::string& name) const
{
    auto it = this->entries.find(name);
    if(it == this->entries.end()) {
        return {};
    }
    if(const auto* mutableIndividuals = std::any_cast<std::shared_ptr<std::vector<std::shared_ptr<Individual>>>>(&it->second.value)) {
        return std::vector<std::shared_ptr<const Individual>>((*mutableIndividuals)->begin(), (*mutableIndividuals)->end());
    }
    return *this->find<std::vector<std::shared_ptr<const Individual>>>(name);
}

std::shared_ptr<const Individual> Evolution::EvolutionState::bestIndividual(const std::string& individuals, const std::string& fitness) const
{
    const std::vector<std::shared_ptr<const Individual>> ranked = this->getIndividuals(individuals);
    const std::shared_ptr<const std::map<uint64_t, double>> fitnessMap = this->getIfAny<std::map<uint64_t, double>>(fitness);
    if(ranked.empty() || fitnessMap == nullptr || fitnessMap->empty()) {
        return nullptr;
    }

    const std::unordered_map<uint64_t, std::shared_ptr<const Individual>> index = makeIndex(ranked);
    for(const auto& [id, value]: Selection::Selector::getOrderedFitness(*fitnessMap, false)) {
        auto found = index.find(id);
        if(found != index.end()) {
            return found->second;
        }
    }
    return nullptr;
}

bool Evolution::EvolutionState::has(const std::string& name) const
{
    return this->entries.find(name) != this->entries.end();
}

void Evolution::EvolutionState::erase(const std::string& name)
{
    this->entries.erase(name);
}

bool Evolution::EvolutionState::isUpToDate(const std::string& name) const
{
    auto it = this->entries.find(name);
    return it != this->entries.end() && it->second.generation == this->generation;
}
