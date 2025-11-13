/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2020 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2020)
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

#ifndef MAP_ELITES_ARCHIVE_LOGGER_H
#define MAP_ELITES_ARCHIVE_LOGGER_H

#include <iomanip>

#include "log/laLogger.h"

namespace Selector {
    namespace MapElites {
        class MapElitesArchive;
    }
} // namespace Selector

namespace Log {

    /**
     * \brief MapElitesArchive logger that will display some useful information
     */
    class MapElitesArchiveLogger : public LALogger
    {
      protected:
        /// archive of the mapElitesSelector
        const Selector::MapElites::MapElitesArchive& archive;

        /// If first generation if finished or not.
        bool firstGenerationEnded = false;

      public:
        /**
         * \brief Same constructor as LaLogger. Default output is cout.
         *
         * \param[in] archive archive of a mapElitesSelector whose information
         * will be logger by the logger
         * \param[in] la LearningAgent whose information will be logged by the LABasicLogger.
         * \param[in] out The output stream the logger will send elements to.
         */
        explicit MapElitesArchiveLogger(
            const Selector::MapElites::MapElitesArchive& archive,
            Learn::LearningAgent& la, std::ostream& out = std::cout)
            : LALogger(la, out), archive{archive}
        {

            // fixing float precision
            this->logHeader();
        }

        /**
         * Inherited via LaLogger
         *
         * \brief Logs the header (column names) of the csv archive.
         */
        virtual void logHeader() override;

        /**
         * Inherited via LALogger.
         *
         * \brief Logs the generation of training.
         *
         * \param[in] generationNumber The number of the current
         * generation.
         */
        virtual void logNewGeneration(uint64_t& generationNumber) override;

        /**
         * Inherited via LaLogger
         *
         * \brief Logs the values of the archive.
         */
        virtual void logEndOfTraining() override;
    };

} // namespace Log

#endif // MAP_ELITES_ARCHIVE_LOGGER_H
