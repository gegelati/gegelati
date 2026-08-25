#include "evaluation/archiveEnvironment.h"



void Evaluation::ArchiveEnvironment::setDimensionDataSource(std::vector<std::reference_wrapper<const Data::DataHandler>> dHandler)
{
    // Store a copy of data handlers.
    this->inputDimensions.clear();
    for (std::reference_wrapper<const Data::DataHandler> dh :
            dHandler) {
        Data::DataHandler* dhCopy = dh.get().clone();
        this->inputDimensions.push_back(*dhCopy);
    }
}

std::vector<std::reference_wrapper<const Data::DataHandler>> Evaluation::ArchiveEnvironment::getDataSources() const
{
    return this->inputDimensions;
}


void Evaluation::ArchiveEnvironment::setArchiveInputPopulation(const Evolution::Population& population)
{
    this->archiveInputPopulation = population;
}

bool Evaluation::ArchiveEnvironment::hasArchiveInputPopulation()
{
    return this->archiveInputPopulation.has_value();
}

const std::optional<std::reference_wrapper<const Evolution::Population>>& Evaluation::ArchiveEnvironment::getArchiveInputPopulation()
{
    return this->archiveInputPopulation;
}

void Evaluation::ArchiveEnvironment::setArchiveOutputPopulation(const Evolution::Population& population)
{
    this->archiveOutputPopulation = population;
}

bool Evaluation::ArchiveEnvironment::hasArchiveOutputPopulation()
{
    return this->archiveOutputPopulation.has_value();
}

const std::optional<std::reference_wrapper<const Evolution::Population>>& Evaluation::ArchiveEnvironment::getArchiveOutputPopulation()
{
    return this->archiveOutputPopulation;
}


std::set<size_t> Evaluation::ArchiveEnvironment::getCurrentIDs() const
{
    std::set<size_t> Ids;
    if(this->archive.size() > 0) {
        for(const auto& pair: this->archive.begin()->second) {
            Ids.insert(pair.first);
        }
    }
    return Ids;
}

size_t Evaluation::ArchiveEnvironment::getSize() const
{
    return this->archiveSize;
}

size_t Evaluation::ArchiveEnvironment::getCurrentSize() const
{
    return this->archive.size();   
}

const std::vector<std::reference_wrapper<const Data::DataHandler>>& Evaluation::ArchiveEnvironment::getInput(size_t idx) const
{
    if(idx >= this->archive.size()) {
        throw std::runtime_error("Evaluation::ArchiveEnvironment::getInput: index out of range.");
    }
    return this->archive.at(idx).first;
}

double Evaluation::ArchiveEnvironment::getOutput(size_t idx, size_t ID) const
{
    if(idx >= this->archive.size()) {
        throw std::runtime_error("Evaluation::ArchiveEnvironment::getOutput: input index out of range.");
    }
    auto it = this->archive.at(idx).second.find(ID);
    if(it == this->archive.at(idx).second.end()) {
        throw std::runtime_error("Evaluation::ArchiveEnvironment::getOutput: output ID not found.");
    }
    return it->second;
}



void Evaluation::ArchiveEnvironment::updateArchiveInputs()
{
    if(!this->archiveInputPopulation.has_value()) {
        throw std::runtime_error("Evaluation::ArchiveEvalAgent::updateArchiveInputs: An population to get the archive input should be set when trying to update the inputs.");
    }
    std::set<std::reference_wrapper<const Evolution::Individual>> teamIndividuals = this->archiveInputPopulation.value().get().getIndividuals();

    std::map<size_t, std::vector<std::reference_wrapper<const Data::DataHandler>>> inputsExtracted;

    // First get all archiveMetric input measured.
    // Disgusting code!
    for(const Evolution::Individual& teams: teamIndividuals) {
        for(const auto& pairRun: teams.getEvaluationResult().getEvaluationRuns()) {
            for(const std::unique_ptr<Evaluation::EvaluationMetric>& metric: pairRun.second->getMetrics()) {
                if(dynamic_cast<const Evaluation::ArchiveMetric*>(metric.get()) != nullptr) {
                    const std::map<size_t, std::vector<std::reference_wrapper<const Data::DataHandler>>>& localInputs = 
                        (dynamic_cast<const Evaluation::ArchiveMetric*>(metric.get()))->getInputsExtracted();

                    inputsExtracted.insert(localInputs.begin(), localInputs.end());
                }
            }
        }
    }
    // Clear archive //TODO: for now it removes everything at each generation.
    this->archive.clear();

    for(size_t idx = 0; idx < this->archiveSize && !inputsExtracted.empty(); idx++) {
        // Select random inputs.
        auto it = inputsExtracted.begin();
        std::advance(it, rng.getUnsignedInt64(0, inputsExtracted.size() - 1));
        
        // Store a copy of data handlers.
        std::vector<std::reference_wrapper<const Data::DataHandler>>
            dHandlersCpy;
        for (std::reference_wrapper<const Data::DataHandler> dh :
                it->second) {
            Data::DataHandler* dhCopy = dh.get().clone();
            dHandlersCpy.push_back(*dhCopy);
        }


        std::map<size_t, double> emptyRecord;
        this->archive.push_back({std::move(dHandlersCpy), std::move(emptyRecord)});
        inputsExtracted.erase(it);
    }
}

void Evaluation::ArchiveEnvironment::updateArchiveOutputs(
    const Evolution::Representation& representation)
{
    if(!this->archiveInputPopulation.has_value()) {
        throw std::runtime_error("Evaluation::ArchiveEvalAgent::updateArchiveOutputs: A population to get the archive output should be set when trying to update the outputs.");
    }
    std::set<std::reference_wrapper<const Evolution::Individual>> individuals(this->archiveOutputPopulation.value().get().getProtectedIndividuals());

    for(const Evolution::Individual& individual: individuals) {
        if(!representation.isValid(individual)){
            throw std::runtime_error("Evaluation::ArchiveEvalAgent::updateArchiveOutputs: Individual not valid for the representation");
        }
    }

    for(auto& pair: this->archive) {

        // Clear the current result
        pair.second.clear();

        // Execute each individual (could be optimized)
        for(const Evolution::Individual& individual: individuals) {
            pair.second.insert({
                individual.getIndividualID(),
                representation.executeIndividual(individual, pair.first).at(0)

            });
        };
    }
}