/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2020 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2020 - 2021)
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

#ifndef POLICY_STATS_H
#define POLICY_STATS_H

#include <iostream>
#include <map>
#include <vector>


#include "representation/individual.h"
#include "evoGraph/graph.h"

namespace Representation {

    /**
     * Utility class for extracting statistics from a policy within a Graph.
     *
     * The entry point for using this class is the analyzePolicy() method which
     * updates all attribute for a "tree" starting from a given individual.
     *
     * To analyze different policies with a single PolicyStats instance, the
     * clear() method should be called between calls to analyzePolicy().
     *
     * For access simplicity, all attributes filled during the analysis are
     * public. Tampering with them will just make the result of the analysis
     * useless.
     */
    class PolicyStats
    {
      public:

        /**
         * \brief Number of time a Team was analyzed.
         *
         * When analyzing a policy, this number corresponds to
         * the number of times this Team is the destination of a Edge.
         */
        std::map<std::reference_wrapper<const EvoGraph::Team>, size_t> nbUsePerTeam;

        /**
         * \brief Number of time a Action was analyzed.
         *
         * When analyzing a policy, this number corresponds to
         * the number of times this Action is the destination of a Edge.
         */
        std::map<std::reference_wrapper<const EvoGraph::Action>, size_t> nbUsePerAction;

        /// Number of outgoing Edge of per Team of the Graph.
        std::vector<size_t> nbOutgoingEdgesPerTeam;

        /**
         * Each entry of this map associates an action ID to the number of
         * time it was present in the analyzed policy.
         */
        std::map<size_t, size_t> nbUsagePerActionID;

        /// Depth of the analyzed policy.
        size_t maxPolicyDepth = 0;


        /**
         * Each entry of this map represents a level deepth of the policy tree
         * and the number of Team that "first" appeared within this level.
         *
         * A Team may appear several time in a single policy, but only its
         * "lowest" level is counted here.
         */
        std::map<size_t, std::set<std::reference_wrapper<const EvoGraph::Vertex>>> vertexPerDepthLevel;

        /// Number of distinct Teams per policy.
        size_t nbDistinctTeams = 0;

        /// Representation name
        std::string representationName;

        /// Representation name
        uint64_t representationID;

        /// @brief map storing the sub policy stats of the sub representations
        std::map<uint64_t, std::shared_ptr<PolicyStats>> subPolicyStats;


        /// Default polymorphic destructor
        virtual ~PolicyStats() = default;

        // Disable copying to avoid accidental copies (use references or pointers instead).
        PolicyStats(const PolicyStats&) = delete;
        PolicyStats& operator=(const PolicyStats&) = delete;
        
        /**
         * \brief Main constructor of the PolicyStats class.
         * 
         * \param[in] representationName the name of the representation for which the stats are stored.
         * \param[in] representationID the id of the representation for which the stats are stored.
         */
        PolicyStats(std::string representationName, uint64_t representationID) : representationName{representationName}, representationID{representationID} {}

        /**
         * \brief Main constructor of the PolicyStats class.
         * 
         * \param[in] representationName the name of the representation for which the stats are stored.
         * \param[in] representationID the id of the representation for which the stats are stored.
         * \param[in] subPolicyStatsMap the map associating the name of the sub representations to their policy stats, used for the analysis of sub policies.
         */
        PolicyStats(std::string representationName, uint64_t representationID, const std::map<uint64_t, std::shared_ptr<PolicyStats>>& subPolicyStatsMap) : representationName{representationName}, representationID{representationID}, subPolicyStats{subPolicyStatsMap} {}

        /**
         * \brief get the sub policy stats of a sub representation.
         * 
         * \param[in] subRepresentationID the id of the sub representation to get the stats of.
         */
        PolicyStats& getSubPolicyStats(uint64_t subRepresentationID) const;

        /**
         * Clear all stats stored in the class attributes.
         */
        virtual void clear();

        /**
         * Analyze the given Vertex.
         * 
         * The method execute either analyzeTeam() or analyzeAction() depending on the type of the vertex given.
         * 
         * Also it analyze program of outgoing edge and program inside the vertex.
         */
        virtual void analyzeVertex(const EvoGraph::Vertex& vertex, size_t depth);

        /**
         * Analyze the given Team.
         *
         * The method updates the following stats:
         * - Number of use per Team.
         * - Number of outgoing Edge per Team.
         * - Total number of distinct Team in the policy.
         *
         * If a Team was already analyzed, it will not be analyzed again and
         * only the number of use per Team will be updated.
         */
        virtual void analyzeTeam(const EvoGraph::Team& team);

        /**
         * Analyze the given Action.
         *
         * The method updates the following stats:
         * - Number of use per Action.
         * - Total number of usage per action.
         *
         * If a Action was already analyzed, it will not be analyzed again
         * and only the number of use per Action will be updated.
         */
        virtual void analyzeAction(const EvoGraph::Action& action);

        /**
         * Analyze the policy starting from the given Individual.
         *
         * This method explores the Graph starting from the given Individual,
         * and analyzes all Team, Action and other individuals encountered along the
         * way.
         *
         * The method updates the following stats:
         */
        virtual void analyzePolicy(const Individual& individual) = 0;

        /**
         * Method returning the specific informations of the representation used
         */
        virtual std::string specificInfos() const = 0;

        /**
         * \brief Get all the sub policy stats of the sub representations.
         */
        std::map<uint64_t, std::reference_wrapper<const PolicyStats>> getAllSubPolicyStats() const;

        friend std::ostream& operator<<(std::ostream& os,
                                        const PolicyStats& policyStats);
    };

    /// Overload of the stream output operator for the PolicyStats class.
    std::ostream& operator<<(std::ostream& os, const PolicyStats& policyStats);
} // namespace Representation

#endif
