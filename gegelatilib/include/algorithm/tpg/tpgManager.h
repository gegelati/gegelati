
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

        /// Archive used by this TPG
        std::reference_wrapper<Archive> archive;


        /**
         * \brief Get the TPGAgent from a const Agent pointer.
         * 
         * \param[in] agent the Agent to cast.
         */
        virtual std::shared_ptr<TPGAgent> getTPGAgentFromCst(std::shared_ptr<const Agent> agent);

    public:

        /**
         * \brief Main TPGManager constructor.
         * 
         * \param[in] outputs outputs of the agents.
         * \param[in] archive Archive used by this TPG
         */
        TPGManager(const Output::OutputHandler& outputs, Archive& archive) : AgentManager(outputs), archive{archive} {};

        /**
         * \brief Set the name of the program algorithm associated with the TPG agents.
         * 
         * \param[in] name the name of the program algorithm.
         */
        void setProgramAlgorithmName(const std::string& name) { this->programAlgorithmName = name; }

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
         * \brief Create a new TPGAgent on a specific vertex.
         * Used for tests mostly
         * 
         * \param[in] vertex the vertex associated with the Agent.
         * 
         * \return a shared pointer to the created Agent.
         */
        virtual std::shared_ptr<const Agent> createAgent(std::shared_ptr<const EvoGraph::Vertex> vertex);


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


        /**
         * \brief Set the vertex of an agent.
         * 
         * \param[in] agent the Agent to delete.
         * \param[in] graph the Graph associated with the Agent.
         * \param[in] vertex the vertex set to the agent.
         */
        virtual void setVertex(std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<const EvoGraph::Vertex> vertex);

        /**
         * \brief create and return a TPG execution engine.
         */
        virtual std::unique_ptr<ExecutionEngine> createExecutionEngine(bool isTraining = false) const override;
    };
}; // namespace Algorithm

#endif // TPG_AGENT_MANAGER_H
