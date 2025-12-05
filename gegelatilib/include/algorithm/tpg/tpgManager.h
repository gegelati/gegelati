
#ifndef TPG_AGENT_MANAGER_H
#define TPG_AGENT_MANAGER_H

#include "algorithm/agentManager.h"
#include "algorithm/tpg/tpgAgent.h"

namespace Algorithm::TPG {


    /**
     * \brief Class representing a TPGManager used by the TPGAlgorithm.
     * 
     * The AgentManager is in charge of storing, creating, copying or removing Agents.
     * Basically, the agentManager is the interface between the Algorithm and the Graph.
     */
    class TPGManager : public AgentManager
    {

    public:

        /**
         * \brief Main TPGManager constructor.
         */
        TPGManager() : AgentManager() {};

        /**
         * \brief Get the current agents used by the algorithm.
         */
        virtual const std::vector<std::shared_ptr<const Agent>> getAgents() const override;

        /**
         * \brief Create a new TPGAgent.
         * 
         * \param[in] graph the Graph associated with the Agent.
         * 
         * \return a shared pointer to the created Agent.
         */
        virtual std::shared_ptr<const Agent> createAgent(std::shared_ptr<EvoGraph::Graph> graph) override;

        /**
         * \brief Copy a TPGAgent.
         * 
         * \param[in] agent the Agent to copy.
         * \param[in] graph the Graph associated with the Agent.
         * 
         * \return a shared pointer to the created Agent.
         */
        virtual std::shared_ptr<const Agent> copyAgent(std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph) override;

        /**
         * \brief Delete the TPGAgent.
         * 
         * \param[in] agent the Agent to delete.
         * \param[in] graph the Graph associated with the Agent.
         * 
         * \return a shared pointer to the created Agent.
         */
        virtual void deleteAgent(std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph) override;
    };
}; // namespace Algorithm

#endif // TPG_AGENT_MANAGER_H
