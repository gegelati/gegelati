/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2023 - 2025)
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

#ifndef LEARNING_AGENT_H
#define LEARNING_AGENT_H

#include <map>
#include <queue>

#include "algorithm/algorithm.h"

#include "archive.h"
#include "environment.h"
#include "instructions/set.h"
#include "log/laLogger.h"
#include "mutator/mutationParameters.h"
#include "tpg/tpgExecutionEngine.h"
#include "tpg/tpgGraph.h"

#include "learn/classificationLearningEnvironment.h"
#include "learn/evaluationResult.h"
#include "learn/job.h"
#include "learn/learningEnvironment.h"
#include "learn/learningParameters.h"

namespace Learn {

    /**
     * \brief Class used to control the learning steps of a TPGGraph within
     * a given LearningEnvironment.
     */
    class LearningAgent
    {
      protected:
        /// LearningEnvironment with which the LearningAgent will interact.
        LearningEnvironment& learningEnvironment;

        /// Vector of shared pointer of algorithms learned by the learning agent.
        std::vector<std::shared_ptr<Algorithm::Algorithm>> algorithms;

        /// Environment for executing Program of the LearningAgent
        Environment env;

        /// Archive used during the training process
        Archive archive;

        /// Parameters for the learning process
        LearningParameters params;

        /// TPGGraph built during the learning process.
        std::shared_ptr<TPG::TPGGraph> graph;

        /// Random Number Generator for this Learning Agent
        RNG::RNG rng;

        /// Control the maximum number of threads when running in parallel.
        uint64_t maxNbThreads = 1;

        /**
         * \brief Set of LALogger called throughout the training process.
         *
         * Each LALogger of this set will be invoked at pre-defined steps of the
         * training process. Dedicated method in the LALogger
         * are used for each step.
         */
        std::vector<std::reference_wrapper<Log::LALogger>> loggers;

      public:
        /**
         * \brief Constructor for LearningAgent.
         *
         * \param[in] le The LearningEnvironment for the TPG.
         * \param[in] algorithms vector of algorithms learned by the learning agent
         * \param[in] iSet Set of Instruction used to compose Programs in the
         *            learning process.
         * \param[in] p The LearningParameters for the LearningAgent.
         * \param[in] factory The TPGFactory used to create the TPGGraph. A
         * default TPGFactory is used if none is provided.
         */
        LearningAgent(LearningEnvironment& le, std::vector<std::shared_ptr<Algorithm::Algorithm>> algorithms, const Instructions::Set& iSet,
                      const LearningParameters& p,
                      const TPG::TPGFactory& factory = TPG::TPGFactory())
            : learningEnvironment{le}, algorithms{algorithms},
              env(iSet, p, le.getDataSources(),
                  (le.isDiscrete()) ? 0 : le.getNbActions()),
              archive(p.archiveSize, p.archivingProbability), params{p},
              graph(factory.createTPGGraph(env)) {};

        /**
         * \brief Constructor for LearningAgent.
         *
         * \param[in] le The LearningEnvironment for the TPG.
         * \param[in] algorithm algorithm learned by the learning agent
         * \param[in] iSet Set of Instruction used to compose Programs in the
         *            learning process.
         * \param[in] p The LearningParameters for the LearningAgent.
         * \param[in] factory The TPGFactory used to create the TPGGraph. A
         * default TPGFactory is used if none is provided.
         */
        LearningAgent(LearningEnvironment& le, std::shared_ptr<Algorithm::Algorithm> algorithm, const Instructions::Set& iSet,
                      const LearningParameters& p,
                      const TPG::TPGFactory& factory = TPG::TPGFactory())
            : learningEnvironment{le}, algorithms{{algorithm}},
              env(iSet, p, le.getDataSources(),
                  (le.isDiscrete()) ? 0 : le.getNbActions()),
              archive(p.archiveSize, p.archivingProbability), params{p},
              graph(factory.createTPGGraph(env)) {};

        /// Default destructor for polymorphism
        virtual ~LearningAgent() = default;

        /**
         * \brief Getter for the TPGGraph built by the LearningAgent.
         *
         * \return Get a shared_pointer to the TPGGraph.
         */
        std::shared_ptr<TPG::TPGGraph> getGraph();

        /**
         * \brief Getter for the vector of algorithms
         */
        std::vector<std::shared_ptr<Selector::Selector>> getAlgorithms();

        /**
         * \brief return the a pointer of the algorithm at the specified index
         * 
         * \param[in] idx specified index
         */
        std::shared_ptr<Algorithm::Algorithm> getAlgorithmAt(size_t idx);

        /**
         * \brief Getter for the Archive filled by the LearningAgent
         *
         * \return a const reference to the Archive.
         */
        const Archive& getArchive() const;

        /**
         * \brief Accessor to the Environment of the TPGGraph.
         *
         * \return the const reference to the env attribute.
         */
        const Environment& getEnvironment() const;

        /**
         * \brief Getter for the RNG used by the LearningAgent.
         *
         * \return Get a reference to the RNG.
         */
        RNG::RNG& getRNG();

        /**
         * \brief Adds a LALogger to the loggers vector.
         *
         * Adds a logger to the loggers vector, so that it will be called in
         * addition of the others at some determined moments. This enables to
         * have several loggers that log different things on different outputs
         * simultaneously.
         *
         * \param[in] logger The logger that will be added to the vector.
         */
        void addLogger(Log::LALogger& logger);

