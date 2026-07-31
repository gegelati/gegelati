
#ifndef ATPG_AGENT_MANAGER_H
#define ATPG_AGENT_MANAGER_H

#include "representation/tpg/tpgManager.h"
#include "representation/atpg/atpgExecutionEngine.h"

namespace Representation::ATPG {


    /**
     * \brief Class representing a TPGManager used by the ActionTPGRepresentation.
     * 
     * The AgentManager is in charge of storing, creating, copying or removing Agents.
     * Basically, the agentManager is the interface between the Representation and the Graph.
     */
    class ATPGManager : public TPG::TPGManager
    {

    protected:
    

        /// Name of the action program representation associated with the ATPG agents.
        uint64_t actionProgramRepresentationID;

    public:

        /**
         * \brief Main ATPGManager constructor.
         * 
         * \param[in] outputs outputs of the agents.
         * \param[in] representationID id of the representation used.
         */
        ATPGManager(const Output::OutputHandler& outputs, uint64_t representationID) : TPGManager(outputs, representationID) {};

        /**
         * \brief Set the id of the action program representation associated with the TPG agents.
         * 
         * \param[in] id the id of the action program representation.
         */
        void setActionProgramRepresentationID(uint64_t id) { this->actionProgramRepresentationID = id; }


        /**
         * \brief Delete the TPGAgent.
         * 
         * \param[in] agent the Agent to delete.
         * \param[in] graph the Graph associated with the Agent.
         * 
         * \return a shared pointer to the created Agent.
         */
        virtual void emptyAgent(const Agent& agent, EvoGraph::Graph& graph) override;

        /**
         * \brief create and return a TPG execution engine.
         */
        virtual std::unique_ptr<ExecutionEngine> createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources = {}, bool isTraining = false) const override;
    };
}; // namespace Representation::ATPG

#endif // ATPG_AGENT_MANAGER_H
