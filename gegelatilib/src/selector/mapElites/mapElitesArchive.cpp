/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2025) :
 *
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */


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
    std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>>&
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
                const TPG::TPGVertex*>&
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
    const TPG::TPGVertex* vertex, std::shared_ptr<Learn::EvaluationResult> eval,
    const std::vector<double>& descriptors)
{
    std::vector<uint64_t> indices;
    for (uint64_t i = 0; i < nbDescriptors; ++i) {
        indices.push_back(getIndexArchive(descriptors[i]));
    }

    archive[computeLinearIndex(indices)] = std::make_pair(eval, vertex);
}

const std::pair<std::shared_ptr<Learn::EvaluationResult>,
                const TPG::TPGVertex*>&
Selector::MapElites::MapElitesArchive::getArchiveAt(
    const std::vector<uint64_t>& indices) const
{
    return archive[computeLinearIndex(indices)];
}

void Selector::MapElites::MapElitesArchive::setArchiveAt(
    const TPG::TPGVertex* vertex, std::shared_ptr<Learn::EvaluationResult> eval,
    const std::vector<uint64_t>& indices)
{
    archive[computeLinearIndex(indices)] = std::make_pair(eval, vertex);
}

bool Selector::MapElites::MapElitesArchive::containsRoot(
    const TPG::TPGVertex* root) const
{

    for (const auto& pair : archive) {
        if (pair.second == root) {
            return true;
        }
    }
    return false;
}

void Selector::MapElites::MapElitesArchive::removeRootFromArchiveIfNotComplete(
    const TPG::TPGVertex* root, size_t maxNbEvaluation)
{
    for (auto it = archive.begin(); it != archive.end(); ++it) {
        if (it->second == root) {
            if (it->first->getNbEvaluation() < maxNbEvaluation) {
                // Remove the root from the archive if it has been evaluated
                // enough
                it->first = nullptr;
                it->second = nullptr; // Clear the vertex pointer
            }
        }
    }
}

void Selector::MapElites::MapElitesArchive::removeRootFromArchive(
    const TPG::TPGVertex* root, size_t maxNbEvaluation)
{
    for (auto it = archive.begin(); it != archive.end(); ++it) {
        if (it->second == root) {
            it->first = nullptr;
            it->second = nullptr; // Clear the vertex pointer
            break;
        }
    }
}

std::set<const TPG::TPGVertex*> Selector::MapElites::MapElitesArchive::
    getVerticesInArchive() const
{
    std::set<const TPG::TPGVertex*> verticesInArchive;
    for (const auto& pair : archive) {
        if (pair.second != nullptr) {
            verticesInArchive.insert(pair.second);
        }
    }

    return verticesInArchive;
}
