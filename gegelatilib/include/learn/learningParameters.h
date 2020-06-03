/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
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
		/// MutationParameters for controlling stochastic aspects of the learning 
		/// process.
		Mutator::MutationParameters mutation;
		/// Number of recordings held in the Archive.
		size_t archiveSize;
		/// Probability of archiving each Program execution.
		double archivingProbability;
		/// Number of evaluation of each policy per generation.
		uint64_t nbIterationsPerPolicyEvaluation;
		/// Maximum number of action per evaluation of a policy. 
		uint64_t maxNbActionsPerEval;
		/// Percentage of deleted (and regenerated) root TPGVertex a each 
		/// generation.
		double ratioDeletedRoots;
		/// Number of generations of the training.
		uint64_t nbGenerations;
		/// Maximum number of times a given policy (i.e. a root TPGVertex) is
		/// evaluated.
		size_t maxNbEvaluationPerPolicy;
	} LearningParameters;
};

#endif
