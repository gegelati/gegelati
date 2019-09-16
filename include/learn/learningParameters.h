#ifndef LEARNING_PARAMETERS_H
#define LEARNING_PARAMETERS_H

#include <cstdint>

#include "mutator/mutationParameters.h"

namespace Learn {
	/**
	* \brief Structure for simplifying the transmission of LearningParameters 
	* to functions.
	*/	
	typedef struct LearningParameters {
		Mutator::MutationParameters mutation;
		/// Number of recordings held in the Archive.
		size_t archiveSize;
		/// Number of evaluation of each policy per generation.
		uint64_t nbIterationsPerPolicyEvaluation;
		/// Maximum number of action per evaluation of a policy. 
		uint64_t maxNbActionsPerEval;
		/// Percentage of deleted (and regenerated) root TPGVertex a each 
		/// generation.
		double ratioDeletedRoots;
		/// Number of generations of the training.
		uint64_t nbGenerations;
	} LearningParameters;
};

#endif