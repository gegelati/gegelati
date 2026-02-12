

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
             * \param[in] manager Manager used by the algorithm
             * \param[in] params parameters used by the
             * Selector.
             */
            MapElitesSelector(std::shared_ptr<Algorithm::AgentManager> manager,
                              const Learn::LearningParameters& params)
                : Selector{manager, params}
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
            virtual std::shared_ptr<SelectionMetrics> createSelectionMetrics() const
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
                Learn::LearningEnvironment& le, RNG::RNG& rng,
                size_t nbIterationInit = 300, size_t nbDotsInit = 1000,
                double a1 = 0.5, double b1 = 0.5, double a2 = 0.5,
                double b2 = 0.5);

            /**
             * \brief override of doSelection method
             *
             *
             * \param[in] graph the Graph on which selection is performed.
             * \param[in,out] results a multimap containing agent
             * associated to their score during an evaluation.
             * \param[in] rng Random Number Generator used in the mutation
             * process.
             */
            virtual void doSelection(
                std::shared_ptr<EvoGraph::Graph> graph,
                std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                              std::weak_ptr<const Algorithm::Agent>>& results,
                RNG::RNG& rng) override;

            /**
             * \brief Specialization of updateContext for MapElites purposes
             */
            virtual std::unique_ptr<SelectionContext> updateContext() const override;
        };
    }; // namespace MapElites

    /**
     * To make the selector accessible from both Selector::MapElites and from
     * Selector
     */
    using MapElitesSelector = MapElites::MapElitesSelector;
}; // namespace Selector

#endif // MAP_ELITES_SELECTOR_H