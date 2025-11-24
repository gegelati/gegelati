
#ifndef AGENT_FACTORY_H
#define AGENT_FACTORY_H

#include <vector>
#include <memory>
#include <ranges>

#include "tpg/tpgGraph.h"
#include "algorithm/agent.h"
#include "util/genericComparator.h"

namespace Algorithm {
    /**
     * \brief Abstract class representing an AgentManager used by an Algorithm.
     * 
     * The AgentManager is in charge of storing, creating, copying or removing Agents.
     * Basically, the agentManager is the interface between the Algorithm and the Graph.
     * 
     * Available algorithms are TPG, MAPLE, and LGP
     */
    class AgentManager
    {
    protected:

        /// Current agents used by the algorithm
        std::set<std::shared_ptr<Agent>, SharedLess<Agent>> agents;

    public:

        AgentManager()
            {};

        /**
         * \brief Get the current agents used by the algorithm.
         */
        virtual const std::vector<std::shared_ptr<Agent>>& getAgents();

        /**
         * \brief Get the current agents used by the algorithm.
         */
        virtual const std::vector<std::shared_ptr<const Agent>> getAgentsCst() const;

        /**
         * \brief Create a new Agent of the type used by the current algorithm.
         * 
         * \param[in] graph the TPGGraph associated with the Agent.
         * 
         * \return a shared pointer to the created Agent.
         */
        virtual std::shared_ptr<Agent> createAgent(std::shared_ptr<TPG::TPGGraph> graph) = 0;

        /**
         * \brief Copy a new Agent of the type used by the current algorithm.
         * 
         * \param[in] agent the Agent to copy.
         * \param[in] graph the TPGGraph associated with the Agent.
         * 
         * \return a shared pointer to the created Agent.
         */
        virtual std::shared_ptr<Agent> copyAgent(std::shared_ptr<const Agent> agent, std::shared_ptr<TPG::TPGGraph> graph) = 0;

        /**
         * \brief Create a new Agent of the type used by the current algorithm.
         * 
         * \param[in] graph the TPGGraph associated with the Agent.
         * 
         * \return a shared pointer to the created Agent.
         */
        virtual std::shared_ptr<Agent> deleteAgent(std::shared_ptr<const Agent> agent, std::shared_ptr<TPG::TPGGraph> graph) = 0;
    };
}; // namespace Algorithm

#endif // AGENT_FACTORY_H
