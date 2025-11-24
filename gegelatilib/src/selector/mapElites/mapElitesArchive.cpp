
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
    std::pair<std::shared_ptr<Learn::EvaluationResult>, std::shared_ptr<const Algorithm::Agent>>>&
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
                std::shared_ptr<const Algorithm::Agent>>&
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
    std::shared_ptr<const Algorithm::Agent> vertex, std::shared_ptr<Learn::EvaluationResult> eval,
    const std::vector<double>& descriptors)
{
    std::vector<uint64_t> indices;
    for (uint64_t i = 0; i < nbDescriptors; ++i) {
        indices.push_back(getIndexArchive(descriptors[i]));
    }

    archive[computeLinearIndex(indices)] = std::make_pair(eval, vertex);
}

const std::pair<std::shared_ptr<Learn::EvaluationResult>,
                std::shared_ptr<const Algorithm::Agent>>&
Selector::MapElites::MapElitesArchive::getArchiveAt(
    const std::vector<uint64_t>& indices) const
{
    return archive[computeLinearIndex(indices)];
}

void Selector::MapElites::MapElitesArchive::setArchiveAt(
    std::shared_ptr<const Algorithm::Agent> vertex, std::shared_ptr<Learn::EvaluationResult> eval,
    const std::vector<uint64_t>& indices)
{
    archive[computeLinearIndex(indices)] = std::make_pair(eval, vertex);
}

bool Selector::MapElites::MapElitesArchive::containsAgent(
    std::shared_ptr<const Algorithm::Agent> agent) const
{

    for (const auto& pair : archive) {
        if (pair.second == agent) {
            return true;
        }
    }
    return false;
}

void Selector::MapElites::MapElitesArchive::removeAgentFromArchiveIfNotComplete(
    std::shared_ptr<const Algorithm::Agent> agent, size_t maxNbEvaluation)
{
    for (auto it = archive.begin(); it != archive.end(); ++it) {
        if (it->second == agent) {
            if (it->first->getNbEvaluation() < maxNbEvaluation) {
                // Remove the agent from the archive if it has been evaluated
                // enough
                it->first = nullptr;
                it->second = nullptr; // Clear the vertex pointer
            }
        }
    }
}

void Selector::MapElites::MapElitesArchive::removeAgentFromArchive(
    std::shared_ptr<const Algorithm::Agent> agent, size_t maxNbEvaluation)
{
    for (auto it = archive.begin(); it != archive.end(); ++it) {
        if (it->second == agent) {
            it->first = nullptr;
            it->second = nullptr; // Clear the vertex pointer
            break;
        }
    }
}

std::set<std::shared_ptr<const Algorithm::Agent>> Selector::MapElites::MapElitesArchive::
    getVerticesInArchive() const
{
    std::set<std::shared_ptr<const Algorithm::Agent>> verticesInArchive;
    for (const auto& pair : archive) {
        if (pair.second != nullptr) {
            verticesInArchive.insert(pair.second);
        }
    }

    return verticesInArchive;
}