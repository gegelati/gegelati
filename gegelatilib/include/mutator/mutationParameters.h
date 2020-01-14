#ifndef MUTATION_PARAMETERS_H
#define MUTATION_PARAMETERS_H

#include <cstddef>

namespace Mutator {
	/**
	* \brief Structure holding all parameters affecting mutations of TPGGraph.
	*/
	typedef struct TPGParameters {
		/// Number of TPGAction vertex of the initialized TPGGraph.
		size_t nbActions;
		/// Number of root TPGTeams to maintain when populating the TPGGraph
		size_t nbRoots;
		/// Maximum number of TPGEdge connected to each TPGTeam of the TPGGraph.
		size_t maxInitOutgoingEdges;
		/// Maximum number of outgoing edge during TPGGraph mutations0 
		size_t maxOutgoingEdges;
		/// Probability of deleting an outgoing TPGEdge of a TPGTeam.
		double pEdgeDeletion;
		/// Probability of adding an outgoing TPGEdge to a TPGTeam.
		double pEdgeAddition;
		/// Probability of mutating the Program of an outgoing TPGEdge.
		double pProgramMutation;
		/// Probability of changing the destination of a TPGEdge.
		double pEdgeDestinationChange;
		/// Probability of the new destination of a TPGEdge to be a TPGAction.
		double pEdgeDestinationIsAction;
	} TPGParameters;

	/**
	* \brief Structure holding all parameters affecting mutations of Program.
	*/
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

	/**
	* \brief Structure holding all parameters affecting stochastic mutations.
	*/
	typedef struct MutationParameters {
		/// Parameters for TPGMutator
		TPGParameters tpg;
		/// Parameters for ProgramMutator 
		ProgramParameters prog;
	} MutationParameters;
}

#endif