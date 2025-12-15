
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

        /// Attribute that specify if the mutator implements crossover, depending on its algorithm.
        bool isUsingCrossover = false;

        /// Name of the algorithm.
        std::string algorithmName;

        /// Sub-mutators for sub-algorithms
        std::map<std::string, std::shared_ptr<Mutator>> subMutators;

        /// Current context update by the selector in updateSpecificContext method.
        Selector::SelectionContext* currentContext = nullptr;

    public:

        Mutator() {};

        /**
         * \brief Update the context used by the TPGMutator to populate the Graph.
         * 
         * \param[in] graph the Graph.
         * \param[in] manager the manager to change the agents.
         * \param[in] selector the Selector of the learningAgent.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void updateSpecificContext(
            std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, std::shared_ptr<Selector::Selector> selector,
            const Learn::LearningParameters& params,
            RNG::RNG& rng);


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
        virtual std::shared_ptr<const Agent> initRandomAgent(std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Learn::LearningParameters& params, RNG::RNG& rng) = 0;

        /**
         * \brief mutate the whole population, by dupplicating and adding new agents from the current algorithm.
         * 
         * \param[in] graph the graph to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] selector the Selector of the learningAgent.
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
            std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, std::shared_ptr<Selector::Selector> selector,
            const Learn::LearningParameters& params,
            RNG::RNG& rng, uint64_t maxNbThreads = std::thread::hardware_concurrency());
        /**
         * \brief mutate a specific agent of an algorithm within a population
         * 
         * \param[in,out] agents the Agent to crossover.
         * \param[in,out] graph the graph to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] context context from the selection algorithm.
         * \param[in] newSubAgents vector of new agents of sub algorithm created while crossing over the agents
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void crossoverAgents(
            std::vector<std::shared_ptr<const Agent>> agents, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Selector::SelectionContext& context, std::vector<std::shared_ptr<const Agent>> newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng
        ) = 0;

        /**
         * \brief mutate a specific agent of an algorithm within a population
         * 
         * \param[in,out] agent the Agent to mutate.
         * \param[in,out] graph the graph to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] context context from the selection algorithm.
         * \param[in] newSubAgents vector of new agents of sub algorithm created while mutating the agent
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void mutateAgent(
            std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Selector::SelectionContext& context, std::vector<std::shared_ptr<const Agent>> newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng) = 0;
    };
}; // namespace Mutator

#endif