        /**
         * \brief Evaluates policy starting from the given root.
         *
         * The policy, that is, the TPGGraph execution starting from the given
         * TPGVertex is evaluated nbIteration times. The generationNumber is
         * combined with the current iteration number to generate a set of
         * seeds for evaluating the policy.
         *
         * The method is const to enable potential parallel calls to it.
         *
         * \param[in] tee The TPGExecutionEngine to use.
         * \param[in] job The job containing the root and archiveSeed for
         * the evaluation.
         * \param[in] generationNumber the integer number of the current
         * generation.
         * \param[in] mode the LearningMode to use during the policy
         * evaluation.
         * \param[in] le Reference to the LearningEnvironment to use
         * during the policy evaluation (may be different from the attribute of
         * the class in child LearningAgentClass).
         *
         * \return a std::shared_ptr to the EvaluationResult for the root. If
         * this root was already evaluated more times then the limit in
         * params.maxNbEvaluationPerPolicy, then the EvaluationResult from the
         * resultsPerRoot map is returned, else the EvaluationResult of the
         * current generation is returned, already combined with the
         * resultsPerRoot for this root (if any).
         */
        virtual std::shared_ptr<EvaluationResult> evaluateJob(
            TPG::TPGExecutionEngine& tee, const Job& job,
            uint64_t generationNumber, LearningMode mode,
            LearningEnvironment& le) const;


        /**
         * \brief Evaluate all agent of the algorithms.
         *
         * This method calls the evaluateJob method for every agent
         * of the algorithms. The method returns a sorted map associating each
         * agent to its average score, in ascending order or score.
         *
         * \param[in] generationNumber the integer number of the current
         * generation.
         * \param[in] mode the LearningMode to use during the policy
         * evaluation.
         */
        virtual std::multimap<std::shared_ptr<EvaluationResult>,
                              std::shared_ptr<const Algorithm::Agent>>
        evaluateAllAgents(uint64_t generationNumber, LearningMode mode);

        /**
         * \brief Evaluate one agent.
         *
         * This method calls the evaluateJob method for a specified Agebt
         * The method returns the average score of this agent.
         *
         * \param[in] generationNumber the integer number of the current
         * generation.
         * \param[in] mode the LearningMode to use during the policy
         * evaluation.
         * \param[in] agent the evaluated Agent.
         * \return the averaged EvaluationResult for the given Agent.
         * \throws an exception in case the given agent does not exist.
         */
        virtual std::shared_ptr<EvaluationResult> evaluateOneAgent(
            uint64_t generationNumber, LearningMode mode,
            std::shared_ptr<const Algorithm::Agent> agent);

        /**
         * \brief Train the TPGGraph for one generation.
         *
         * Training for one generation includes:
         * - Populating the TPGGraph according to given MutationParameters.
         * - Evaluating all agents of the TPGGraph. (call to evaluateAllRoots)
         * - Removing from the Graph and Algorithms the worst performing agents.
         *
         * \param[in] generationNumber the integer number of the current
         * generation.
         * \param[in] doPopulate boolean to indicate if the populateTPG method
         * should be called. This parameter is used to avoid populating at the
         * last generation of a training.
         *
         */
        virtual void trainOneGeneration(uint64_t generationNumber,
                                        bool doPopulate = true);

        /**
         * \brief Train the TPGGraph for a given number of generation.
         *
         * The method trains the TPGGraph for a given number of generation,
         * unless the referenced boolean value becomes false (evaluated at each
         * generation).
         * Optionally, a simple progress bar can be printed within the terminal.
         * The TPGGraph is NOT (re)initialized before starting the training.
         *
         * \param[in] altTraining a reference to a boolean value that can be
         * used to halt the training process before its completion.
         * \param[in] printProgressBar select whether a progress bar will be
         * printed in the console. 
         * \return the number of completed generations.
         */
        virtual uint64_t train(volatile bool& altTraining,
                               bool printProgressBar);

        /**
         * \brief Takes a given Agent and creates a job containing it.
         *
         * \param[in] agent the Agent to be evaluated.
         * \param[in] mode the mode of the training, determining for example
         * if we generate values that we only need for training.
         * \param[in] idx The index of the job, can be used to organize a map
         * for example.
         *
         * \return A job representing the agent.
         */
        virtual std::shared_ptr<Learn::Job> makeJob(
            std::shared_ptr<const Algorithm::Agent> agent, Learn::LearningMode mode, int idx = 0);

        /**
         * \brief Puts all roots into jobs to be able to use them in simulation
         * later.
         *
         * \param[in] mode the mode of the training, determining for example
         * if we generate values that we only need for training.
         *
         * @return A queue containing pointers of the newly created jobs.
         */
        virtual std::queue<std::shared_ptr<Learn::Job>> makeJobs(
            Learn::LearningMode mode);

        /**
         * \brief find the algorithm corresponding to the given agent.
         * 
         * \param[in] agent agent from which the algorithm is found
         * 
         * \throw std::runtime_error if no algorithm contain the agent.
         */
        virtual std::shared_ptr<Algorithm::Algorithm> findCorrespondingAlgorithm(std::shared_ptr<const Algorithm::Agent> agent);

        /**
         * \brief launch the selection of the different algorithms
         * 
         * \param[in] results results of the evaluation
         * \param[in] rng Random Number Generator for this Learning Agent.
         */
        virtual void launchAlgorithmsSelection(
            std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                          std::shared_ptr<const Algorithm::Agent>>& results,
            RNG::RNG& rng);

        /**
         * \brief Initialize the LearningAgent.
         *
         * Calls the TPGMutator::initRandomTPG function.
         * Initialize the RNG::RNG with the given seed.
         * Clears the Archive.
         *
         * \param[in] seed the seed given to the TPGMutator.
         */
        virtual void init(uint64_t seed = 0);
    };
}; // namespace Learn

#endif
