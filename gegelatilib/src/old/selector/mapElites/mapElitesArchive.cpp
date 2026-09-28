
#include "selector/mapElites/mapElitesArchive.h"

uint64_t Selector::MapElites::MapElitesArchive::size() const
{
    return archive.size();
}

std::pair<uint64_t, uint64_t> Selector::MapElites::MapElitesArchive::
    getDimensions() const
{
    return std::make_pair(nbBinPerDescriptor, nbDescriptors);
}

std::vector<double> Selector::MapElites::MapElitesArchive::getArchiveLimits()
    const
{
    return this->archiveLimits;
}

const std::vector<
    std::pair<std::shared_ptr<Learn::EvaluationResult>, std::optional<std::reference_wrapper<const Representation::Individual>>>>&
Selector::MapElites::MapElitesArchive::getAllArchive() const
{
    return archive;
}

uint64_t Selector::MapElites::MapElitesArchive::getIndexArchive(
    double value) const
{
    uint64_t idx = 0;
    while (idx < archiveLimits.size() && value > archiveLimits[idx]) {
        idx++;
    }
    return idx >= nbBinPerDescriptor ? nbBinPerDescriptor - 1 : idx;
}

uint64_t Selector::MapElites::MapElitesArchive::computeLinearIndex(
    const std::vector<uint64_t>& indices) const
{
    uint64_t index = 0;
    uint64_t multiplier = 1;

    for (int i = nbDescriptors - 1; i >= 0; --i) {
        index += indices[i] * multiplier;
        multiplier *= nbBinPerDescriptor;
    }

    return index;
}

std::vector<uint64_t> Selector::MapElites::MapElitesArchive::computeIndices(
    uint64_t index) const
{
    std::vector<uint64_t> indices(nbDescriptors, 0);
    for (int i = nbDescriptors - 1; i >= 0; --i) {
        indices[i] = index % nbBinPerDescriptor;
        index /= nbBinPerDescriptor;
    }
    return indices;
}

const std::pair<std::shared_ptr<Learn::EvaluationResult>,
                std::optional<std::reference_wrapper<const Representation::Individual>>>&
Selector::MapElites::MapElitesArchive::getArchiveFromDescriptors(
    const std::vector<double>& descriptors) const
{
    std::vector<uint64_t> indices;
    for (uint64_t i = 0; i < nbDescriptors; ++i) {
        indices.push_back(getIndexArchive(descriptors[i]));
    };

    return archive[computeLinearIndex(indices)];
}

void Selector::MapElites::MapElitesArchive::setArchiveFromDescriptors(
    const Representation::Individual& individual, std::shared_ptr<Learn::EvaluationResult> eval,
    const std::vector<double>& descriptors)
{
    std::vector<uint64_t> indices;
    for (uint64_t i = 0; i < nbDescriptors; ++i) {
        indices.push_back(getIndexArchive(descriptors[i]));
    }

    std::reference_wrapper<const Representation::Individual> individualRef = individual;
    archive[computeLinearIndex(indices)] = std::make_pair(eval, individualRef);
}

const std::pair<std::shared_ptr<Learn::EvaluationResult>,
                std::optional<std::reference_wrapper<const Representation::Individual>>>&
Selector::MapElites::MapElitesArchive::getArchiveAt(
    const std::vector<uint64_t>& indices) const
{
    return archive[computeLinearIndex(indices)];
}

void Selector::MapElites::MapElitesArchive::setArchiveAt(
    const Representation::Individual& individual, std::shared_ptr<Learn::EvaluationResult> eval,
    const std::vector<uint64_t>& indices)
{
    std::reference_wrapper<const Representation::Individual> individualRef = individual;
    archive[computeLinearIndex(indices)] = std::make_pair(eval, individualRef);
}

bool Selector::MapElites::MapElitesArchive::containsIndividual(
    const Representation::Individual& individual) const
{

    for (const auto& pair : archive) {
        if (pair.second == individual) {
            return true;
        }
    }
    return false;
}

void Selector::MapElites::MapElitesArchive::removeIndividualFromArchive(
    const Representation::Individual& individual, size_t maxNbEvaluation)
{
    for (auto it = archive.begin(); it != archive.end(); ++it) {
        if (it->second == individual) {
            if (it->first->getNbEvaluation() < maxNbEvaluation) {
                // Remove the individual from the archive if it has been evaluated
                // enough
                it->first = nullptr;
                it->second = std::nullopt; // Clear the vertex pointer
            }
        }
    }
}


std::set<std::reference_wrapper<const Representation::Individual>> Selector::MapElites::MapElitesArchive::
    getVerticesInArchive() const
{
    std::set<std::reference_wrapper<const Representation::Individual>> verticesInArchive;
    for (const auto& pair : archive) {
        if (pair.second) {
            verticesInArchive.insert(pair.second.value());
        }
    }

    return verticesInArchive;
}