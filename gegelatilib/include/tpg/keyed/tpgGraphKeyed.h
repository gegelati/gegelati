#ifndef TPG_GRAPH_KEYED_H
#define TPG_GRAPH_KEYED_H

#include "tpg/keyed/tpgEdgeKeyed.h"
#include "tpg/keyed/tpgKeyedFactory.h"
#include "tpg/keyed/tpgTeamKeyed.h"
#include "tpg/tpgGraph.h"

namespace TPG {

    /**
     * \brief Specialization of TPGGraph for managing keyed TPG elements.
     *
     * TPGGraphKeyed extends TPGGraph to provide convenient methods for
     * creating TPGTeamKeyed and TPGEdgeKeyed with specified key and lock
     * values respectively. This class overloads addNewTeam() and
     * addNewEdge() methods to allow direct specification of these parameters
     * during element creation.
     */
    class TPGGraphKeyed : public TPGGraph
    {
      public:
        /**
         * \brief Constructor for TPGGraphKeyed.
         *
         * \param[in] e the Environment for the TPGGraph.
         * \param[in] f the TPGFactory used to create the graph elements.
         *              Should be a TPGKeyedFactory or derivative.
         */
        TPGGraphKeyed(const Environment& e,
                      std::unique_ptr<TPGFactory> f =
                          std::make_unique<TPG::TPGKeyedFactory>())
            : TPGGraph(e, std::move(f))
        {
        }

        /**
         * \brief Set a new key to a TPGTeamKeyed.
         *
         * \param[in] team the TPGTeamKeyed to change key.
         * \param[in] newKey the new key to set.
         */
        void addNewTeamKey(const TPG::TPGTeamKeyed& team, uint64_t newKey);

        /**
         * \brief Set the key of a TPGTeamKeyed to the next prime number.
         *
         * This method uses the lastPrime attribute to generate a new prime
         * number and set it as the key of the given TPGTeamKeyed.
         *
         * @return the new prime number set as the key of the TPGTeamKeyed.
         */
        uint64_t addNextTeamKey(const TPG::TPGTeamKeyed& team);

        /**
         * \brief Set a new lock to a TPGEdgeKeyed.
         *
         * \param[in] edge the TPGEdgeKeyed to change lock.
         * \param[in] newLock the new lock to set.
         */
        void setNewEdgeLock(const TPG::TPGEdgeKeyed& edge, uint64_t newLock);

        /**
         * @brief Add a new lock value to the existing lock of a TPGEdgeKeyed.
         *
         * @param[in] edge the TPGEdgeKeyed to modify.
         * @param[in] newLock the new lock value to add.
         */
        void addNewEdgeLock(const TPG::TPGEdgeKeyed& edge, uint64_t newLock);

        /**
         * @brief Get a subtree of the TPGGraphKeyed.
         *
         * Get the TPGTeamKeyed and TPGEdgeKeyed of the subtree rooted at the
         * given vertex, with a specific key.
         *
         * @param[in] root The root vertex of the subtree.
         * @param[in] keys Optional vector of keys to consider for unlocking
         * edges. If not provided, an empty set is used, meaning that only keys
         * collected during traversal will be considered.
         * @return a Pair of sets containing the TPGTeamKeyed and TPGEdgeKeyed
         * of the subtree.
         */
        std::pair<std::set<const TPGTeamKeyed*>, std::set<const TPGEdgeKeyed*>>
        getSubtree(const TPGVertex& root, std::set<uint64_t> keys = {}) const;

      protected:
        uint64_t lastPrime = 1; ///< Last prime number used for key generation.
    };

} // namespace TPG

#endif // TPG_GRAPH_KEYED_H
