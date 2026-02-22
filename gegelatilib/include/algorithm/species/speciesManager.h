
#ifndef SPECIES_AGENT_MANAGER_H
#define SPECIES_AGENT_MANAGER_H

#include "algorithm/agentManager.h"
#include "algorithm/species/speciesAgent.h"
#include "algorithm/species/speciesExecutionEngine.h"

namespace Algorithm::Species {


    /**
     * \brief Class representing a SpeciesManager used by the SpeciesAlgorithm.
     * 
     * The AgentManager is in charge of storing, creating, copying or removing Agents.
     * Basically, the agentManager is the interface between the Algorithm and the Graph.
     */
    class SpeciesManager : public AgentManager
    {

    protected:
    

        /// id of the program algorithm associated with the Species agents.
        uint64_t programAlgorithmID;


        /**
         * \brief Get the SpeciesAgent from a const Agent pointer.
         * 
         * \param[in] agent the Agent to cast.
         */
        virtual SpeciesAgent& getSpeciesAgentFromCst(const Agent& agent);

    public:

        /**
         * \brief Main SpeciesManager constructor.
         * 
         * \param[in] outputs outputs of the agents.
         * \param[in] algorithmID id of the algorithm used.
         */
        SpeciesManager(const Output::OutputHandler& outputs, uint64_t algorithmID) : AgentManager(outputs, algorithmID) {};

        /**
         * \brief Set the id of the program algorithm associated with the Species agents.
         * 
         * \param[in] id the id of the program algorithm.
         */
        void setProgramAlgorithmID(uint64_t id) { this->programAlgorithmID = id; }

        /**
         * \brief Get the current agents used by the algorithm.
         */
        virtual const std::vector<std::reference_wrapper<const Agent>> getAgents() const override;

        /**
         * \brief Create a new SpeciesAgent.
         * 
         * \param[in] graph the Graph associated with the Agent.
         * 
         * \return the created Agent.
         */
        virtual const Agent& createAgent(EvoGraph::Graph& graph) override;

        /**
         * \brief Create a new SpeciesAgent on a specific vertex.
         * The vertex used can only be a EvoGraph::Team
         * 
         * \param[in] vertex the vertex associated with the Agent.
         * 
         * \return the created Agent.
         */
        virtual const Agent& createAgent(std::optional<std::reference_wrapper<const EvoGraph::Vertex>> vertex);

        /**
         * \brief Create a new SpeciesAgent with no vertex. 
         * This method is made only for readAgent during loading a .dot file.
         * 
         * \return the created Agent.
         */
        virtual const Agent& createEmptyAgent();

        /**
         * \brief Copy a SpeciesAgent.
         * 
         * if element is not a nullptr, a copy of the team of the agent is done. 
         * 
         * \param[in] agent the Agent to copy.
         * \param[in] graph the Graph associated with the Agent.
         * 
         * \return the created Agent.
         */
        virtual const Agent& copyAgent(const Agent& agent, EvoGraph::Graph& graph) override;

        /**
         * \brief Delete the SpeciesAgent.
         * 
         * \param[in] agent the Agent to delete.
         * \param[in] graph the Graph associated with the Agent.
         */
        virtual void deleteAgent(const Agent& agent, EvoGraph::Graph& graph) override;

        /**
         * \brief Empty a SpeciesAgent of its program.
         * 
         * \param[in] agent the Agent to empty.
         * \param[in] graph the Graph associated with the Agent.
         */
        virtual void emptyAgent(const Agent& agent, EvoGraph::Graph& graph) override;

        /**
         * \brief Set the vertex of an agent.
         * 
         * \param[in] agent the Agent to delete.
         * \param[in] vertex the vertex set to the agent.
         */
        virtual void setVertex(const Agent& agent, const EvoGraph::Vertex& vertex);

        /**
         * \brief create and return a Species execution engine.
         */
        virtual std::unique_ptr<ExecutionEngine> createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources = {}, bool isTraining = false) const override;
    };
}; // namespace Algorithm

#endif // SPECIES_AGENT_MANAGER_H
