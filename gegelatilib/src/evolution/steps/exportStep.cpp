#include "evolution/steps/exportStep.h"

#include <memory>
#include <vector>

#include "file/populationDotExporter.h"
#include "util/individualUtils.h"


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
        const std::vector<std::shared_ptr<Individual>>& list = *state.get<std::vector<std::shared_ptr<Individual>>>(this->individuals);
        const std::shared_ptr<const Individual> best =
            (!state.has(*this->bestFitness)) ? nullptr : Util::bestIndividual(std::vector<std::shared_ptr<const Individual>>(list.begin(), list.end()), *state.get<std::map<uint64_t, double>>(*this->bestFitness));
        if(best == nullptr) {
            return;
        }
        exported.push_back(best);
    } else {
        const std::vector<std::shared_ptr<Individual>>& list = *state.get<std::vector<std::shared_ptr<Individual>>>(this->individuals);
        exported.assign(list.begin(), list.end());
    }

    const std::string path = this->filePath + (this->oneFilePerGeneration ? std::to_string(state.generation) : "") + ".dot";
    File::PopulationDotExporter::exportToFile(path, exported);
}
