#include "evolution/evolutionData.h"

#include <algorithm>
#include <sstream>
#include <vector>

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

std::string Evolution::EvolutionData::summary() const
{
    std::ostringstream result;
    result << "Data after " << this->nbGenerationsDone << " generation(s):" << std::string(1, 10);
    if(this->data.empty()) {
        result << "  (empty)" << std::string(1, 10);
        return result.str();
    }

    std::vector<std::vector<std::string>> rows;
    // Columns: name, type, content, age. All but the last are padded to align.
    std::vector<size_t> widths(3, 0);
    constexpr size_t maxTypeSize = 70;
    for(const auto& [name, entry]: this->data) {
        std::string age = "stored at generation " + std::to_string(entry->generation());
        if(entry->generation() == this->generation) {
            age = "up to date";
        } else if(entry->generation() == initGeneration) {
            age = "stored at initialization";
        }
        std::string type = Data::demangle(entry->type().name());
        if(type.size() > maxTypeSize) {
            type = type.substr(0, maxTypeSize - 3) + "...";
        }
        rows.push_back({name, type, entry->describe(), age});
        for(size_t idx = 0; idx < widths.size(); idx++) {
            widths[idx] = std::max(widths[idx], rows.back()[idx].size());
        }
    }
    for(const std::vector<std::string>& row: rows) {
        result << " ";
        for(size_t idx = 0; idx < row.size(); idx++) {
            result << " " << row[idx] << ((idx < widths.size()) ? std::string(widths[idx] - row[idx].size(), ' ') : "") << " ";
        }
        result << std::string(1, 10);
    }
    return result.str();
}
