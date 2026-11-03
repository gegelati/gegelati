#include "evolution/steps/exportStep.h"

#include <memory>
#include <vector>

#include "file/populationDotExporter.h"


std::set<std::string> Evolution::ExportStep::requiredData() const
{
    if(this->bestFitness.has_value()) {
        return {this->individuals.name(), this->bestFitness->name()};
    }
    return {this->individuals.name()};
}

std::string Evolution::ExportStep::details() const
{
    return this->filePath + (this->oneFilePerGeneration ? "<generation>" : "") + ".dot";
}

void Evolution::ExportStep::apply(EvolutionData& state)
{
    if(state.generation == EvolutionData::initGeneration) {
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
        exported = state.getIndividuals(this->individuals);
    }

    const std::string path = this->filePath + (this->oneFilePerGeneration ? std::to_string(state.generation) : "") + ".dot";
    File::PopulationDotExporter::exportToFile(path, exported);
}
