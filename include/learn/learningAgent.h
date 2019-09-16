#ifndef LEARNING_AGENT_H
#define LEARNING_AGENT_H

#include <map>

#include "instructions/set.h"
#include "environment.h"
#include "archive.h"
#include "tpg/tpgGraph.h"
#include "mutator/mutationParameters.h"

#include "learn/learningEnvironment.h"

namespace Learn {
	class LearningAgent {
	protected:
		/// LearningEnvironment with which the LearningAgent will interact.
		LearningEnvironment& learningEnvironment;

		/// Environment for executing Program of the LearningAgent
		Environment env;

		/// Archive used during the training process
		Archive archive;

		/// Mutation Parameters for the learning process
		Mutator::MutationParameters params;

		/// TPGGraph built during the learning process.
		TPG::TPGGraph tpg;

	public:
		/**
		* \brief Constructor for LearningAgent.
		*
		* \param[in] le The LearningEnvironment for the TPG.
		*/
		LearningAgent(LearningEnvironment& le, const Instructions::Set& iSet, const Mutator::MutationParameters& p, const unsigned int nbRegs = 8, const size_t archiveSize = 50) :
			learningEnvironment{ le },
			env(iSet, le.getDataSources(), nbRegs),
			tpg(this->env),
			params{ p },
			archive(archiveSize)
		{
			// override the number of actions from the parameters.
			this->params.tpg.nbActions = this->learningEnvironment.getNbActions();
		};

		/**
		* \brief Getter for the TPGGraph built by the LearningAgent.
		* 
		* \return Get a reference to the TPGGraph.
		*/
		TPG::TPGGraph& getTPGGraph();

		/**
		* \brief Initialize the LearningAgent.
		*
		* Calls the TPGMutator::initRandomTPG function.
		* Initialize the Mutator::RNG with the given seed.
		* Clears the Archive.
		*
		*/
		void init(uint64_t seed = 0);

		/**
		* \brief Evaluates policy starting from the given root.
		*
		* The policy, that is, the TPGGraph execution starting from the given 
		* TPGVertex is evaluated nbIteration times. The generationNumber is 
		* combined with the current iteration number to generate a set of 
		* seeds for evaluating the policy.
		* The method returns the average score for this policy.
		*/
		double evaluateRoot(const TPG::TPGVertex& root, uint64_t generationNumber, uint64_t nbIterations, uint64_t maxNbActionsPerEval);

		/**
		* \brief Evaluate all root TPGVertex of the TPGGraph.
		*
		* This method calls the evaluateRoot method for every root TPGVertex
		* of the TPGGraph. The method returns a sorted map associating each root 
		* vertex to its average score, in ascending order or score.
		*/
		std::multimap<double, const TPG::TPGVertex*> evaluateAllRoots(uint64_t generationNumber, uint64_t nbIterations, uint64_t maxNbActionsPerEval);

		/**
		* \brief Train the TPGGraph for one generation.
		*
		* Training for one generation includes:
		* - Populating the TPGGraph according to given MutationParameters.
		* - Evaluating all roots of the TPGGraph. (call to evaluateAllRoots)
		* - Removing from tge TPGGraph the worst performing root TPGVertex.
		*/
		void trainOneGeneration(double ratioDeletedRoots, uint64_t generationNumber, uint64_t nbIterations, uint64_t maxNbActionsPerEval);
	};
};

#endif