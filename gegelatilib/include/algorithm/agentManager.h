
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
        std::set<std::unique_ptr<Agent>, UniqueLess<Agent>> agents;

        /// Sub-managers for sub-algorithms
        std::map<uint64_t, std::reference_wrapper<AgentManager>> subManagers;

        /// Id of the algorithm.
        uint64_t algorithmID;

        /// Number of outputs of the agents
        const Output::OutputHandler& outputs;

        /// Managers of aggregated algorithms
        std::vector<std::reference_wrapper<const AgentManager>> aggregatedManagers;

        /**
         * \brief Get the Agent from a const Agent pointer.
         * 
         * \param[in] agent the Agent to cast.
         */
        virtual std::set<std::unique_ptr<Agent>>::iterator getAgentFromCst(const Agent& agent);

    public:

    
        /// Default polymorphic destructor
        virtual ~AgentManager() = default;

        // Disable copying to avoid accidental copies (use references or pointers instead).
        AgentManager(const AgentManager&) = delete;
        AgentManager& operator=(const AgentManager&) = delete;

        /**
         * Constructor for agent manager
         * 
         * \param[in] outputs outputs of the agents
         * \param[in] algorithmID id of the algorithm used.
         */
        AgentManager(const Output::OutputHandler& outputs, uint64_t algorithmID) : outputs{outputs}, algorithmID{algorithmID} {}

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
        virtual void addSubManager(AgentManager& subManager);

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
         * \param[in] algorithmID name of the algorithm given.
         */
        virtual AgentManager& getSubManager(uint64_t algorithmID);

        /**
         * \brief return the subManager corresponding to the name of the algorithm given.
         * 
         * \param[in] algorithmID name of the algorithm given.
         */
        virtual const AgentManager& cGetSubManager(uint64_t algorithmID) const;

        /**
         * \brief return the aggregated manager corresponding to the name of the algorithm given.
         * 
         * \param[in] algorithmID name of the algorithm given.
         */
        virtual const AgentManager& getAggregatedManager(uint64_t algorithmID) const;

        /**
         * \brief return the aggregated managers of this manager
         */
        virtual const std::vector<std::reference_wrapper<const AgentManager>>& getAggregatedManagers() const;

        /**
         * \brief Return the id of the algorithm.
         */
        uint64_t getAlgorithmID() const { return this->algorithmID; }

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
        virtual const Agent& createAgent(EvoGraph::Graph& graph) = 0;

        /**
         * \brief Copy a new Agent of the type used by the current algorithm.
         * 
         * \param[in] agent the Agent to copy.
         * \param[in] graph the Graph associated with the Agent.
         * 
         * \return a shared pointer to the created Agent.
         */
        virtual const Agent& copyAgent(const Agent& agent, EvoGraph::Graph& graph) = 0;

        /**
         * \brief Create a new Agent of the type used by the current algorithm.
         * 
         * \param[in] agent the Agent to delete.
         * \param[in] graph the Graph associated with the Agent.
         * 
         * \return a shared pointer to the created Agent.
         */
        virtual void deleteAgent(const Agent& agent, EvoGraph::Graph& graph);

        /**
         * \brief Empty an Agent of the type used by the current algorithm.
         * 
         * \param[in] agent the Agent to empty.
         * \param[in] graph the Graph associated with the Agent.
         */
        virtual void emptyAgent(const Agent& agent, EvoGraph::Graph& graph) = 0;

        /**
         * \brief Clear all agents from the manager.
         * 
         * \param[in] graph the Graph associated with the Agents.
         */
        virtual void clearAgents(EvoGraph::Graph& graph);

        /**
         * \brief Create the execution engine associated with the algorithm.
         * 
         * \param[in] dataSources the data sources to use for the execution engine.
         * \param[in] isTraining Boolean indicating if this executionEngine will be executed for training or testing purpose.
         * 
         * \return a shared pointer to the created execution engine.
         */
        virtual std::unique_ptr<ExecutionEngine> createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources = {}, bool isTraining = false) const = 0;

        /**
         * \brief Set a new ID to an agent
         *
         * An error is thrown if the agent does not belong to the manager
         * An error is thrown if the newID is already used
         *
         * \param[in] agent the agent to change ID
         * \param[in] newID the new ID to set
         */
        virtual void setNewAgentID(const Agent& agent, uint64_t newID);
    };
}; // namespace Algorithm

#endif // AGENT_MANAGER_H
