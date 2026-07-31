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

#include "representation/representation.h"

#include "log/laLogger.h"
#include "mutator/mutationParameters.h"
#include "evoGraph/graph.h"

#include "learn/classificationLearningEnvironment.h"
#include "learn/evaluationResult.h"
#include "learn/learningEnvironment.h"
#include "learn/learningParameters.h"

namespace Learn {

    /**
     * \brief Class used to control the learning steps of a Graph within
     * a given LearningEnvironment.
     */
    class LearningAgent
    {
      protected:
        /// LearningEnvironment with which the LearningAgent will interact.
        LearningEnvironment& learningEnvironment;

        /// Map of reference of representations learned by the learning agent.
        std::map<uint64_t, std::reference_wrapper<Representation::Representation>> representations;

        /// Parameters for the learning process
        std::unique_ptr<LearningParameters> params;

        /// Graph built during the learning process.
        std::shared_ptr<EvoGraph::Graph> graph;

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

        /// Currently executed representation during evaluation
        Representation::Representation* currentExecutedRepresentation;

        /// Currently best representation during evaluation
        Representation::Representation* currentBestRepresentation;

        /**
         * \brief return the representation managed by the learning agent corresponding to the given representation.
         * 
         * \param[in] representation the representation to get.
         */
        Representation::Representation& getRepresentation(const Representation::Representation& representation);

      public:
        /**
         * \brief Constructor for LearningAgent.
         *
         * \param[in] le The LearningEnvironment for the TPG.
         * \param[in] representations vector of representations learned by the learning agent
         * \param[in] parameters The LearningParameters for the LearningAgent.
         * \param[in] factory The GraphFactory used to create the Graph. A
         * default GraphFactory is used if none is provided.
         */
        LearningAgent(LearningEnvironment& le, std::vector<std::reference_wrapper<Representation::Representation>> representations,
                      std::unique_ptr<LearningParameters> parameters = std::make_unique<LearningParameters>(),
                      const EvoGraph::GraphFactory& factory = EvoGraph::GraphFactory())
            : learningEnvironment{le},
              params{std::make_unique<LearningParameters>(*parameters)}, graph(factory.createGraph()) {
                for(Representation::Representation& representation: representations){
                    this->representations.insert({representation.getRepresentationID(), representation});
                }
              };

        /**
         * \brief Constructor for LearningAgent.
         *
         * \param[in] le The LearningEnvironment for the TPG.
         * \param[in] representation representation learned by the learning agent
         * \param[in] parameters The LearningParameters for the LearningAgent.
         * \param[in] factory The GraphFactory used to create the Graph. A
         * default GraphFactory is used if none is provided.
         */
        LearningAgent(LearningEnvironment& le, Representation::Representation& representation,
                      std::unique_ptr<LearningParameters> parameters = std::make_unique<LearningParameters>(),
                      const EvoGraph::GraphFactory& factory = EvoGraph::GraphFactory())
            : LearningAgent(le, std::vector<std::reference_wrapper<Representation::Representation>>{representation}, std::move(parameters), factory) {};

        /// Default destructor for polymorphism
        virtual ~LearningAgent() = default;

        /**
         * \brief set a new number of generation (mainly used for validation purpose)
         * 
         * \param[in] gen new number of generation
         */
        void setNbGen(size_t gen);

        /**
         * \brief Set the current executed representation during evaluation
         * 
         * \param[in] representation the representation to set as current executed representation
         */
        void setCurrentRepresentation(Representation::Representation* representation);

        /**
         * \brief Add an representation to the learning agent.
         * 
         * \param[in] representation the representation to add.
         */
        void addRepresentation(Representation::Representation& representation);


        /**
         * \brief Getter for the Graph built by the LearningAgent.
         *
         * \return Get a shared_pointer to the Graph.
         */
        EvoGraph::Graph& getGraph();

        /**
         * \brief Getter for the vector of representations
         */
        std::vector<std::reference_wrapper<const Representation::Representation>> cGetRepresentations() const;

        /**
         * \brief Getter for the vector of representations
         */
        std::vector<std::reference_wrapper<Representation::Representation>> getRepresentations();

