
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

        /// Vertex from which the species algorithm starts.
        std::optional<std::reference_wrapper<const EvoGraph::Vertex>> rootVertex;

        /// All edges contained in the species
        std::set<std::reference_wrapper<const EvoGraph::Edge>> edges;

        /// All edges contained in the species, containing an agent used for context decisions
        std::set<std::reference_wrapper<const EvoGraph::Edge>> contextEdges;

        /// All edges contained in the species, containing an agent used for action decisions
        std::set<std::reference_wrapper<const EvoGraph::Edge>> actionEdges;

        /// All teams contained in the species.
        std::set<std::reference_wrapper<const EvoGraph::Team>> teams;

        /// All actions contained in the species.
        std::set<std::reference_wrapper<const EvoGraph::Action>> actions;


        
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
         * \brief Recursive method called during setSpeciesGraphStructure to explore the structure of a team.
         * 
         * \param[in] vertex vertex analyzed
         * \param[in] depth current depth in the graph
         */
        void setVertexStructure(const EvoGraph::Vertex& vertex, size_t depth);

        /**
         * \brief Set the graph structure of the species.
         * The root vertex is used to explore the graph structure and fill the edges and vertices set.
         * 
         * \param[in] rootVertex the root vertex to start the exploration of the graph structure
         */
        virtual void setSpeciesGraphStructure(const EvoGraph::Vertex& rootVertex);


        /// @brief getter for the edges 
        const std::set<std::reference_wrapper<const EvoGraph::Edge>>& getEdges() const;

        /// @brief getter for the context edges 
        const std::set<std::reference_wrapper<const EvoGraph::Edge>>& getContextEdges() const;

        /// @brief getter for the action edges 
        const std::set<std::reference_wrapper<const EvoGraph::Edge>>& getActionEdges() const;

        /// @brief getter for the teams
        const std::set<std::reference_wrapper<const EvoGraph::Team>>& getTeams() const;

        /// @brief getter for the actions 
        const std::set<std::reference_wrapper<const EvoGraph::Action>>& getActions() const;

        /**
         * \brief Create a new SpeciesAgent.
         * 
         * \param[in] graph the Graph associated with the Agent.
         * 
         * \return the created Agent.
         */
        virtual const Agent& createAgent(EvoGraph::Graph& graph) override;

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
         * \brief Empty a SpeciesAgent of its program.
         * 
         * \param[in] agent the Agent to empty.
         * \param[in] graph the Graph associated with the Agent.
         */
        virtual void emptyAgent(const Agent& agent, EvoGraph::Graph& graph) override;

        /**
         * \brief Set a specific program to the edge of an agent.
         * 
         * \param[in] agent the speciesAgent on which the program is set
         * \param[in] edge the edge link to the program set
         * \param[in] program the program set.
         */
        virtual void setProgram(const Agent& agent, const EvoGraph::Edge& edge, const Agent& program);

        /**
         * \brief create and return a Species execution engine.
         */
        virtual std::unique_ptr<ExecutionEngine> createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources = {}, bool isTraining = false) const override;
    };
}; // namespace Algorithm

#endif // SPECIES_AGENT_MANAGER_H
