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

#ifndef OLD_POLICY_STATS_H
#define OLD_POLICY_STATS_H

#include <iostream>
#include <map>
#include <vector>

#include "instructions/instruction.h"

#include "program/program.h"

#include "evoGraph/action.h"
#include "evoGraph/graph.h"
#include "evoGraph/team.h"

namespace EvoGraph {

    /**
     * Utility class for extracting statistics from a policy within a Graph.
     *
     * The entry point for using this class is the analyzePolicy() method which
     * updates all attribute for a "tree" starting from a given root Vertex.
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
      private:
        /// Environment used during analyses
        const Environment* environment = nullptr;

        /// Data sources (including registers) used in the Program.
        std::vector<std::reference_wrapper<const Data::DataHandler>>
            dataSourcesAndRegisters;

      public:
        /**
         * \brief Number of time a Program was analyzed.
         *
         * When analyzing a policy, this number corresponds to
         * the number of Edge referencing a Program.
         */
        std::map<const Program::Program*, size_t> nbUsePerProgram;

        /**
         * \brief Number of time a Program was analyzed.
         *
         * When analyzing a policy, this number corresponds to
         * the number of Edge referencing a Program.
         */
        std::map<const Program::Program*, size_t> nbUsePerActionProgram;

        /**
         * \brief Number of time a Team was analyzed.
         *
         * When analyzing a policy, this number corresponds to
         * the number of times this Team is the destination of a Edge.
         */
        std::map<const Team*, size_t> nbUsePerTeam;

        /**
         * \brief Number of time a Action was analyzed.
         *
         * When analyzing a policy, this number corresponds to
         * the number of times this Action is the destination of a Edge.
         */
        std::map<const Action*, size_t> nbUsePerAction;

        /// Number of lines of analyzed Program.
        std::vector<size_t> nbLinesPerProgram;

        /// Number of lines of analyzed Program.
        std::vector<size_t> nbLinesPerActionProgram;

        /// Number of intron lines of analyzed Program.
        std::vector<size_t> nbIntronPerProgram;

        /// Number of intron lines of analyzed Program.
        std::vector<size_t> nbIntronPerActionProgram;

        /**
         * Each entry of this map associates an Instruction identifier from
         * an instruction set with the total number of times it was used in
         * analyzed Programs.
         */
        std::map<size_t, size_t> nbUsagePerInstruction;

        /**
         * Each entry of this map associates an Instruction identifier from
         * an instruction set with the total number of times it was used in
         * analyzed Programs.
         */
        std::map<size_t, size_t> nbUsagePerInstructionActionProg;

        /**
         * Each entry of this map associates a data location with the total
         * number of times it was accessed by non-intron lines of analyzed
         * Programs.
         * Each data location is itself represented with a pair consisting of
         * the data source index, and the location within this data source.
         */
        std::map<std::pair<size_t, size_t>, size_t> nbUsagePerDataLocation;

        /**
         * Each entry of this map associates a data location with the total
         * number of times it was accessed by non-intron lines of analyzed
         * Programs.
         * Each data location is itself represented with a pair consisting of
         * the data source index, and the location within this data source.
         */
        std::map<std::pair<size_t, size_t>, size_t>
            nbUsagePerDataLocationActionProg;

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
        std::map<size_t, size_t> nbVertexPerDepthLevel;

        /// Number of distinct Teams per policy.
        size_t nbDistinctTeams = 0;

        /// Default constructor
        PolicyStats() = default;

        /**
         * Clear all stats stored in the class attributes.
         */
        void clear();

        /**
         * \brief Set Environement used during analyses.
         *
         * From the given Environment, this method sets class attributes used
         * during the analyses of the Program and Line of the policy. If the
         * given Environment does not correspond to the one known to the Program
         * exceptions may be thrown during analyses.
         */
        void setEnvironment(const Environment& env);

        /**
         * \brief Analyze the given Line.
         *
         * The method updates the following stats:
         * - Total number of usage of each Instruction.
         * - Total number of access for each location.
         *
         * \param[in] line line analized
         * \param[in] actionProgram boolean to indicate if the program is an
         * action program or a context program
         */
        void analyzeLine(const Program::Line* line, bool actionProgram = false);

        /**
         * \brief Analyze the given Program.
         *
         * The method updates the following stats:
         * - Number of use per Program.
         * - Number of lines per Program.
         * - Number of intron lines per Program.
         *
         * For each non-intron line, the analyzeLine() method will be called.
         *
         * If a Program was already analyzed, it will not be analyzed again and
         * only the number of use per program will be updated.
         *
         * \param[in] prog the analyzed Program.
         * \throws std::runtime_error if the given Program has incorrect lines
         * accessing for example non existing instructions.
         */
        void analyzeProgram(const Program::Program* prog);

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
        void analyzeTeam(const EvoGraph::Team* team);

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
        void analyzeAction(const EvoGraph::Action* action);

        /**
         * Analyze the policy starting from the given Vertex.
         *
         * This method explores the Graph starting from the given Vertex,
         * and analyzes all Team, Action and Program encountered along the
         * way.
         *
         * The method updates the following stats:
         * - Depth of the policy.
         * - Number of Team per depth level.
         */
        void analyzePolicy(const EvoGraph::Vertex* vertex);

        friend std::ostream& operator<<(std::ostream& os,
                                        const PolicyStats& policyStats);
    };

    /// Overload of the stream output operator for the PolicyStats class.
    std::ostream& operator<<(std::ostream& os, const PolicyStats& policyStats);
} // namespace EvoGraph

#endif
