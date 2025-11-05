

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
        class MapElitesSelector : public Selector::Selector
        {
        protected:

            /// Map of descriptor to their corresponding archive
            std::map<std::shared_ptr<const MapElitesDescriptor>, std::shared_ptr<MapElitesArchive>> mapEliteArchives;
        public:
            /**
             * \brief Constructor for Selector.
             *
             * \param[in] graph shared pointer of the graph on which the selection
             * is done. 
             * \param[in] params parameters used by the Selector.
             */
            MapElitesSelector(std::shared_ptr<TPG::TPGGraph> graph,
                            const Learn::LearningParameters& params)
                : Selector::Selector{graph, params} {}

            
            /**
             * \brief Get the archive corresponding to a given descriptor.
             * 
             * \param[in] descriptor the descriptor to get the archive for.
             */
            virtual std::shared_ptr<const MapElitesArchive> getMapElitesArchiveAt(std::shared_ptr<const MapElitesDescriptor> descriptor);

            /**
             * \brief Get all the map elites archives.
             */
            virtual const std::map<std::shared_ptr<const MapElitesDescriptor>, std::shared_ptr<MapElitesArchive>>& getMapElitesArchives();

            /**
             * Specialization of createSelectionMetrics
             * 
             * Creates and return an instance of MapElitesSelectionMetrics
             */
            virtual std::shared_ptr<SelectionMetrics> createSelectionMetrics();

            /**
             * \brief Add an archive for a given descriptor.
             * 
             * \param[in] descriptor the descriptor to add an archive for.
             * \param[in] le the learning environment used to get size and ranges.
             */
            virtual void addArchiveFromDescriptor(std::shared_ptr<const MapElitesDescriptor> descriptor, Learn::LearningEnvironment& le);

            /**
             * \brief override of doSelection method
             *
             *
             * \param[in,out] results a multimap containing root TPGVertex
             * associated to their score during an evaluation.
             * \param[in] rng Random Number Generator used in the mutation process.
             */
            virtual void doSelection(
                std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                            const TPG::TPGVertex*>& results,
                Mutator::RNG& rng) override;
        };
    }; // namespace MapElites

    /**
     * To make the selector accessible from both Selector::MapElites and from Selector
     */
    using MapElitesSelector = MapElites::MapElitesSelector;
}; // namespace Selector

#endif // MAP_ELITES_SELECTOR_H