#ifndef POLICY_STATS_H
#define POLICY_STATS_H

#include <vector>
#include <map>

#include "instructions/instruction.h"

#include "program/program.h"

#include "tpg/tpgAction.h"
#include "tpg/tpgTeam.h"
#include "tpg/tpgGraph.h"

namespace TPG {

	/**
	* Utility class for extracting statistics from a policy within a TPGGraph.
	*
	* The entry point for using this class is the analyzePolicy() method which
	* updates all attribute for a "tree" starting from a given root TPGVertex.
	*
	* To analyze different policies with a single PolicyStats instance, the
	* clear() method should be called between calls to analyzePolicy().
	*/
	class PolicyStats {
	private:

		/**
		* \brief Number of time a Program was analyzed.
		*
		* When analyzing a policy, this number corresponds to
		* the number of TPGEdge referencing a Program.
		*/
		std::map<const Program::Program*, size_t> nbUsePerProgram;

		/**
		* \brief Number of time a TPGTeam was analyzed.
		*
		* When analyzing a policy, this number corresponds to
		* the number of times this TPGTeam is the destination of a TPGEdge.
		*/
		std::map<const TPGTeam*, size_t> nbUsePerTPGTeam;

		/**
		* \brief Number of time a TPGAction was analyzed.
		*
		* When analyzing a policy, this number corresponds to
		* the number of times this TPGAction is the destination of a TPGEdge.
		*/
		std::map<const TPGAction*, size_t> nbUsePerTPGAction;

		/// Number of lines of analyzed Program.
		std::vector<size_t> nbLinesPerProgram;

		/// Number of intron lines of analyzed Program.
		std::vector<size_t> nbIntronPerProgram;

		/**
		* Each entry of this map associates an Instruction identifier from
		* an instruction set with the total number of times it was used in
		* analyzed Programs.
		*/
		std::map<size_t, size_t> nbUsagePerInstruction;

		/**
		* Each entry of this map associates a data location with the total
		* number of times it was accessed by non-intron lines of analyzed
		* Programs.
		* Each data location is itself represented with a pair consisting of
		* the data source index, and the location within this data source.
		*/
		std::map<std::pair<size_t, size_t>, size_t> nbUsagePerDataLocation;

		/// Number of outgoing TPGEdge of per TPGTeam of the TPGGraph.
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
		* and the number of TPGTeam that "first" appeared within this level.
		*
		* A TPGTeam may appear several time in a single policy, but only its
		* "lowest" level is counted here.
		*/
		std::map<size_t, size_t> nbTeamPerDepthLevel;

		/// Number of distinct TPGTeams per policy.
		size_t nbDistinctTeams;

	public:
		/// Default constructor
		PolicyStats() = default;

		/**
		* Clear all stats stored in the class attributes.
		*/
		void clear();

		/**
		* Analyze the given Program.
		*
		* The method updates the following stats:
		* - Number of use per Program.
		* - Number of lines per Program.
		* - Number of intron lines per Program.
		* - Total number of usage of each Instruction.
		* - Total number of access for each location (ignoring intron).
		*
		* If a Program was already analyzed, it will not be analyzed again and
		* only the number of use per program will be updated.
		*
		* \param[in] prog the analyzed Program.
		*/
		void analyzeProgram(const Program::Program prog);

		/**
		* Analyze the given TPGTeam.
		*
		* The method updates the following stats:
		* - Number of use per TPGTeam.
		* - Number of outgoing TPGEdge per TPGTeam.
		*
		* If a TPGTeam was already analyzed, it will not be analyzed again and
		* only the number of use per TPGTeam will be updated.
		*/
		void analyzeTPGTeam(const TPG::TPGTeam* team);

		/**
		* Analyze the given TPGAction.
		*
		* The method updates the following stats:
		* - Number of use per TPGAction.
		* - Total number of usage per action.
		*
		* If a TPGAction was already analyzed, it will not be analyzed again and
		* only the number of use per TPGAction will be updated.
		*/
		void analyzeTPGAction(const TPG::TPGAction* team);

		/**
		* Analyze the policy starting from the given TPGVertex.
		*
		* This method explores the TPGGraph starting from the given TPGVertex,
		* and analyzes all TPGTeam, TPGAction and Program encountered along the
		* way.
		*
		* The method updates the following stats:
		* - Depth of the policy.
		* - Number of TPGTeam per depth level.
		* - Total number or distinct TPGTeam in the policy.
		*/
		void analyzePolicy(const TPG::TPGVertex* vertex);
	};
}

#endif 
