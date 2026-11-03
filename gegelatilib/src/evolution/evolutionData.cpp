#include "evolution/evolutionData.h"

#include <algorithm>
#include <cmath>
#include <sstream>

std::unordered_map<uint64_t, std::shared_ptr<const Individual>> Evolution::makeIndex(const std::vector<std::shared_ptr<const Individual>>& individuals)
{
    std::unordered_map<uint64_t, std::shared_ptr<const Individual>> index;
    index.reserve(individuals.size());
    for(const std::shared_ptr<const Individual>& individual: individuals) {
        index.emplace(individual->getIndividualID(), individual);
    }
    return index;
}

void Evolution::EvolutionData::reset(uint64_t seed)
{
    this->generation = initGeneration;
    this->nbGenerationsDone = 0;
    this->rng.setSeed(seed);
    this->data.clear();
    this->initializedSteps.clear();
}

std::set<std::string> Evolution::EvolutionData::dataNames() const
{
    std::set<std::string> names;
    for(const auto& entry: this->data) {
        names.insert(entry.first);
    }
    return names;
}

std::vector<std::shared_ptr<const Individual>> Evolution::EvolutionData::getIndividuals(const Key<std::vector<std::shared_ptr<Individual>>>& key) const
{
    const std::shared_ptr<const std::vector<std::shared_ptr<Individual>>> individuals = this->getIfAny(key);
    if(individuals == nullptr) {
        return {};
    }
    return std::vector<std::shared_ptr<const Individual>>(individuals->begin(), individuals->end());
}

std::vector<std::shared_ptr<const Individual>> Evolution::EvolutionData::getIndividuals(const std::vector<Key<std::vector<std::shared_ptr<Individual>>>>& keys) const
{
    std::vector<std::shared_ptr<const Individual>> individuals;
    for(const Key<std::vector<std::shared_ptr<Individual>>>& key: keys) {
        const std::vector<std::shared_ptr<const Individual>> list = this->getIndividuals(key);
        individuals.insert(individuals.end(), list.begin(), list.end());
    }
    return individuals;
}

std::shared_ptr<const Individual> Evolution::EvolutionData::bestIndividual(const Key<std::vector<std::shared_ptr<Individual>>>& individuals,
                                                                            const Key<std::map<uint64_t, double>>& fitness) const
{
    const std::shared_ptr<const std::map<uint64_t, double>> fitnessMap = this->getIfAny(fitness);
    if(fitnessMap == nullptr) {
        return nullptr;
    }

    // The highest fitness wins, then the lowest ID. Individuals without fitness are ignored.
    std::shared_ptr<const Individual> best = nullptr;
    double bestFitness = 0.0;
    for(const std::shared_ptr<const Individual>& individual: this->getIndividuals(individuals)) {
        const auto found = fitnessMap->find(individual->getIndividualID());
        if(found == fitnessMap->end() || std::isnan(found->second)) {
            continue;
        }
        if(best == nullptr || found->second > bestFitness || (found->second == bestFitness && individual->getIndividualID() < best->getIndividualID())) {
            best = individual;
            bestFitness = found->second;
        }
    }
    return best;
}

std::string Evolution::EvolutionData::summary() const
{
    std::ostringstream result;
    result << "Data after " << this->nbGenerationsDone << " generation(s):" << std::string(1, 10);
    if(this->data.empty()) {
        result << "  (empty)" << std::string(1, 10);
        return result.str();
    }

    std::vector<std::vector<std::string>> rows;
    std::vector<size_t> widths(2, 0);
    for(const auto& [name, entry]: this->data) {
        std::string age = "stored at generation " + std::to_string(entry.generation);
        if(entry.generation == this->generation) {
            age = "up to date";
        } else if(entry.generation == initGeneration) {
            age = "stored at initialization";
        }
        rows.push_back({name, (entry.describe == nullptr) ? "" : entry.describe(), age});
        widths[0] = std::max(widths[0], rows.back()[0].size());
        widths[1] = std::max(widths[1], rows.back()[1].size());
    }
    for(const std::vector<std::string>& row: rows) {
        result << "  " << row[0] << std::string(widths[0] - row[0].size(), ' ') << "  " << row[1] << std::string(widths[1] - row[1].size(), ' ') << "  " << row[2]
               << std::string(1, 10);
    }
    return result.str();
}
