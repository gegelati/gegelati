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

#include "log/mapElitesArchiveLogger.h"
#include "selector/mapElites/mapElitesArchive.h"

void Log::MapElitesArchiveLogger::logHeader()
{

    size_t nbBinPerDescriptor = archive.getDimensions().first;
    size_t nbDescriptors = archive.getDimensions().second;

    *this << "generation";

    std::vector<size_t> indices(nbDescriptors, 0);
    size_t total = std::pow(nbBinPerDescriptor, nbDescriptors);
    for (size_t count = 0; count < total; ++count) {

        std::string key;
        for (size_t i = 0; i < nbDescriptors; ++i) {
            key += std::to_string(indices[i]);
            if (i != nbDescriptors - 1)
                key += "_";
        }
        *this << "," << key;


        for (int i = nbDescriptors - 1; i >= 0; --i) {
            if (++indices[i] < nbBinPerDescriptor)
                break;
            indices[i] = 0;
        }
    }

    *this << ",archiveRange\n";

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

    std::vector<size_t> indices(nbDescriptors, 0);
    size_t total = std::pow(nbBinPerDescriptor, nbDescriptors);
    for (size_t count = 0; count < total; ++count) {
        const auto& elem = archive.getArchiveAt(indices);

        if (elem.second != nullptr) {
            *this << "," << elem.first->getSelectionMetrics()->getScore();
        } else {
            *this << ",nan";
        }

        for (int i = nbDescriptors - 1; i >= 0; --i) {
            if (++indices[i] < nbBinPerDescriptor)
                break;
            indices[i] = 0;
        }
    }

    if (!this->firstGenerationEnded) {
        *this << ",";
        for (size_t i = 0; i < archiveLimits.size(); ++i) {
            *this << archiveLimits[i];
            if (i != archiveLimits.size() - 1)
                *this << ";";
        }
        firstGenerationEnded = true;
    }

    *this << "\n";
}
