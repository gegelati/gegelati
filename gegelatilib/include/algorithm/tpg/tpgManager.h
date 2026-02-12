
#ifndef TPG_AGENT_MANAGER_H
#define TPG_AGENT_MANAGER_H

#include "algorithm/agentManager.h"
#include "algorithm/tpg/tpgAgent.h"
#include "algorithm/tpg/tpgExecutionEngine.h"

namespace Algorithm::TPG {


    /**
     * \brief Class representing a TPGManager used by the TPGAlgorithm.
     * 
     * The AgentManager is in charge of storing, creating, copying or removing Agents.
     * Basically, the agentManager is the interface between the Algorithm and the Graph.
     */
    class TPGManager : public AgentManager
    {

    protected:
    

        /// Name of the program algorithm associated with the TPG agents.
        std::string programAlgorithmName;


        /**
         * \brief Get the TPGAgent from a const Agent pointer.
         * 
         * \param[in] agent the Agent to cast.
         */
        virtual std::shared_ptr<TPGAgent> getTPGAgentFromCst(const Agent& agent);

    public:

        /**
         * \brief Main TPGManager constructor.
         * 
         * \param[in] outputs outputs of the agents.
         */
        TPGManager(const Output::OutputHandler& outputs) : AgentManager(outputs) {};

        /**
         * \brief Set the name of the program algorithm associated with the TPG agents.
         * 
         * \param[in] name the name of the program algorithm.
         */
        void setProgramAlgorithmName(const std::string& name) { this->programAlgorithmName = name; }

        /**
         * \brief Get the current agents used by the algorithm.
         */
        virtual const std::vector<std::weak_ptr<const Agent>> getAgents() const override;

        /**
         * \brief Create a new TPGAgent.
         * 
         * \param[in] graph the Graph associated with the Agent.
         * 
         * \return a shared pointer to the created Agent.
         */
        virtual std::weak_ptr<const Agent> createAgent(std::shared_ptr<EvoGraph::Graph> graph) override;

        /**
         * \brief Create a new TPGAgent on a specific vertex.
         * The vertex used can only be a EvoGraph::Team
         * 
         * \param[in] vertex the vertex associated with the Agent.
         * 
         * \return a shared pointer to the created Agent.
         */
        virtual std::weak_ptr<const Agent> createAgent(std::shared_ptr<const EvoGraph::Vertex> vertex);


        /**
         * \brief Copy a TPGAgent.
         * 
         * if element is not a nullptr, a copy of the team of the agent is done. 
         * 
         * \param[in] agent the Agent to copy.
         * \param[in] graph the Graph associated with the Agent.
         * 
         * \return a shared pointer to the created Agent.
         */
        virtual std::weak_ptr<const Agent> copyAgent(const Agent& agent, std::shared_ptr<EvoGraph::Graph> graph) override;

        /**
         * \brief Delete the TPGAgent.
         * 
         * \param[in] agent the Agent to delete.
         * \param[in] graph the Graph associated with the Agent.
         * 
         * \return a shared pointer to the created Agent.
         */
        virtual void deleteAgent(const Agent& agent, std::shared_ptr<EvoGraph::Graph> graph) override;


        /**
         * \brief Set the vertex of an agent.
         * 
         * \param[in] agent the Agent to delete.
         * \param[in] graph the Graph associated with the Agent.
         * \param[in] vertex the vertex set to the agent.
         */
        virtual void setVertex(const Agent& agent, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<const EvoGraph::Vertex> vertex);

        /**
         * \brief create and return a TPG execution engine.
         */
        virtual std::unique_ptr<ExecutionEngine> createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources = {}, bool isTraining = false) const override;
    };
}; // namespace Algorithm

#endif // TPG_AGENT_MANAGER_H
