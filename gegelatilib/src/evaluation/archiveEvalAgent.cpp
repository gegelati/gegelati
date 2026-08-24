#include "evaluation/archiveEvalAgent.h"

void Evaluation::ArchiveEvalAgent::setDimensionDataSource(std::vector<std::reference_wrapper<const Data::DataHandler>> dHandler)
{
    // Store a copy of data handlers.
    this->inputDimensions.clear();
    for (std::reference_wrapper<const Data::DataHandler> dh :
            dHandler) {
        Data::DataHandler* dhCopy = dh.get().clone();
        this->inputDimensions.push_back(*dhCopy);
    }
}

std::vector<std::reference_wrapper<const Data::DataHandler>> Evaluation::ArchiveEvalAgent::getDimensionsDataSources() const
{
    return this->inputDimensions;
}


std::shared_ptr<Evaluation::EvaluationResult> Evaluation::ArchiveEvalAgent::evaluateIndividual(
    const Evolution::Individual& individual, 
    const Evolution::Representation& representation,
    uint64_t generationNumber,
    Learn::LearningMode mode) const
{
    if(!representation.isValid(individual)){
        throw std::runtime_error("Evaluation::ArchiveEvalAgent::evaluateIndividual: Individual not valid for the representation");
    }


    // Create a list with all individual IDs.
    std::set<size_t> currentIndenticalIndivID;
    for(const ArchiveRecording& record : this->archive.begin()->second) {
        currentIndenticalIndivID.emplace(record.individualID);
    }


    // For each pair input/outputs in the archive, compare the current individual output to the outputs of every individuals recorded.
    for (const auto& pair : this->archive) {
        // Get the outputs
        double outputs =
            representation.executeIndividual(individual, pair.first).at(0);

        for (const ArchiveRecording& recording: pair.second) {
            // Execute only if the id is still in the set of possible identical IDs.
            auto it = currentIndenticalIndivID.find(recording.individualID);
            if(it != currentIndenticalIndivID.end()) {    
                if(outputs != recording.result) {
                    currentIndenticalIndivID.erase(it);
                }
            }
        }

        // No need to continue
        if(currentIndenticalIndivID.empty()) {
            break;
        }
    }

    // Novelty score is 1 if the identical list is empty, else 0
    double noveltyScore = double(currentIndenticalIndivID.empty());
    return std::make_shared<EvaluationResult>(
               std::move(std::make_unique<EvaluationRun>(
                   std::move(std::make_unique<ScoreMetric>(noveltyScore)))), 0);
}



std::map<std::reference_wrapper<const Evolution::Individual>, std::shared_ptr<Evaluation::EvaluationResult>>
Evaluation::ArchiveEvalAgent::evaluateIndividuals(
    const std::vector<std::reference_wrapper<const Evolution::Individual>>& individuals, 
    const Evolution::Representation& representation,
    uint64_t generationNumber,
    Learn::LearningMode mode) const
{
    std::map<std::reference_wrapper<const Evolution::Individual>, std::shared_ptr<Evaluation::EvaluationResult>>
        results;

    // Check that all input are recorded the same amout.
    size_t nbRecording = this->archive.begin()->second.size();
    for(const auto& pair: this->archive) {
        if(pair.second.size() != nbRecording) {
            throw std::runtime_error("Evaluation::ArchiveEvalAgent::evaluateIndividuals: Number of output recording per input is not constant");
        }
    }

    for(const Evolution::Individual& indiv: individuals){
        // Evaluate the individuals and insert the results
        const auto& result = this->evaluateIndividual(
            indiv, representation, generationNumber, mode
        );
        
        results.insert({indiv, result});
    }

    return results;
}

void Evaluation::ArchiveEvalAgent::updateArchiveInputs(std::map<std::reference_wrapper<const Evolution::Individual>, std::shared_ptr<Evaluation::EvaluationResult>> mapInputResults)
{
    std::map<size_t, std::vector<std::reference_wrapper<const Data::DataHandler>>> inputsExtracted;

    // First get all archiveMetric input measured.
    // Disgusting code!
    for(const auto& pairResults: mapInputResults) {
        for(const auto& pairRun: pairResults.second->getEvaluationRuns()) {
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


        std::vector<Evaluation::ArchiveRecording> emptyRecord;
        this->archive.push_back({std::move(dHandlersCpy), std::move(emptyRecord)});
        inputsExtracted.erase(it);
    }
}

void Evaluation::ArchiveEvalAgent::updateArchiveOutputs(
    const std::vector<std::reference_wrapper<const Evolution::Individual>>& individuals, 
    const Evolution::Representation& representation)
{
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
            pair.second.push_back(ArchiveRecording{
                individual.getIndividualID(),
                representation.executeIndividual(individual, pair.first).at(0)
            });
        };
    }
}
