
#ifndef MUTATOR_H
#define MUTATOR_H

#include <vector>
#include <string>
#include <array>

#include "algorithm/agent.h"
#include "algorithm/agentManager.h"
#include "mutator/rng.h"
#include "learn/learningParameters.h"
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
        uint64_t algorithmID;

        /// Sub-mutators for sub-algorithms
        std::map<uint64_t, std::reference_wrapper<Mutator>> subMutators;

        /// Pointer to the current context used by the mutator to populate the Graph.
        std::unique_ptr<Selector::SelectionContext> currentContext;

        /// Reference to the current context used by the mutator to populate the Graph.
        std::reference_wrapper<const Selector::Selector> selector;

    public:


        // Disable copying to avoid accidental copies (use references or pointers instead).
        Mutator(const Mutator&) = delete;
        Mutator& operator=(const Mutator&) = delete;
    
        /**
         * \brief Main Mutator constructor.
         * 
         * \param[in] selector Reference to the current selector used by the algorithm.
         * \param[in] algorithmID id of the algorithm used.
         */
        Mutator(const Selector::Selector& selector, uint64_t algorithmID) : selector(selector), algorithmID{algorithmID} {};

        /**
         * \brief Update the context used by the TPGMutator to populate the Graph.
         * 
         * \param[in] graph the Graph.
         * \param[in] manager the manager to change the agents.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void updateSpecificContext(
            EvoGraph::Graph& graph, AgentManager& manager,
            const Learn::LearningParameters& params,
            RNG::RNG& rng);

        /**
         * \brief Return a reference to the current context
         */
        virtual const Selector::SelectionContext& getContext();

        /**
         * \brief Return the id of the algorithm.
         */
        uint64_t getAlgorithmID() const { return this->algorithmID; }

        
        /**
         * \brief Add a sub-mutator to the current mutator.
         * 
         * \param[in] subMutator the sub-mutator to add.
         */
        virtual void addSubMutator(Mutator& subMutator);


        /**
         * \brief return the subMutator corresponding to the name of the algorithm given.
         * 
         * \param[in] algorithmID id of the algorithm given.
         */
        virtual Mutator& getSubMutator(uint64_t algorithmID);


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
        virtual std::vector<std::reference_wrapper<const EvoGraph::Action>> initActionVertices(EvoGraph::Graph& graph, size_t nbActionVertices);

        /**
         * \brief Initialize a random population.
         *
         * \param[in,out] graph the initialized Graph.
         * \param[in] manager the manager to change the agents.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void initRandomPopulation(EvoGraph::Graph& graph, AgentManager& manager, const Learn::LearningParameters& params, RNG::RNG& rng) = 0;

        /**
         * \brief Initialize a random Agent.
         *
         * \param[in,out] graph the Graph.
         * \param[in] manager the manager to change the agents.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual const Agent& initRandomAgent(EvoGraph::Graph& graph, AgentManager& manager, const Learn::LearningParameters& params, RNG::RNG& rng);

        
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
            const Agent& agent,
            EvoGraph::Graph& graph,
            AgentManager& manager,
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
            EvoGraph::Graph& graph, AgentManager& manager,
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
            std::array<std::reference_wrapper<const Agent>, 2> agents, EvoGraph::Graph& graph, AgentManager& manager, std::vector<std::reference_wrapper<const Agent>>& newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng
        ) {};

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
            const Agent& agent, EvoGraph::Graph& graph, AgentManager& manager, std::vector<std::reference_wrapper<const Agent>>& newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng) = 0;

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
        virtual void mutateSubAgents(std::vector<std::reference_wrapper<const Agent>>& agents, EvoGraph::Graph& graph, AgentManager& manager, const Learn::LearningParameters& params, RNG::RNG& rng, uint64_t maxNbThreads) {};
    };
}; // namespace Mutator

#endif
