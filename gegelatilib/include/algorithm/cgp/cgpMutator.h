
#ifndef CGP_MUTATOR_H
#define CGP_MUTATOR_H

#include <vector>
#include <numeric>

#include "algorithm/lgp/lgpMutator.h"
#include "algorithm/cgp/cgpAgent.h"
#include "algorithm/cgp/cgpManager.h"

namespace Algorithm::CGP {

    /**
     * \brief Class representing a CGPMutator
     */
    class CGPMutator : public LGP::LGPMutator
    {
    protected:

    public:

        /**
         * \brief Main CGPMutator constructor.
         * 
         * \param[in] selector Reference to the current selector used by the algorithm.
         * \param[in] algorithmID id of the algorithm used.
         */
        CGPMutator(const Selector::Selector& selector, uint64_t algorithmID): LGPMutator(selector, algorithmID) {};


        /**
         * \brief Check if the configuration of the mutator is valid according to the given parameters.
         * 
         * This method is called before initializing the population.
         * 
         * \param[in] params the Parameters for the mutation.
         * \param[in] outputs the OutputHandler of the manager.
         */
        virtual bool isConfigurationValid(const Learn::LearningParameters& params, const Output::OutputHandler& outputs) const override;

        /**
         * \brief Initialize a random Agent.
         *
         * \param[in] agent The agent initialized
         * \param[in,out] graph the Graph.
         * \param[in] manager the manager to change the agents.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void initRandomSpecificAgent(const Agent& agent, EvoGraph::Graph& graph, AgentManager& manager, const Learn::LearningParameters& params, RNG::RNG& rng) override;
        
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
        ) override;

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
            const Agent& agent, EvoGraph::Graph& graph, AgentManager& manager, std::vector<std::reference_wrapper<const Agent>>& newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng
        ) override;

        /**
         * \brief mutate a specific CGPagent of an algorithm within a population
         * 
         * \param[in,out] agent the Agent to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual bool mutateCGPAgent(
            const CGPAgent& agent, CGPManager& manager, const Learn::LearningParameters& params, RNG::RNG& rng
        );

        /**
         * \brief Alter a randomly selected Line in a given Program.
         *
         * If the given Program has more than 0 Line, this function selects a
         * Line (pseudo)-randomly in a given Program and calls the
         * Mutator::LineMutator:AlterCorrectLine function on it.
         * Random selection is based on the given RNG::RNG.
         *
         * \param[in,out] agent the Agent to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] rng Random Number Generator used in the mutation process.
         * \return true if a line was successfully altered, false if the
         *         Program has less than one line.
         */
        bool alterRandomLine(const CGPAgent& agent, CGPManager& manager, RNG::RNG& rng);

    };



}; // namespace Mutator

#endif // CGP_MUTATOR_H
