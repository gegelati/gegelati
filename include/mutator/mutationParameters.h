#ifndef MUTATION_PARAMETERS_H
#define MUTATION_PARAMETERS_H

namespace Mutator {
	typedef struct TPGParameters {
		/// Number of TPGAction vertex of the initialized TPGGraph.
		size_t nbActions;
		/// Number of root TPGTeams to maintain when populating the TPGGraph
		size_t nbRoots;
		/// Maximum number of TPGEdge connected to each TPGTeam of the TPGGraph.
		size_t maxInitOutgoingEdges;
		size_t maxOutgoingEdges;
		double pEdgeDeletion;
		double pEdgeAddition;
		double pProgramMutation;
		double pEdgeDestinationChange;
		double pEdgeDestinationIsAction;
	} TPGParameters;

	typedef struct ProgramParameters {
		/// Maximum number of Line within the Program of the TPGGraph.
		size_t maxProgramSize;
		/// Probability of deleting a line of the Program.
		double pDelete;
		/// Probability of inserting a line in the Program
		double pAdd;
		/// Probability of altering a line of the Program.
		double pMutate;
		/// Probability of swapping two lines of the Program.
		double pSwap;
	} ProgramParameters;

	typedef struct MutationParameters {
		TPGParameters tpg;
		ProgramParameters prog;
	} MutationParameters;
}

#endif