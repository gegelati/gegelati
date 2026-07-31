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

#include "learn/evaluationResult.h"
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
         * Vector containing the environment cloned specifically for the parallelism
         */
        std::vector<LearningEnvironment*> allCloneLearningEnvironments;

        /**
         * \brief Method for evaluating agents with parallelism.
         *
         * The work is delegated in two distinct methods (this structure is
         * made for inheritance purpose) : evaluateAgentsInParallelExecute and
         * evaluateAgentsInParallelCompileResults.
         *
         * \param[in] jobsToProcess Ordered list of jobs of
         * Agents to process
         * \param[in] generationNumber the integer number of the current
         * generation.
         * \param[in] mode the LearningMode to use during the policy
         * evaluation.
         * \param[in] results Map to store the resulting score of
         * evaluated agents.
         */
        virtual void evaluateAgentsInParallel(
            std::queue<std::shared_ptr<Representation::Job>>& jobsToProcess, uint64_t generationNumber, LearningMode mode,
            std::multimap<std::shared_ptr<EvaluationResult>,
                          std::reference_wrapper<const Representation::Agent>>& results);

        /**
         * \brief Subfunction of evaluateAllAgentsInParallel which handles the
         * creation of threads, their execution and junction.
         *
         * \param[in] jobsToProcess Ordered list of jobs of
         * Agents to process
         * \param[in] generationNumber the integer number of the current
         * generation.
         * \param[in] mode the LearningMode to use during the policy
         * evaluation.
         * \param[out] resultsPerJobMap map linking the job number with its
         * results and itself.
         */
        virtual void evaluateAgentsInParallelExecute(
            std::queue<std::shared_ptr<Representation::Job>>& jobsToProcess, uint64_t generationNumber, LearningMode mode,
            std::map<uint64_t, std::pair<std::shared_ptr<EvaluationResult>,
                                         std::shared_ptr<Representation::Job>>>&
                resultsPerJobMap);

        /**
         * \brief Subfunction of evaluateAllAgentsInParallel which handles the
         * gathering of results.
         *
         * This method just emplaces results from resultsPerJobMap, as each
         * job only contains 1 agent is is quite easy.
         *
         * @param[in] resultsPerJobMap map linking the job number with its
         * results and itself.
         * @param[out] results map linking single results to their agent vertex.
         */
        virtual void evaluateAgentsInParallelCompileResults(
            std::map<uint64_t, std::pair<std::shared_ptr<EvaluationResult>,
                                         std::shared_ptr<Representation::Job>>>&
                resultsPerJobMap,
            std::multimap<std::shared_ptr<EvaluationResult>,
                          std::reference_wrapper<const Representation::Agent>>& results);

        /**
         * \brief Function implementing the behavior of slave threads during
         * parallel evaluation of agents.
         *
         * \param[in] generationNumber the integer number of the current
         * generation.
         * \param[in] mode the LearningMode to use during the policy
         * evaluation.
         * \param[in,out] jobsToProcess Ordered list of jobs. 
         * The jobs are groups of agents that shall be agents in the
         * same simulation, there is only 1 agent if there is no adversarial
         * (e.g. if the environmnent is not multiplayer).
         * \param[in] agentsToProcessMutex Mutex protecting the
         * agentsToProcess
         * \param[in] resultsPerAgentMap Map to store the
         * resulting score of evaluated agents.
         * \param[in] resultsPerAgentMapMutex
         * Mutex protecting the results.
         * \param[in] indexEnvironment Index of the environment.
         */
        void slaveEvalJobThread(
            uint64_t generationNumber, LearningMode mode,
            std::queue<std::shared_ptr<Representation::Job>>& jobsToProcess,
            std::mutex& agentsToProcessMutex,
            std::map<uint64_t, std::pair<std::shared_ptr<EvaluationResult>,
                                         std::shared_ptr<Representation::Job>>>&
                resultsPerAgentMap,
            std::mutex& resultsPerAgentMapMutex,
            size_t indexEnvironment);

      public:
        /// @brief Destructor need to delete the copied environments
        virtual ~ParallelLearningAgent();

        /**
         * \brief Constructor for ParallelLearningAgent.
         *
         * Based on default constructor of LearningAgent
         *
         * \param[in] le The LearningEnvironment for the TPG.
         * \param[in] representations vector of representation learned by the learning agent
         * \param[in] parameters The LearningParameters for the LearningAgent.
         * \param[in] factory The GraphFactory used to create the Graph. A
         * default GraphFactory is used if none is provided.
         */
        ParallelLearningAgent(
            LearningEnvironment& le, std::vector<std::reference_wrapper<Representation::Representation>> representations,
            std::unique_ptr<LearningParameters> parameters = std::make_unique<LearningParameters>(),
            const EvoGraph::GraphFactory& factory = EvoGraph::GraphFactory())
            : LearningAgent(le, representations, std::move(parameters), factory)
        {};

        /**
         * \brief Constructor for ParallelLearningAgent.
         *
         * Based on default constructor of LearningAgent
         *
         * \param[in] le The LearningEnvironment for the TPG.
         * \param[in] representation vector of representation learned by the learning agent
         * \param[in] parameters The LearningParameters for the LearningAgent.
         * \param[in] factory The GraphFactory used to create the Graph. A
         * default GraphFactory is used if none is provided.
         */
        ParallelLearningAgent(
            LearningEnvironment& le, Representation::Representation& representation,
            std::unique_ptr<LearningParameters> parameters = std::make_unique<LearningParameters>(),
            const EvoGraph::GraphFactory& factory = EvoGraph::GraphFactory())
            : ParallelLearningAgent(le, std::vector<std::reference_wrapper<Representation::Representation>>{representation}, std::move(parameters), factory) {};


        /**
         * \brief Inherrit from LearningAgent, set the maxNbThreads value.
         * 
         * \param[in] seed the seed given to the TPGMutator.
         * \param[in] doGeneratePopulation boolean to indicate if population of the representations should be generated here.
         */
        virtual void init(uint64_t seed = 0, bool doGeneratePopulation = true) override;

        /**
         * \brief Evaluate all agent of an representation.
         *
         * **Replaces the function from the base class LearningAgent.**
         *
         * This method must always return the same results as the evaluateOneRepresentationAgents for
         * a sequential execution. 
         *
         * \param[in] generationNumber the integer number of the current
         * generation.
         * \param[in] mode the LearningMode to use during the policy
         * evaluation.
         * 
         */
        std::multimap<std::shared_ptr<EvaluationResult>, std::reference_wrapper<const Representation::Agent>>
        evaluateCurrentRepresentationAgents(uint64_t generationNumber, LearningMode mode) override;
    };
} // namespace Learn
#endif
