#ifndef CLASSIFICATION_LEARNING_AGENT_H
#define CLASSIFICATION_LEARNING_AGENT_H

#include <type_traits>

#include "learn/learningAgent.h"
#include "learn/ParallelLearningAgent.h"

namespace Learn {
	/**
	* \brief LearningAgent specialized for LearningEnvironments representing a
	* classification problem.
	*
	* The key difference between this ClassificationLearningAgent and the base
	* LearningAgent is the way roots are selected for decimation after each
	* generation. In this agent, the roots are decimated based on an average
	* score **per class** instead of decimating roots based on their
	* global average score (over all classes) during the last evaluation.
	* By doing so, the roots providing the best score in each class are
	* preserved which increases the chances of correct classifiers emergence
	* for all classes.
	*
	* In this context, it is assumed that each action of the
	* LearningEnvironment represents a class of the classification problem.
	*
	* The BaseLearningAgent template parameter is the LearningAgent from which
	* the ClassificationLearningAgent inherits. This template notably enable
	* selecting between the classical and the ParallelLearningAgent.
	*/
	template <class BaseLearningAgent = ParallelLearningAgent>  class ClassificationLearningAgent : public BaseLearningAgent {
		static_assert(std::is_convertible<BaseLearningAgent*, LearningAgent*>::value);

	public:
		/**
		* \brief Constructor for LearningAgent.
		*
		* \param[in] le The LearningEnvironment for the TPG.
		* \param[in] iSet Set of Instruction used to compose Programs in the
		*            learning process.
		* \param[in] p The LearningParameters for the LearningAgent.
		* \param[in] nbRegs The number of registers for the execution
		*                   environment of Program.
		*/
		ClassificationLearningAgent(LearningEnvironment& le, const Instructions::Set& iSet, const LearningParameters& p, const unsigned int nbRegs = 8) : BaseLearningAgent(le, iSet, p, nbRegs) {};

		/**
		* \brief Decimate worst root specialized for classification purposes.
		*/
		void decimateWorstRoots(std::multimap<double, const TPG::TPGVertex*>& results) override;
	};
	
};
#endif