
#include "evaluation/archiveMetric.h"


Evaluation::ArchiveMetric::~ArchiveMetric()
{
    for (const auto& pairHashInputs : this->inputsExtracted) {
        for (const Data::DataHandler& dHandler : pairHashInputs.second) {
            // Free memory of DataHandlers within the archive
            delete &dHandler;
        }
    }
}

size_t Evaluation::ArchiveMetric::getCombinedHash(
    const std::vector<std::reference_wrapper<const Data::DataHandler>>&
        dHandlers)
{
    size_t hash = 0;
    for (const std::reference_wrapper<const Data::DataHandler> dHandler :
         dHandlers) {
        hash ^= dHandler.get().getHash();
    }
    return hash;
}

const std::map<size_t, std::vector<std::reference_wrapper<const Data::DataHandler>>>&  Evaluation::ArchiveMetric::getInputsExtracted() const
{
    return this->inputsExtracted;
}


void Evaluation::ArchiveMetric::extractMetricsStep(
    const Evolution::Individual& individual, std::vector<double> actionValues,
    const Learn::LearningEnvironment& learningEnvironment)
{
    
    if(this->rng.getDouble(0.0, 1.0) < this->extractionProbability) {
        /// Success, the inputs are extracted.
        const std::vector<std::reference_wrapper<const Data::DataHandler>>& dHandler = learningEnvironment.getDataSources();
        // get the combined hash
        size_t hash = getCombinedHash(dHandler);

        // Check if dataHandler is already contained, if yes do not add it.
        if (this->inputsExtracted.find(hash) != this->inputsExtracted.end()) {
            return;
        }

        
        // Store a copy of data handlers.
        std::vector<std::reference_wrapper<const Data::DataHandler>>
            dHandlersCpy;
        for (std::reference_wrapper<const Data::DataHandler> dh :
                dHandler) {
            Data::DataHandler* dhCopy = dh.get().clone();
            dHandlersCpy.push_back(*dhCopy);
        }

        // Create the map entry
        this->inputsExtracted.emplace(hash, std::move(dHandlersCpy));
    }
}
