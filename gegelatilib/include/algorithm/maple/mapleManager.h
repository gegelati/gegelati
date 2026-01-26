
#ifndef MAPLE_AGENT_MANAGER_H
#define MAPLE_AGENT_MANAGER_H

#include "algorithm/tpg/tpgManager.h"
#include "algorithm/maple/mapleAgent.h"
#include "algorithm/maple/mapleExecutionEngine.h"

namespace Algorithm::Maple {


    /**
     * \brief Class representing a MapleManager used by the MapleAlgorithm.
     * 
     * The AgentManager is in charge of storing, creating, copying or removing Agents.
     * Basically, the agentManager is the interface between the Algorithm and the Graph.
     */
    class MapleManager : public TPG::TPGManager
    {

    protected:

        /**
         * \brief Get the MapleAgent from a const Agent pointer.
         * 
         * \param[in] agent the Agent to cast.
         */
        virtual std::shared_ptr<MapleAgent> getMapleAgentFromCst(std::shared_ptr<const Agent> agent);
    public:

        /**
         * \brief Main TPGManager constructor.
         * 
         * \param[in] outputs outputs of the agents.
         * \param[in] archive Archive used by this Maple
         */
        MapleManager(const Output::OutputHandler& outputs, Archive& archive) : TPGManager(outputs, archive) {};


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
         * \brief Delete the TPGAgent.
         * 
         * \param[in] agent the Agent to delete.
         * \param[in] graph the Graph associated with the Agent.
         * 
         * \return a shared pointer to the created Agent.
         */
        virtual void deleteAgent(std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph) override;


        /**
         * \brief create and return a TPG execution engine.
         */
        virtual std::unique_ptr<ExecutionEngine> createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources = {}, bool isTraining = false) const override;
    };
}; // namespace Algorithm::MAPLE

#endif // MAPLE_AGENT_MANAGER_H
