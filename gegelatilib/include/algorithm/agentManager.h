
#ifndef AGENT_FACTORY_H
#define AGENT_FACTORY_H

#include <vector>
#include <memory>
#include <ranges>
#include <string>

#include "evoGraph/graph.h"
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

        /// Sub-managers for sub-algorithms
        std::map<std::string, std::shared_ptr<AgentManager>> subManagers;

        /// Name of the algorithm.
        std::string algorithmName;

    public:

        /**
         * \brief Get the current agents used by the algorithm.
         */
        virtual const std::vector<std::shared_ptr<const Agent>> getAgents() const;


        /**
         * \brief Add a sub-manager to the current manager.
         * 
         * \param[in] subManager the sub-manager to add.
         */
        virtual void addSubManager(std::shared_ptr<AgentManager> subManager);

        /**
         * \brief return the subManager corresponding to the name of the algorithm given.
         * 
         * \param[in] nameAlgorithm name of the algorithm given.
         */
        virtual std::shared_ptr<AgentManager> getSubManager(std::string nameAlgorithm);

        /**
         * \brief Set the name of the algorithm.
         */
        void setAlgorithmName(std::string name) { this->algorithmName = name; }

        /**
         * \brief Return the name of the algorithm.
         */
        std::string getAlgorithmName() const { return this->algorithmName; }

        /**
         * \brief method that indicate if the manager contains a specific agent.
         * 
         * \param[in] agent searched agent.
         */
        virtual bool containsAgent(std::shared_ptr<const Agent> agent) const;

        /**
         * \brief Create a new Agent of the type used by the current algorithm.
         * 
         * \param[in] graph the Graph associated with the Agent.
         * 
         * \return a shared pointer to the created Agent.
         */
        virtual std::shared_ptr<const Agent> createAgent(std::shared_ptr<EvoGraph::Graph> graph) = 0;

        /**
         * \brief Copy a new Agent of the type used by the current algorithm.
         * 
         * \param[in] agent the Agent to copy.
         * \param[in] graph the Graph associated with the Agent.
         * 
         * \return a shared pointer to the created Agent.
         */
        virtual std::shared_ptr<const Agent> copyAgent(std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph) = 0;

        /**
         * \brief Create a new Agent of the type used by the current algorithm.
         * 
         * \param[in] agent the Agent to delete.
         * \param[in] graph the Graph associated with the Agent.
         * 
         * \return a shared pointer to the created Agent.
         */
        virtual void deleteAgent(std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph) = 0;

        /**
         * \brief Clear all agents from the manager.
         */
        virtual void clearAgents();
    };
}; // namespace Algorithm

#endif // AGENT_MANAGER_H
