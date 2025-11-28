/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2022) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
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

#ifndef PARALLEL_LEARNING_AGENT
#define PARALLEL_LEARNING_AGENT

#include <mutex>
#include <queue>
#include <thread>

#include "instructions/set.h"
#include "tpg/tpgExecutionEngine.h"

#include "learn/evaluationResult.h"
#include "learn/job.h"
#include "learn/learningAgent.h"
#include "learn/learningEnvironment.h"
#include "learn/learningParameters.h"

namespace Learn {
    /**
     * \brief  Class used to control the learning steps of a Graph within
     * a given LearningEnvironment, with parallel executions for speedup
     * purposes.
     *
     * This class is intented to replace the default LearningAgent soon.
     *
     * Because of parallelism, determinism of the LearningProcess could easiliy
     * be lost, but this implementation must remain deterministic at all costs.
     */
    class ParallelLearningAgent : public LearningAgent
    {
      protected:
        /**
         * \brief Method for evaluating all agents with parallelism.
         *
         * The work is delegated in two distinct methods (this structure is
         * made for inheritance purpose) : evaluateAllAgentsInParallelExecute and
         * evaluateAllAgentsInParallelCompileResults.
         *
         * \param[in] generationNumber the integer number of the current
         * generation.
         * \param[in] mode the LearningMode to use during the policy
         * evaluation.
         * \param[in] results Map to store the resulting score of
         * evaluated agents.
         */
        virtual void evaluateAllAgentsInParallel(
            uint64_t generationNumber, LearningMode mode,
            std::multimap<std::shared_ptr<EvaluationResult>,
                          std::shared_ptr<const Algorithm::Agent>>& results);

        /**
         * \brief Subfunction of evaluateAllAgentsInParallel which handles the
         * creation of threads, their execution and junction.
         *
         * @param[in] generationNumber the integer number of the current
         * generation.
         * @param[in] mode the LearningMode to use during the policy
         * evaluation.
         * @param[out] resultsPerJobMap map linking the job number with its
         * results and itself.
         * @param[out] archiveMap map linking the job number with its gathered
         * archive. These archive swill later be merged with the ones of the
         * other jobs.
         */
        virtual void evaluateAllAgentsInParallelExecute(
            uint64_t generationNumber, LearningMode mode,
            std::map<uint64_t, std::pair<std::shared_ptr<EvaluationResult>,
                                         std::shared_ptr<Job>>>&
                resultsPerJobMap,
            std::map<uint64_t, Archive*>& archiveMap);

        /**
         * \brief Subfunction of evaluateAllAgentsInParallel which handles the
         * gathering of results and the merge of the archives.
         *
         * This method just emplaces results from resultsPerJobMap, as each
         * job only contains 1 agent is is quite easy.
         * The archive is merged with the mergeArchiveMap method.
         *
         * @param[in] resultsPerJobMap map linking the job number with its
         * results and itself.
         * @param[out] results map linking single results to their agent vertex.
         * @param[in,out] archiveMap map linking the job number with its
         * gathered archive. These archive swill later be merged with the ones
         * of the other jobs.
         */
        virtual void evaluateAllAgentsInParallelCompileResults(
            std::map<uint64_t, std::pair<std::shared_ptr<EvaluationResult>,
                                         std::shared_ptr<Job>>>&
                resultsPerJobMap,
            std::multimap<std::shared_ptr<EvaluationResult>,
                          std::shared_ptr<const Algorithm::Agent>>& results,
            std::map<uint64_t, Archive*>& archiveMap);

