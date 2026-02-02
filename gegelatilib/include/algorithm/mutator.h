
#ifndef MUTATOR_H
#define MUTATOR_H

#include <vector>
#include <string>

#include "algorithm/agent.h"
#include "algorithm/agentManager.h"
#include "mutator/rng.h"
#include "selector/selector.h"
#include "evoGraph/graph.h"

namespace Algorithm {
    /**
     * \brief Abstract class representing a Mutator used by an Algorithm.
     * 
     * Available algorithms are TPG, MAPLE, and LGP
     */
    class Mutator
    {
    protected:


        /// Name of the algorithm.
        std::string algorithmName;

        /// Sub-mutators for sub-algorithms
        std::map<std::string, std::shared_ptr<Mutator>> subMutators;

        /// Pointer to the current context used by the mutator to populate the Graph.
        std::unique_ptr<Selector::SelectionContext> currentContext;

        /// Reference to the current context used by the mutator to populate the Graph.
        std::reference_wrapper<const Selector::Selector> selector;

    public:

        /**
         * \brief Main Mutator constructor.
         * 
         * \param[in] selector Reference to the current selector used by the algorithm.
         */
        Mutator(const Selector::Selector& selector) : selector(selector) {};

        /**
         * \brief Update the context used by the TPGMutator to populate the Graph.
         * 
         * \param[in] graph the Graph.
         * \param[in] manager the manager to change the agents.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void updateSpecificContext(
            std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager,
            const Learn::LearningParameters& params,
            RNG::RNG& rng);

        /**
         * \brief Return a reference to the current context
         */
        virtual const Selector::SelectionContext& getContext();


        /**
         * \brief Set the name of the algorithm.
         */
        void setAlgorithmName(std::string name) { this->algorithmName = name; }

        /**
         * \brief Return the name of the algorithm.
         */
        std::string getAlgorithmName() const { return this->algorithmName; }

        
        /**
         * \brief Add a sub-mutator to the current mutator.
         * 
         * \param[in] subMutator the sub-mutator to add.
         */
        virtual void addSubMutator(std::shared_ptr<Mutator> subMutator);


        /**
         * \brief return the subMutator corresponding to the name of the algorithm given.
         * 
         * \param[in] nameAlgorithm name of the algorithm given.
         */
        virtual std::shared_ptr<Mutator> getSubMutator(std::string nameAlgorithm);


        /**
         * \brief Check if the configuration of the mutator is valid according to the given parameters.
         * 
         * This method should be call before initializing the population.
         * 
         * \param[in] params the Parameters for the mutation.
         * \param[in] outputs the OutputHandler of the manager.
         */
        virtual bool isConfigurationValid(const Learn::LearningParameters& params, const Output::OutputHandler& outputs) const = 0;

        /**
         * \brief Initialize a the action vertices.
         * 
         * This methods created only the action vertices that does not already exist in the graph
         * 
         * The action vertices are based on the outputs attribute of the manager. 
         * If the outputs contain a single discrete output, an action vertex for each takeable output value is created.
         * If the outputs contain multiple discrete or continuous outputs, an action vertex is created for each output.
         * 
         * However, Gegelati currently does not handle environment with discrete and continuous outputs. 
         * 
         * \param[in,out] graph the initialized Graph.
         * \param[in] nbActionVertices number of agents to create.
         */
        virtual std::vector<std::shared_ptr<const EvoGraph::Action>> initActionVertices(std::shared_ptr<EvoGraph::Graph> graph, size_t nbActionVertices);

        /**
         * \brief Initialize a random population.
         *
         * \param[in,out] graph the initialized Graph.
         * \param[in] manager the manager to change the agents.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void initRandomPopulation(std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Learn::LearningParameters& params, RNG::RNG& rng) = 0;

        /**
         * \brief Initialize a random Agent.
         *
         * \param[in,out] graph the Graph.
         * \param[in] manager the manager to change the agents.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual std::shared_ptr<const Agent> initRandomAgent(std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Learn::LearningParameters& params, RNG::RNG& rng);

        
        /**
         * \brief Initialize a random Agent.
         *
         * \param[in] agent initialized.
         * \param[in,out] graph the Graph.
         * \param[in] manager the manager to change the agents.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void initRandomSpecificAgent(
            std::shared_ptr<const Agent> agent,
            std::shared_ptr<EvoGraph::Graph> graph,
            std::shared_ptr<AgentManager> manager,
            const Learn::LearningParameters& params, RNG::RNG& rng) = 0;

        /**
         * \brief mutate the whole population, by dupplicating and adding new agents from the current algorithm.
         * 
         * \param[in] graph the graph to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         * \param[in] maxNbThreads Integer parameter controlling the number of
         * threads used for parallel execution. Possible values are:
         *   - default:  Let the runtime decide using
         *               std::thread::hardware_concurrency().
         *   - `0` and `1`: Do not use parallelism.
         *   - `n > 1`: Set the number of threads explicitly.
         */
        virtual void mutatePopulation(
            std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager,
            const Learn::LearningParameters& params,
            RNG::RNG& rng, uint64_t maxNbThreads = std::thread::hardware_concurrency());
        /**
         * \brief mutate a specific agent of an algorithm within a population
         * 
         * \param[in,out] agents the Agent to crossover.
         * \param[in,out] graph the graph to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] newSubAgents vector of new agents of sub algorithm created while crossing over the agents
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void crossoverAgents(
            std::vector<std::shared_ptr<const Agent>> agents, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, std::vector<std::shared_ptr<const Agent>>& newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng
        ) = 0;

        /**
         * \brief mutate a specific agent of an algorithm within a population
         * 
         * \param[in,out] agent the Agent to mutate.
         * \param[in,out] graph the graph to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] newSubAgents vector of new agents of sub algorithm created while mutating the agent
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void mutateAgent(
            std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, std::vector<std::shared_ptr<const Agent>>& newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng) = 0;

        /**
         * \brief mutate new sub agents of sub algorithms created during the evolution process of the current algorithm
         * 
         * \param[in] agents vector of new agents of sub algorithm created while mutating the agent
         * \param[in,out] graph the graph to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         * \param[in] maxNbThreads Integer parameter controlling the number of
         * threads used for parallel execution. Possible values are:
         *   - default:  Let the runtime decide using
         *               std::thread::hardware_concurrency().
         *   - `0` and `1`: Do not use parallelism.
         *   - `n > 1`: Set the number of threads explicitly.
         */
        virtual void mutateSubAgents(std::vector<std::shared_ptr<const Agent>>& agents, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Learn::LearningParameters& params, RNG::RNG& rng, uint64_t maxNbThreads) = 0;
    };
}; // namespace Mutator

#endif
