
#ifndef ATPG_AGENT_MANAGER_H
#define ATPG_AGENT_MANAGER_H

#include "algorithm/tpg/tpgManager.h"
#include "algorithm/atpg/atpgAgent.h"
#include "algorithm/atpg/atpgExecutionEngine.h"

namespace Algorithm::ATPG {


    /**
     * \brief Class representing a TPGManager used by the ActionTPGAlgorithm.
     * 
     * The AgentManager is in charge of storing, creating, copying or removing Agents.
     * Basically, the agentManager is the interface between the Algorithm and the Graph.
     */
    class ATPGManager : public TPG::TPGManager
    {

    protected:
    

        /// Name of the action program algorithm associated with the ATPG agents.
        std::string actionProgramAlgorithmName;


        /**
         * \brief Get the ATPGAgent from a const Agent pointer.
         * 
         * \param[in] agent the Agent to cast.
         */
        virtual std::shared_ptr<ATPGAgent> getATPGAgentFromCst(std::shared_ptr<const Agent> agent);

    public:

        /**
         * \brief Main ATPGManager constructor.
         * 
         * \param[in] outputs outputs of the agents.
         * \param[in] archive Archive used by this TPG
         */
        ATPGManager(const Output::OutputHandler& outputs, Archive& archive) : TPGManager(outputs, archive) {};

        /**
         * \brief Set the name of the action program algorithm associated with the TPG agents.
         * 
         * \param[in] name the name of the action program algorithm.
         */
        void setActionProgramAlgorithmName(const std::string& name) { this->actionProgramAlgorithmName = name; }

        /**
         * \brief Create a new TPGAgent on a specific vertex.
         * Used for tests mostly
         * 
         * \param[in] vertex the vertex associated with the Agent.
         * 
         * \return a shared pointer to the created Agent.
         */
        virtual std::shared_ptr<const Agent> createAgent(std::shared_ptr<const EvoGraph::Vertex> vertex);


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
}; // namespace Algorithm::ATPG

#endif // ATPG_AGENT_MANAGER_H
