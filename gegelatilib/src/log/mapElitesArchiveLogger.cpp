/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2020 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2020 - 2022)
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
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

#include <iomanip>
#include <numeric>

#include "learn/learningAgent.h"
#include "log/mapElitesArchiveLogger.h"
#include "selector/mapElites/cvtMapElitesArchive.h"
#include "selector/mapElites/mapElitesArchive.h"

void Log::MapElitesArchiveLogger::logHeader()
{

    size_t nbBinPerDescriptor = archive.getDimensions().first;
    size_t nbDescriptors = archive.getDimensions().second;

    // Always log the "generation" column first
    *this << "generation";

    // Handle CvtMapElitesArchive case
    if (const auto* cvtArchive =
            dynamic_cast<const Selector::MapElites::CvtMapElitesArchive*>(
                &archive)) {
        const auto& centroids = cvtArchive->getCentroids();
        for (size_t i = 0; i < centroids.size(); ++i) {
            *this << "," << i << "(";
            for (size_t j = 0; j < centroids[i].size(); ++j) {
                *this << centroids[i][j];
                if (j != centroids[i].size() - 1)
                    *this << ";";
            }
            *this << ")";
        }
    }
    // Handle default MapElitesArchive case
    else {
        for (uint64_t count = 0; count < archive.size(); ++count) {
            std::string key;
            const auto indices = archive.computeIndices(count);
            for (size_t idx = 0; idx < indices.size(); ++idx) {
                if (idx != 0)
                    key += "_";
                key += std::to_string(indices[idx]);
            }
            *this << "," << key;
        }
        *this << ",archiveRange";
    }

    *this << std::endl;
}

void Log::MapElitesArchiveLogger::logNewGeneration(uint64_t& generationNumber)
{
    *this << generationNumber;
}

void Log::MapElitesArchiveLogger::logEndOfTraining()
{
    size_t nbBinPerDescriptor = archive.getDimensions().first;
    size_t nbDescriptors = archive.getDimensions().second;
    std::vector<double> archiveLimits = archive.getArchiveLimits();

    for (size_t i = 0; i < archive.size(); ++i) {
        const auto& elem = archive.getAllArchive()[i];
        if (elem.second) {
            *this << "," << elem.first->getSelectionMetrics()->getScore();
        }
        else {
            *this << ",nan";
        }
    }

    if (dynamic_cast<const Selector::MapElites::CvtMapElitesArchive*>(
            &archive) == nullptr &&
        !this->firstGenerationEnded) {
        *this << ",";
        for (size_t i = 0; i < archiveLimits.size(); ++i) {

            *this << archiveLimits[i];
            if (i != archiveLimits.size() - 1)
                *this << ";";
        }
        firstGenerationEnded = true;
    }

    *this << std::endl;
}
