#ifndef PARALLEL_LEARNING_AGENT
#define PARALLEL_LEARNING_AGENT

#include <thread>
#include <mutex>
#include <queue>

#include "instructions/set.h"
#include "learn/learningParameters.h"
#include "learn/learningEnvironment.h"
#include "learn/learningAgent.h"

namespace Learn {
	/**
	* \brief  Class used to control the learning steps of a TPGGraph within
	* a given LearningEnvironment, with parallel executions for speedup
	* purposes.
	*
	* This class is intented to replace the default LearningAgent soon.
	*
	* Because of parallelism, determinism of the LearningProcess could easiliy
	* be lost, but this implementation must remain deterministic at all costs.
	*/
	class ParallelLearningAgent : public LearningAgent {
	protected:
		/// Control the maximum number of threads when running in parallel.
		const uint64_t maxNbThreads;

		/**
		* \brief Method for evaluating all roots with parallelism.
		*
		* \param[in] generationNumber the integer number of the current generation.
		* \param[in] mode the LearningMode to use during the policy evaluation.
		* \param[in] results Map to store the resulting score of evaluated roots.
		*/
		void evaluateAllRootsInParallel(uint64_t generationNumber, LearningMode mode, std::multimap<double, const TPG::TPGVertex*>& results);

		/**
		* \brief Function implementing the behavior of slave threads during
		* parallel evaluation of roots.
		*/
		void slaveEvalRootThread(uint64_t generationNumber, LearningMode mode,
			std::queue<std::pair<uint64_t, const TPG::TPGVertex*>>& rootsToProcess, std::mutex& rootsToProcessMutex,
			std::multimap<double, const TPG::TPGVertex*>& results, std::mutex& resultsMutex,
			std::map<uint64_t, ExhaustiveArchive*>& archiveMap, std::mutex& archiveMapMutex,
			uint64_t& nextArchiveToMerge, std::mutex& archiveMergingMutex);


		void mergeArchiveMap(std::mutex& archiveMergingMutex, std::mutex& archiveMapMutex, uint64_t& nextArchiveToMerge, std::map<uint64_t, ExhaustiveArchive*>& archiveMap);

	public:
		/**
		* \brief Constructor for ParallelLearningAgent.
		*
		* Based on default constructor of LearningAgent
		*
		* \param[in] le The LearningEnvironment for the TPG.
		* \param[in] iSet Set of Instruction used to compose Programs in the
		*            learning process.
		* \param[in] p The LearningParameters for the LearningAgent.
		* \param[in] nbRegs The number of registers for the execution
		*                   environment of Program.
		* \param[in] maxNbThreads Integer parameter controlling the number of
		* threads used for parallel execution. Possible values are:
		*   - default:  Let the runtime decide using
		*               std::thread::hardware_concurrency().
		*   - `0` and `1`: Do not use parallelism.
		*   - `n > 1`: Set the number of threads explicitly.
		*/
		ParallelLearningAgent(LearningEnvironment& le, const Instructions::Set& iSet, const LearningParameters& p, const uint64_t maxNbThreads = std::thread::hardware_concurrency(), const unsigned int nbRegs = 8) :
			LearningAgent(le, iSet, p, nbRegs), maxNbThreads{ maxNbThreads } {};

		/**
		* \brief Evaluates policy starting from the given root.
		*
		* **Overload the function from the base class LearningAgent.**
		*
		* The policy, that is, the TPGGraph execution starting from the given
		* TPGVertex is evaluated nbIteration times. The generationNumber is
		* combined with the current iteration number to generate a set of
		* seeds for evaluating the policy.
		* The method returns the average score for this policy.
		*
		* \param[in] root the TPGVertex from which the policy evaluation starts.
		* \param[in] generationNumber the integer number of the current generation.
		* \param[in] mode the LearningMode to use during the policy evaluation.
		* \param[in] le Reference to the LearningEnvironment to use during the
		* policy evaluation.
		* \param[in,out] archive Reference to the Archive to use during the policy
		* evaluation.
		* \param[in] params Reference to the LearningParameters.
		*/
		static double evaluateRoot(const TPG::TPGVertex& root, uint64_t generationNumber, LearningMode mode, LearningEnvironment& le, Archive& archive, const Learn::LearningParameters& params);

		/**
		* \brief Evaluate all root TPGVertex of the TPGGraph.
		*
		* **Replaces the function from the base class LearningAgent.**
		*
		* This method must always the same results as the evaluateAllRoots for
		* a sequential execution. The Archive should also be updated in the
		* exact same manner.
		*
		* This method calls the evaluateRoot method for every root TPGVertex
		* of the TPGGraph. The method returns a sorted map associating each root
		* vertex to its average score, in ascending order or score.
		*
		* \param[in] generationNumber the integer number of the current generation.
		* \param[in] mode the LearningMode to use during the policy evaluation.
		*/
		std::multimap<double, const TPG::TPGVertex*> evaluateAllRoots(uint64_t generationNumber, LearningMode mode) override;
	};
}
#endif 
