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



#ifndef MAP_ELITES_SELECTOR_H
#define MAP_ELITES_SELECTOR_H

#include "selector/mapElites/mapElitesArchive.h"
#include "selector/mapElites/mapElitesDescriptor.h"
#include "selector/mapElites/mapElitesSelectionMetrics.h"
#include "selector/selector.h"

#include "learn/learningEnvironment.h"

namespace Selector {
    namespace MapElites {

        /**
         * \brief Selection class that will do a selection with a Map Elites
         * algorithm.
         *
         * The different archives are stored in the mapEliteArchives attribute,
         * each archive corresponding to a descriptor.
         */
        class MapElitesSelector : public Selector
        {
          protected:
            /// Map of descriptor to their corresponding archive
            std::map<std::shared_ptr<const MapElitesDescriptor>,
                     std::shared_ptr<MapElitesArchive>>
                mapEliteArchives;

          public:
            /**
             * \brief Constructor for Selector.
             *
             * \param[in] graph shared pointer of the graph on which the
             * selection is done.
             * \param[in] params parameters used by the
             * Selector.
             */
            MapElitesSelector(std::shared_ptr<TPG::TPGGraph> graph,
                              const Learn::LearningParameters& params)
                : Selector{graph, params}
            {
            }

            /**
             * \brief Get the archive corresponding to a given descriptor.
             *
             * \param[in] descriptor the descriptor to get the archive for.
             */
            virtual std::shared_ptr<const MapElitesArchive>
            getMapElitesArchiveAt(
                std::shared_ptr<const MapElitesDescriptor> descriptor);

            /**
             * \brief Get all the map elites archives.
             */
            virtual const std::map<std::shared_ptr<const MapElitesDescriptor>,
                                   std::shared_ptr<MapElitesArchive>>&
            getMapElitesArchives();

            /**
             * Specialization of createSelectionMetrics
             *
             * Creates and return an instance of MapElitesSelectionMetrics
             */
            virtual std::shared_ptr<SelectionMetrics> createSelectionMetrics()
                override;

            /**
             * \brief Add an archive for a given descriptor.
             *
             * \param[in] nbBins the number of bins for the archive.
             * \param[in] descriptor the descriptor to add an archive for.
             * \param[in] le the learning environment used to get size and
             * ranges.
             */
            virtual std::shared_ptr<const MapElitesArchive>
            addArchiveFromDescriptor(
                size_t nbBins,
                std::shared_ptr<const MapElitesDescriptor> descriptor,
                Learn::LearningEnvironment& le);

            /**
             * \brief Add a CVT archive for a given descriptor.
             *
             * \param[in] nbCentroids the number of centroids for the CVT
             * archive.
             * \param[in] descriptor the descriptor to add an archive
             * for.
             * \param[in] le the learning environment used to get size and
             * ranges.
             * \param[in] rng the random number generator used for the
             * CVT initialization.
             * \param[in] nbIterationInit number of iterations for the
             * CVT initialization.
             * \param[in] nbDotsInit number of dots for the CVT
             * initialization.
             * \param[in] a1 CVT default parameter.
             * \param[in] b1 CVT default parameter.
             * \param[in] a2 CVT default parameter.
             * \param[in] b2 CVT default parameter.
             */
            virtual std::shared_ptr<const MapElitesArchive>
            addCvtArchiveFromDescriptor(
                size_t nbCentroids,
                std::shared_ptr<const MapElitesDescriptor> descriptor,
                Learn::LearningEnvironment& le, Mutator::RNG& rng,
                size_t nbIterationInit = 300, size_t nbDotsInit = 1000,
                double a1 = 0.5, double b1 = 0.5, double a2 = 0.5,
                double b2 = 0.5);

            /**
             * \brief override of doSelection method
             *
             *
             * \param[in,out] results a multimap containing root TPGVertex
             * associated to their score during an evaluation.
             * \param[in] rng Random Number Generator used in the mutation
             * process.
             */
            virtual void doSelection(
                std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                              const TPG::TPGVertex*>& results,
                Mutator::RNG& rng) override;

            /**
             * \brief Specialization of updateContext for MapElites purposes
             */
            virtual const SelectionContext& updateContext() override;
        };
    }; // namespace MapElites

    /**
     * To make the selector accessible from both Selector::MapElites and from
     * Selector
     */
    using MapElitesSelector = MapElites::MapElitesSelector;
}; // namespace Selector

#endif // MAP_ELITES_SELECTOR_H
