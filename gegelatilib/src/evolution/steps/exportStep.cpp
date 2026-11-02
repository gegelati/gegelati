#include "evolution/steps/exportStep.h"

#include <memory>
#include <vector>

#include "file/populationDotExporter.h"


std::set<std::string> Evolution::ExportStep::requiredData() const
{
    if(this->bestFitness.has_value()) {
        return {this->individuals, *this->bestFitness};
    }
    return {this->individuals};
}

std::string Evolution::ExportStep::details() const
{
    return this->filePath + (this->oneFilePerGeneration ? "<generation>" : "") + ".dot";
}

void Evolution::ExportStep::apply(EvolutionState& state)
{
    if(state.generation == EvolutionState::initGeneration) {
        return;
    }

    std::vector<std::shared_ptr<const Individual>> exported;
    if(this->bestFitness.has_value()) {
        std::shared_ptr<const Individual> best = state.bestIndividual(this->individuals, *this->bestFitness);
        if(best == nullptr) {
            return;
        }
        exported.push_back(best);
    } else {
        const std::shared_ptr<const std::vector<std::shared_ptr<const Individual>>> individuals = state.get<std::vector<std::shared_ptr<const Individual>>>(this->individuals);
        exported.insert(exported.end(), individuals->begin(), individuals->end());
    }

    const std::string path = this->filePath + (this->oneFilePerGeneration ? std::to_string(state.generation) : "") + ".dot";
    File::PopulationDotExporter::exportToFile(path, exported);
}