        /**
         * \brief Function implementing the behavior of slave threads during
         * parallel evaluation of agents.
         *
         * \param[in] generationNumber the integer number of the current
         * generation.
         * \param[in] mode the LearningMode to use during the policy
         * evaluation.
         * \param[in,out] jobsToProcess Ordered list of jobs of
         * TPGVertex to process, stored as a pair with an id filling the
         * archiveMap. The jobs are groups of agents that shall be agents in the
         * same simulation, there is only 1 agent if there is no adversarial
         * (e.g. if the environmnent is not multiplayer).
         * \param[in] agentsToProcessMutex Mutex protecting the
         * agentsToProcess
         * \param[in] resultsPerAgentMap Map to store the
         * resulting score of evaluated agents.
         * \param[in] resultsPerAgentMapMutex
         * Mutex protecting the results.
         * \param[in,out] archiveMap Map
         * storing the exhaustiveArchive to be merged.
         * \param[in] archiveMapMutex Mutex protecting the archiveMap.
         * \param[in] useMainEnvironment Boolean that is true if we use the
         * declared LearningEnvironment, otherwise the method will clone it.
         */
        void slaveEvalJobThread(
            uint64_t generationNumber, LearningMode mode,
            std::queue<std::shared_ptr<Learn::Job>>& jobsToProcess,
            std::mutex& agentsToProcessMutex,
            std::map<uint64_t, std::pair<std::shared_ptr<EvaluationResult>,
                                         std::shared_ptr<Job>>>&
                resultsPerAgentMap,
            std::mutex& resultsPerAgentMapMutex,
            std::map<uint64_t, Archive*>& archiveMap,
            std::mutex& archiveMapMutex, bool useMainEnvironment);

        /**
         * \brief Method to merge several Archive created in parallel
         * threads.
         *
         * The purpose of this method is to merhe several Archive
         * into the archive attribute of this ParallelLearningAgent. This
         * method is the key to obtain deterministic Archive even in a parallel
         * context.
         *
         * \param[in,out] archiveMap Map storing the Archive to be merged.
         */
        void mergeArchiveMap(std::map<uint64_t, Archive*>& archiveMap);

      public:
        /**
         * \brief Constructor for ParallelLearningAgent.
         *
         * Based on default constructor of LearningAgent
         *
         * \param[in] le The LearningEnvironment for the TPG.
         * \param[in] iSet Set of Instruction used to compose Programs in the
         *            learning process.
         * \param[in] algorithms vector of algorithm learned by the learning agent
         * \param[in] p The LearningParameters for the LearningAgent.
         * \param[in] factory The TPGFactory used to create the Graph. A
         * default TPGFactory is used if none is provided.
         */
        ParallelLearningAgent(
            LearningEnvironment& le, std::vector<std::shared_ptr<Algorithm::Algorithm>> algorithms, const Instructions::Set& iSet,
            const LearningParameters& p,
            const EvoGraph::TPGFactory& factory = EvoGraph::TPGFactory())
            : LearningAgent(le, algorithms, iSet, p, factory)
        {
            // overriding the maxNbThreads that basic LA defined to 1
            maxNbThreads = p.nbThreads;
        };

        /**
         * \brief Constructor for ParallelLearningAgent.
         *
         * Based on default constructor of LearningAgent
         *
         * \param[in] le The LearningEnvironment for the TPG.
         * \param[in] iSet Set of Instruction used to compose Programs in the
         *            learning process.
         * \param[in] algorithm vector of algorithm learned by the learning agent
         * \param[in] p The LearningParameters for the LearningAgent.
         * \param[in] factory The TPGFactory used to create the Graph. A
         * default TPGFactory is used if none is provided.
         */
        ParallelLearningAgent(
            LearningEnvironment& le, std::shared_ptr<Algorithm::Algorithm> algorithm, const Instructions::Set& iSet,
            const LearningParameters& p,
            const EvoGraph::TPGFactory& factory = EvoGraph::TPGFactory())
            : LearningAgent(le, algorithm, iSet, p, factory)
        {
            // overriding the maxNbThreads that basic LA defined to 1
            maxNbThreads = p.nbThreads;
        };

        /**
         * \brief Evaluate all agent TPGVertex of the Graph.
         *
         * **Replaces the function from the base class LearningAgent.**
         *
         * This method must always the same results as the evaluateAllAgents for
         * a sequential execution. The Archive should also be updated in the
         * exact same manner.
         *
         * This method calls the evaluateJob method for every agent TPGVertex
         * of the Graph. The method returns a sorted map associating each
         * agent vertex to its average score, in ascending order or score.
         *
         * \param[in] generationNumber the integer number of the current
         * generation.
         * \param[in] mode the LearningMode to use during the policy
         * evaluation.
         */
        std::multimap<std::shared_ptr<EvaluationResult>, std::shared_ptr<const Algorithm::Agent>>
        evaluateAllAgents(uint64_t generationNumber, LearningMode mode) override;
    };
} // namespace Learn
#endif