        /**
         * \brief return the a pointer of the representation at the specified index
         * 
         * \param[in] id specified index
         */
        Representation::Representation& getRepresentationAt(size_t id);

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
         * The policy, that is, the Graph execution starting from the given
         * Vertex is evaluated nbIteration times. The generationNumber is
         * combined with the current iteration number to generate a set of
         * seeds for evaluating the policy.
         *
         * The method is const to enable potential parallel calls to it.
         *
         * \param[in] execEngine The ExecutionEngine to use.
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
            Representation::ExecutionEngine& execEngine, const Representation::Job& job,
            uint64_t generationNumber, LearningMode mode,
            LearningEnvironment& le) const;


        /**
         * \brief Evaluate all agent of the representations.
         *
         * This method calls the evaluateJob method for every agent
         * of the representations. The method returns a sorted map associating each
         * agent to its average score, in ascending order or score.
         *
         * \param[in] generationNumber the integer number of the current
         * generation.
         * \param[in] mode the LearningMode to use during the policy
         * evaluation.
         */
        virtual std::multimap<std::shared_ptr<EvaluationResult>,
                              std::reference_wrapper<const Representation::Individual>>
        evaluateAllAgents(uint64_t generationNumber, LearningMode mode);

        /**
         * \brief Evaluate all agents of one representation.
         *
         * This method calls the evaluateJob method for every agent
         * of one representation. The method returns a sorted map associating each
         * agent to its average score, in ascending order or score.
         *
         * \param[in] generationNumber the integer number of the current
         * generation.
         * \param[in] mode the LearningMode to use during the policy
         * evaluation.
         */
        virtual std::multimap<std::shared_ptr<EvaluationResult>,
                              std::reference_wrapper<const Representation::Individual>>
        evaluateCurrentRepresentationAgents(uint64_t generationNumber, LearningMode mode);

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
         * \param[in] agent the evaluated Individual.
         * \return the averaged EvaluationResult for the given Individual.
         * \throws an exception in case the given agent does not exist.
         */
        virtual std::shared_ptr<EvaluationResult> evaluateOneAgent(
            uint64_t generationNumber, LearningMode mode,
            const Representation::Individual& agent);

        /**
         * \brief Train the Graph for one generation.
         *
         * Training for one generation includes:
         * - Populating the Graph according to given MutationParameters.
         * - Evaluating all agents of the Graph. (call to evaluateAllRoots)
         * - Removing from the Graph and Representations the worst performing agents.
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
         * \brief Train the Graph for a given number of generation.
         *
         * The method trains the Graph for a given number of generation,
         * unless the referenced boolean value becomes false (evaluated at each
         * generation).
         * Optionally, a simple progress bar can be printed within the terminal.
         * The Graph is NOT (re)initialized before starting the training.
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
         * \brief Puts all roots into jobs to be able to use them in simulation
         * later.
         *
         * \param[in] mode the mode of the training, determining for example
         * if we generate values that we only need for training.
         *
         * @return A vector containing pointers of the newly created jobs.
         */
        virtual std::vector<std::shared_ptr<Representation::Job>> makeJobs(
            Learn::LearningMode mode);


        /**
         * \brief launch the selection of the different representations
         * 
         * \param[in] results results of the evaluation
         * \param[in] rng Random Number Generator for this Learning Agent.
         */
        virtual void launchRepresentationsSelection(
            std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                          std::reference_wrapper<const Representation::Individual>>& results,
            RNG::RNG& rng);

        /**
         * \brief Initialize the LearningAgent.
         *
         * Calls the TPGMutator::initRandomTPG function.
         * Initialize the RNG::RNG with the given seed.
         * Clears the Archive.
         *
         * \param[in] seed the seed given to the TPGMutator.
         * \param[in] doGeneratePopulation boolean to indicate if population of the representations should be generated here.
         */
        virtual void init(uint64_t seed = 0, bool doGeneratePopulation = true);


        /**
         * \brief Return the current best representation
         */
        const Representation::Representation& getBestRepresentation();

        /**
         * \brief Method that indicate if the learning agent contains a specific representation.
         * 
         * \param[in] representation the representation to search.
         */
        virtual bool containsRepresentation(Representation::Representation& representation);
    };
}; // namespace Learn

#endif
