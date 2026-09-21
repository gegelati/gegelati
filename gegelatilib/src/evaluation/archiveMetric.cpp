
#include "evaluation/archiveMetric.h"

std::unique_ptr<Evaluation::EvaluationMetric> Evaluation::ArchiveMetric::cloneEmptyUniquePtr(size_t seed) const
{
    return std::make_unique<Evaluation::ArchiveMetric>(seed, this->extractionProbability);
}


std::string Evaluation::ArchiveMetric::toString(std::string prefix) const
{
    std::ostringstream oss;
    oss << prefix << "Seed:"<<this->seed <<"; pExtract:" << this->extractionProbability<<"; nbExtract:" << this->inputsExtracted.size();

    return oss.str();
}
std::type_index Evaluation::ArchiveMetric::typeId() const noexcept
{
    return typeid(ArchiveMetric);
}


const std::map<size_t, std::vector<std::pair<std::unique_ptr<std::byte[]>, Data::DataType>>>& Evaluation::ArchiveMetric::getInputsExtracted() const
{
    return this->inputsExtracted;
}

void Evaluation::ArchiveMetric::extractBeforeExecution(
    const Individual& individual,
    const Evaluation::Problem& problem)
{
    
    if(this->rng.getDouble(0.0, 1.0) < this->extractionProbability) {
        /// Success, the inputs are extracted.
        std::vector<Data::DataView> views = problem.getDataSources();

        // get the combined hash
        size_t hash = Data::DataView::getCombinedHash(views);

        // Check if dataHandler is already contained, if yes do not add it.
        if (this->inputsExtracted.find(hash) != this->inputsExtracted.end()) {
            return;
        }

        // Store a copy of data handlers.
        std::vector<std::pair<std::unique_ptr<std::byte[]>, Data::DataType>> viewDeepCopies;
        for (const Data::DataView& view : views) {
            viewDeepCopies.push_back(view.deepClone());
        }

        // Create the map entry
        this->inputsExtracted.emplace(hash, std::move(viewDeepCopies));
    }
}
