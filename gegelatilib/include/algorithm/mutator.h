
#ifndef MUTATOR_H
#define MUTATOR_H

#include <vector>

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

    public:

        Mutator() {};

        /**
         * \brief Initialize a random Graph.
         *
         * \param[in,out] graph the initialized Graph.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void initRandomPopulation(std::shared_ptr<EvoGraph::Graph> graph, const Learn::LearningParameters& params, RNG::RNG& rng) = 0;

        /**
         * \brief mutate the whole population, by dupplicating and adding new agents from the current algorithm.
         * 
         * \param[in] graph the graph to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] selector the Selector of the learningAgent.
         * \param[in] archive Archive used to assess the uniqueness of the
         *            mutated Program behavior.
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
            const Archive& archive, const Learn::LearningParameters& params,
            RNG::RNG& rng,
            uint64_t maxNbThreads = std::thread::hardware_concurrency());

        /**
         * \brief mutate a specific agent of an algorithm within a population
         * 
         * \param[in,out] agents the Agent to crossover.
         * \param[in,out] graph the graph to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] context context from the selection algorithm.
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void crossoverAgents(
            std::vector<std::shared_ptr<const Agent>> agents, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Selector::SelectionContext& context, const Learn::LearningParameters& params, RNG::RNG& rng
        ) = 0;

        /**
         * \brief mutate a specific agent of an algorithm within a population
         * 
         * \param[in,out] agent the Agent to mutate.
         * \param[in,out] graph the graph to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] context context from the selection algorithm.
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void mutateAgent(
            std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Selector::SelectionContext& context, const Learn::LearningParameters& params, RNG::RNG& rng
        ) = 0;
    };
}; // namespace Mutator

#endif
