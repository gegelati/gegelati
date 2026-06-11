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

#ifndef MAP_ELITES_ARCHIVE_H
#define MAP_ELITES_ARCHIVE_H

#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "learn/evaluationResult.h"
#include "tpg/tpgGraph.h"

namespace Selector {
    namespace MapElites {

        /**
         * \brief Class representing a Map Elites archive.
         *
         * The archive is represented as a multi-dimensional grid, where each
         * cell can store an EvaluationResult and its corresponding agent.
         */
        class MapElitesArchive
        {
          protected:
            /// Number of bins per descriptor
            uint64_t nbBinPerDescriptor;

            /// Number of descriptors for this archive
            uint64_t nbDescriptors;

            /// Minimum value for the descriptors
            double minValue;

            /// Maximum value for the descriptors
            double maxValue;

            /// Limits of the bins for the descriptors
            std::vector<double> archiveLimits;

            /// The archive storing evaluation results and their corresponding
            /// root TPGVertex
            std::vector<std::pair<std::shared_ptr<Learn::EvaluationResult>,
                                  const TPG::TPGVertex*>>
                archive;

          public:
            /**
             * \brief Constructor of the MapElitesArchive
             *
             * \param[in] nbBinPerDescriptor Number of bins per descriptor
             * \param[in] nbDescriptors Number of descriptors for this archive
             * \param[in] minValue Minimum value for the descriptors
             * \param[in] maxValue Maximum value for the descriptors
             */
            MapElitesArchive(size_t nbBinPerDescriptor, size_t nbDescriptors,
                             double minValue, double maxValue)
                : nbBinPerDescriptor{nbBinPerDescriptor},
                  nbDescriptors{nbDescriptors}, minValue{minValue},
                  maxValue{maxValue}
            {

                if (nbBinPerDescriptor > 0 && nbDescriptors > 0) {
                    archive.resize(std::pow(nbBinPerDescriptor, nbDescriptors));
                }

                for (size_t idx = 1; idx <= nbBinPerDescriptor; ++idx) {
                    archiveLimits.push_back((double)idx *
                                                (maxValue - minValue) /
                                                (double)nbBinPerDescriptor +
                                            minValue);
                }
            }

            /**
             * \brief Get the size of the archive
             */
            uint64_t size() const;

            /**
             * \brief Get the dimensions of the archive
             */
            std::pair<uint64_t, uint64_t> getDimensions() const;

            /**
             * \brief get the archive limits
             */
            std::vector<double> getArchiveLimits() const;

            /**
             * \brief Get all the archive content
             */
            virtual const std::vector<
                std::pair<std::shared_ptr<Learn::EvaluationResult>,
                          const TPG::TPGVertex*>>&
            getAllArchive() const;

            /**
             * \brief Get the archive content at given indices
             *
             * \param[in] indices the indices to get the archive content from
             */
            virtual const std::pair<std::shared_ptr<Learn::EvaluationResult>,
                                    const TPG::TPGVertex*>&
            getArchiveAt(const std::vector<uint64_t>& indices) const;

            /**
             * \brief Get the archive content at given descriptors
             *
             * \param[in] descriptors the descriptors to get the archive content
             * from
             */
            virtual const std::pair<std::shared_ptr<Learn::EvaluationResult>,
                                    const TPG::TPGVertex*>&
            getArchiveFromDescriptors(
                const std::vector<double>& descriptors) const;

            /**
             * \brief Set the archive content at given indices
             *
             * \param[in] vertex the TPGVertex to set in the archive
             * \param[in] eval the EvaluationResult to set in the archive
             * \param[in] indices the indices to set the archive content at
             */
            virtual void setArchiveAt(
                const TPG::TPGVertex* vertex,
                std::shared_ptr<Learn::EvaluationResult> eval,
                const std::vector<uint64_t>& indices);

            /**
             * \brief Set the archive content at given descriptors
             *
             * \param[in] vertex the TPGVertex to set in the archive
             * \param[in] eval the EvaluationResult to set in the archive
             * \param[in] descriptors the descriptors to set the archive content
             * at
             */
            virtual void setArchiveFromDescriptors(
                const TPG::TPGVertex* vertex,
                std::shared_ptr<Learn::EvaluationResult> eval,
                const std::vector<double>& descriptors);

            /**
             * \brief Get the index in one dimension of the archive from a value
             *
             * \param[in] value the value to get the index from
             */
            virtual uint64_t getIndexArchive(double value) const;

            /**
             * \brief Compute the linear index from multi-dimensional indices
             *
             * \param[in] indices the multi-dimensional indices to compute the
             * linear index from
             */
            virtual uint64_t computeLinearIndex(
                const std::vector<uint64_t>& indices) const;

            /**
             * \brief Compute the multi-dimensional indices from linear index
             *
             * \param[in] index the linear index to compute
             */
            virtual std::vector<uint64_t> computeIndices(uint64_t index) const;

            /**
             * \brief Check if the archive contains a root TPGVertex
             *
             * \param[in] root the root TPGVertex to check
             */
            virtual bool containsRoot(const TPG::TPGVertex* root) const;

            /**
             * \brief Remove a root TPGVertex from the archive if its number of
             * evaluation is below maxNbEvaluation
             *
             * \param[in] root the root TPGVertex to remove
             * \param[in] maxNbEvaluation the maximum number of evaluation
             * allowed
             */
            virtual void removeRootFromArchiveIfNotComplete(
                const TPG::TPGVertex* root, size_t maxNbEvaluation);

            /**
             * \brief Remove a root TPGVertex from the archive
             *
             * \param[in] root the root TPGVertex to remove
             * \param[in] maxNbEvaluation the maximum number of evaluation
             * allowed
             */
            virtual void removeRootFromArchive(const TPG::TPGVertex* root,
                                               size_t maxNbEvaluation);

            /**
             * \brief Return a set with the current vectors in the archive.
             */
            virtual std::set<const TPG::TPGVertex*> getVerticesInArchive()
                const;
        };

    }; // namespace MapElites
}; // namespace Selector

#endif // MAP_ELITES_SELECTOR_H
