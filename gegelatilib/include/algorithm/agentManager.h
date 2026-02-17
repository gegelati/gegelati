
#ifndef AGENT_MANAGER_H
#define AGENT_MANAGER_H

#include <vector>
#include <memory>
#include <ranges>
#include <string>

#include "evoGraph/graph.h"
#include "algorithm/agent.h"
#include "algorithm/executionEngine.h"
#include "util/genericComparator.h"
#include "outputInfo.h"

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

        /// Number of outputs of the agents
        const Output::OutputHandler& outputs;

        /// Managers of aggregated algorithms
        std::vector<std::reference_wrapper<const AgentManager>> aggregatedManagers;

        /**
         * \brief Get the Agent from a const Agent pointer.
         * 
         * \param[in] agent the Agent to cast.
         */
        virtual std::shared_ptr<Agent> getAgentFromCst(const Agent& agent);

    public:

        /**
         * Constructor for agent manager
         * 
         * \param[in] outputs outputs of the agents
         */
        AgentManager(const Output::OutputHandler& outputs) : outputs{outputs} {}

        /**
         * \brief Get the current agents used by the algorithm.
         */
        virtual const std::vector<std::reference_wrapper<const Agent>> getAgents() const;

        /**
         * \brief Return the outputs of the agents.
         */
        virtual const Output::OutputHandler& getOutputs() const {return outputs; };

        /**
         * \brief Add a sub-manager to the current manager.
         * 
         * \param[in] subManager the sub-manager to add.
         */
        virtual void addSubManager(std::shared_ptr<AgentManager> subManager);

        /**
         * \brief Method that add an access from this manager to another manager.
         * 
         * The manager need to be the same type.
         * This access allows for the manager to dupplicate an agent from the accessed manager to its own agents.
         * 
         * \param[in] managerAggregated the manager to access.
         */
        virtual void addAggregatedManager(const AgentManager& managerAggregated);

        /**
         * \brief return the subManager corresponding to the name of the algorithm given.
         * 
         * \param[in] nameAlgorithm name of the algorithm given.
         */
        virtual std::shared_ptr<AgentManager> getSubManager(std::string nameAlgorithm);

        /**
         * \brief return the subManager corresponding to the name of the algorithm given.
         * 
         * \param[in] nameAlgorithm name of the algorithm given.
         */
        virtual std::shared_ptr<const AgentManager> cGetSubManager(std::string nameAlgorithm) const;

        /**
         * \brief return the aggregated manager corresponding to the name of the algorithm given.
         * 
         * \param[in] nameAlgorithm name of the algorithm given.
         */
        virtual const AgentManager& getAggregatedManager(std::string nameAlgorithm) const;

        /**
         * \brief return the aggregated managers of this manager
         */
        virtual const std::vector<std::reference_wrapper<const AgentManager>>& getAggregatedManagers() const;

        
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
        virtual bool containsAgent(const Agent& agent) const;

        /**
         * \brief method that indicate if the agent is accessible by the manager.
         * 
         * An agent is accessible by the manager if it is created by the manager or by one of its aggregated managers.
         * 
         * \param[in] agent searched agent.
         */
        virtual bool isAgentAccessible(const Agent& agent) const;

        /**
         * \brief Create a new Agent of the type used by the current algorithm.
         * 
         * \param[in] graph the Graph associated with the Agent.
         * 
         * \return a shared pointer to the created Agent.
         */
        virtual const Agent& createAgent(std::shared_ptr<EvoGraph::Graph> graph) = 0;

        /**
         * \brief Copy a new Agent of the type used by the current algorithm.
         * 
         * \param[in] agent the Agent to copy.
         * \param[in] graph the Graph associated with the Agent.
         * 
         * \return a shared pointer to the created Agent.
         */
        virtual const Agent& copyAgent(const Agent& agent, std::shared_ptr<EvoGraph::Graph> graph) = 0;

        /**
         * \brief Create a new Agent of the type used by the current algorithm.
         * 
         * \param[in] agent the Agent to delete.
         * \param[in] graph the Graph associated with the Agent.
         * 
         * \return a shared pointer to the created Agent.
         */
        virtual void deleteAgent(const Agent& agent, std::shared_ptr<EvoGraph::Graph> graph);

        /**
         * \brief Empty an Agent of the type used by the current algorithm.
         * 
         * \param[in] agent the Agent to empty.
         * \param[in] graph the Graph associated with the Agent.
         */
        virtual void emptyAgent(const Agent& agent, std::shared_ptr<EvoGraph::Graph> graph) = 0;

        /**
         * \brief Clear all agents from the manager.
         * 
         * \param[in] graph the Graph associated with the Agents.
         */
        virtual void clearAgents(std::shared_ptr<EvoGraph::Graph> graph);

        /**
         * \brief Create the execution engine associated with the algorithm.
         * 
         * \param[in] dataSources the data sources to use for the execution engine.
         * \param[in] isTraining Boolean indicating if this executionEngine will be executed for training or testing purpose.
         * 
         * \return a shared pointer to the created execution engine.
         */
        virtual std::unique_ptr<ExecutionEngine> createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources = {}, bool isTraining = false) const = 0;
    };
}; // namespace Algorithm

#endif // AGENT_MANAGER_H
