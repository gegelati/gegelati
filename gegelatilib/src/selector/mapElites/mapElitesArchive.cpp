
#include "selector/mapElites/mapElitesArchive.h"

uint64_t Selector::MapElites::MapElitesArchive::size() const
{
    return archive.size();
}

std::pair<uint64_t, uint64_t> Selector::MapElites::MapElitesArchive::getDimensions() const
{
    return std::make_pair(nbBinPerDescriptor, nbDescriptors);
}

std::vector<double> Selector::MapElites::MapElitesArchive::getArchiveLimits() const
{
    return this->archiveLimits;
}

const std::vector<std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>>& Selector::MapElites::MapElitesArchive::getAllArchive() const
{
    return archive;
}

uint64_t Selector::MapElites::MapElitesArchive::getIndexArchive(double value) const
{
    uint64_t idx = 0;
    while (idx < archiveLimits.size() && value > archiveLimits[idx]) {
        idx++;
    }
    return idx >= nbBinPerDescriptor ? nbBinPerDescriptor - 1 : idx;
}

uint64_t Selector::MapElites::MapElitesArchive::computeLinearIndex(const std::vector<uint64_t>& indices) const
{
    uint64_t index = 0;
    uint64_t multiplier = 1;

    for (int i = nbDescriptors - 1; i >= 0; --i) {
        index += indices[i] * multiplier;
        multiplier *= nbBinPerDescriptor;
    }

    return index;
}

const std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>& 
Selector::MapElites::MapElitesArchive::getArchiveFromDescriptors(const std::vector<double>& descriptors) const
{
    std::vector<uint64_t> indices;
    for (uint64_t i = 0; i < nbDescriptors; ++i) {
        indices.push_back(getIndexArchive(descriptors[i]));
    };

    

    return archive[computeLinearIndex(indices)];
}

void Selector::MapElites::MapElitesArchive::setArchiveFromDescriptors(
    const TPG::TPGVertex* vertex,
    std::shared_ptr<Learn::EvaluationResult> eval,
    const std::vector<double>& descriptors)
{
    std::vector<uint64_t> indices;
    for (uint64_t i = 0; i < nbDescriptors; ++i) {
        indices.push_back(getIndexArchive(descriptors[i]));
    }

    archive[computeLinearIndex(indices)] = std::make_pair(eval, vertex);
}

const std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>& 
    Selector::MapElites::MapElitesArchive::getArchiveAt(const std::vector<uint64_t>& indices) const
{
    return archive[computeLinearIndex(indices)];
}

void Selector::MapElites::MapElitesArchive::setArchiveAt(
    const TPG::TPGVertex* vertex,
    std::shared_ptr<Learn::EvaluationResult> eval,
    const std::vector<uint64_t>& indices)
{
    archive[computeLinearIndex(indices)] = std::make_pair(eval, vertex);
}


void Selector::MapElites::MapElitesArchive::initCSVarchive(std::string path) const {
    std::ofstream outFile(path);
    if (!outFile.is_open()) {
        std::cerr << "Archive file could not be created " << path << std::endl;
        return;
    }

    outFile << "generation";

    std::vector<size_t> indices(nbDescriptors, 0);
    size_t total = std::pow(nbBinPerDescriptor, nbDescriptors);
    for (size_t count = 0; count < total; ++count) {

        std::string key;
        for (size_t i = 0; i < nbDescriptors; ++i) {
            key += std::to_string(indices[i]);
            if (i != nbDescriptors - 1)
                key += "_";
        }
        outFile << "," << key;


        for (int i = nbDescriptors - 1; i >= 0; --i) {
            if (++indices[i] < nbBinPerDescriptor)
                break;
            indices[i] = 0;
        }
    }

    outFile << ",archiveRange\n";
    outFile.close();
}

void Selector::MapElites::MapElitesArchive::updateCSVArchive(std::string path, uint64_t generationNumber) const {
    std::ofstream outFile(path, std::ios::app);
    if (!outFile.is_open()) {
        std::cerr << "Archive file not found " << path << std::endl;
        return;
    }

    outFile << generationNumber;

    std::vector<size_t> indices(nbDescriptors, 0);
    size_t total = std::pow(nbBinPerDescriptor, nbDescriptors);
    for (size_t count = 0; count < total; ++count) {
        const auto& elem = archive[computeLinearIndex(indices)];

        if (elem.second != nullptr) {
            outFile << "," << elem.first->getSelectionMetrics()->getScore();
        } else {
            outFile << ",nan";
        }

        for (int i = nbDescriptors - 1; i >= 0; --i) {
            if (++indices[i] < nbBinPerDescriptor)
                break;
            indices[i] = 0;
        }
    }

    if (generationNumber == 0) {
        outFile << ",";
        for (size_t i = 0; i < archiveLimits.size(); ++i) {
            outFile << archiveLimits[i];
            if (i != archiveLimits.size() - 1)
                outFile << ";";
        }
    }

    outFile << "\n";
    outFile.close();
}


bool Selector::MapElites::MapElitesArchive::containsRoot(const TPG::TPGVertex* root) const {

    for (const auto& pair : archive) {
        if (pair.second == root) {
            return true;
        }
    }
    return false;
}



void Selector::MapElites::MapElitesArchive::removeRootFromArchiveIfNotComplete(const TPG::TPGVertex* root, size_t maxNbEvaluation)
{
    for (auto it = archive.begin(); it != archive.end(); ++it) {
        if (it->second == root) {
            if (it->first->getNbEvaluation() < maxNbEvaluation) {
                // Remove the root from the archive if it has been evaluated enough
                it->first = nullptr;
                it->second = nullptr; // Clear the vertex pointer
            }
        }
    }
}

void Selector::MapElites::MapElitesArchive::removeRootFromArchive(const TPG::TPGVertex* root, size_t maxNbEvaluation)
{
    for (auto it = archive.begin(); it != archive.end(); ++it) {
        if (it->second == root) {
            it->first = nullptr;
            it->second = nullptr; // Clear the vertex pointer
            break;
        }
    }
}

std::set<const TPG::TPGVertex*> Selector::MapElites::MapElitesArchive::getVerticesInArchive()
{
    std::set<const TPG::TPGVertex*> verticesInArchive;
    for(const auto& pair: archive){
        if(pair.second != nullptr){
            verticesInArchive.insert(pair.second);
        }
    }

    return verticesInArchive;
}