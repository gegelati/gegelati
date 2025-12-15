
#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <memory>
#include <vector>
#include <string>

#include "algorithm/agent.h"
#include "algorithm/agentManager.h"
#include "algorithm/mutator.h"
#include "learn/learningParameters.h"
#include "evoGraph/graph.h"
#include "selector/selectorFactory.h"
namespace Algorithm {
    /**
     * \brief Abstract class representing an Algorithm.
     *
     * Available algorithms are TPG, MAPLE, and LGP
     */
    class Algorithm
    {
      protected:

        /// Graph used by the algorithm
        std::shared_ptr<EvoGraph::Graph> graph;

        /// Graph used by the algorithm
        std::shared_ptr<AgentManager> manager;

        /// Parameters used by the algorithm
        const Learn::LearningParameters& params;

        /// Selector used for the selection process
        std::shared_ptr<Selector::Selector> selector;
        /// Mutator used by the algorithm
        std::shared_ptr<Mutator> mutator;

        /// Number of values to outputs 
        size_t nbOutputs;

        /// Sub-algorithms used by the algorithm
        std::vector<std::shared_ptr<Algorithm>> subAlgorithms;
        /// Name of the algorithm.
        std::string algorithmName;
        
        /**
         * \brief return the subAlgorithm corresponding to the name of the algorithm given.
         * 
         * \param[in] nameAlgorithm name of the algorithm given.
         */
        std::shared_ptr<Algorithm> getSubAlgorithm(std::string nameAlgorithm);     

        /**
         * \brief return the subAlgorithm corresponding to the name of the algorithm given.
         * 
         * \param[in] nameAlgorithm name of the algorithm given.
         */
        std::shared_ptr<const Algorithm> cGetSubAlgorithm(std::string nameAlgorithm) const;     
      public:

        /**
         * \brief Main Algorithm constructor.
         * 
         * \param[in] params the LearningParameters used by the Algorithm.
         * \param[in] nbOutputs number of outputs that will be usable for
         * interacting with this LearningEnviromnent.
         * \param[in] algorithmName name of the algorithm used.
         * 
         */
        Algorithm(const Learn::LearningParameters& params, size_t nbOutputs, std::string algorithmName)
               : params{params}, nbOutputs{nbOutputs}, algorithmName(algorithmName) {
              };

        /**
         * \brief Return the name of the algorithm.
         */
        std::string getAlgorithmName() const { return this->algorithmName; }

        /**
         * \brief Add a sub-algorithm to the current algorithm.
         * 
         * \param[in] subAlgorithm the sub-algorithm to add.
         */
        void addSubAlgorithm(std::shared_ptr<Algorithm> subAlgorithm);


        /// Constant getter for the manager
        virtual std::shared_ptr<const AgentManager> getManagerCst() const;

        /// Constant getter for the selector
        virtual std::shared_ptr<const Selector::Selector> getSelectorCst() const;

        /// Getter for the manager
        virtual std::shared_ptr<AgentManager> getManager();

        /// Getter for the selector
        virtual std::shared_ptr<Selector::Selector> getSelector();

        /// Getter for the mutator
        virtual std::shared_ptr<Mutator> getMutator();
      
        /**
         * \brief Get the current number of agents used by the algorithm.
         */
        virtual size_t getNbAgents() const;

        /**
         * \brief Get the current agents used by the algorithm.
         */
        virtual const std::vector<std::shared_ptr<const Agent>> getAgents () const;

        /**
         * \brief method that indicate if the algorithm contains a specific agent.
         * 
         * \param[in] agent searched agent.
         */
        virtual bool containsAgent(std::shared_ptr<const Agent> agent) const;

        /**
         * Initialize the algorithm
         * 
         * \param[in] rng deterministic random generator
         * \param[in] le the LearningEnvironment used by the algorithm.
         * \param[in] graph the EvoGraph::Graph used by the algorithm.
         */
        virtual void init(RNG::RNG& rng, Learn::LearningEnvironment& le, std::shared_ptr<EvoGraph::Graph> graph);


        /**
         * \brief Method to start the population process of the algorithm
         * 
         * \param[in] rng deterministic random generator
         * \param[in] maxNbThreads maximum number of threads
         */
        virtual void populate(RNG::RNG& rng, size_t maxNbThreads);


        /**
         * \brief Method detecting whether an agent should be evaluated again.
         *
         * Using the resultsPerAgent map and the
         * params.maxNbEvaluationPerPolicy, this method checks whether an agent
         * should be evaluated again, or if sufficient evaluations were already
         * performed.
         *
         * \param[in] agent The agent whose number of evaluation is
         * checked.
         * \param[out] previousResult the std::shared_ptr to the
         * EvaluationResult of the agent from the resultsPerAgent if any.
         * \return true if the agent has been evaluated enough times, false
         * otherwise.
         */
        virtual bool isAgentEvalSkipped(
            std::shared_ptr<const Agent>,
            std::shared_ptr<Learn::EvaluationResult>& previousResult) const;

        /**
         * \brief Method executing an Agent and outputting action values.
         * 
         * \param[in] agent The agent which is evaluated.
         */
        virtual std::vector<double> executeAgent(std::shared_ptr<const Agent> agent) const = 0;

        /**
         * \brief Clear all the parts of agents that are not used, such as introns for LGPs
         */
        virtual void clearUnusedAgentParts() = 0;


    };
}; // namespace Algorithm

#endif // ALGORITHM_H
