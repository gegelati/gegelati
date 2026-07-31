
#ifndef MAPLE_AGENT_MANAGER_H
#define MAPLE_AGENT_MANAGER_H

#include "representation/maple/mapleAgent.h"
#include "representation/maple/mapleExecutionEngine.h"

namespace Representation::Maple {


    /**
     * \brief Class representing a MapleManager used by the MapleRepresentation.
     * 
     * The AgentManager is in charge of storing, creating, copying or removing Agents.
     * Basically, the agentManager is the interface between the Representation and the Graph.
     */
    class MapleManager : public TPG::TPGManager
    {

    protected:

        /**
         * \brief Get the MapleAgent from a const Agent pointer.
         * 
         * \param[in] agent the Agent to cast.
         */
        virtual MapleAgent& getMapleAgentFromCst(const Agent& agent);
    public:

        /**
         * \brief Main MapleManager constructor.
         * 
         * \param[in] outputs outputs of the agents.
         * \param[in] representationID id of the representation used.
         */
        MapleManager(const Output::OutputHandler& outputs, uint64_t representationID) : TPGManager(outputs, representationID) {};


        /**
         * \brief Get the current agents used by the representation.
         */
        virtual const std::vector<std::reference_wrapper<const Agent>> getAgents() const override;

        /**
         * \brief Create a new MapleAgent on a specific vertex.
         * 
         * \param[in] vertex the vertex associated with the Agent.
         * 
         * \return a shared pointer to the created Agent.
         */
        virtual const Agent& createAgent(std::optional<std::reference_wrapper<const EvoGraph::Vertex>> vertex) override;

        /**
         * \brief create and return a Maple execution engine.
         */
        virtual std::unique_ptr<ExecutionEngine> createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources = {}, bool isTraining = false) const override;
    };
}; // namespace Representation::MAPLE

#endif // MAPLE_AGENT_MANAGER_H
