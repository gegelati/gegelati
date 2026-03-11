
#ifndef SPECIES_MUTATOR_H
#define SPECIES_MUTATOR_H

#include <vector>
#include <numeric>
#include <queue>
#include <mutex>
#include <array>
#include <map>


#include "algorithm/mutator.h"
#include "algorithm/species/speciesAgent.h"
#include "algorithm/species/speciesManager.h"

namespace Algorithm::Species {

    /**
     * \brief Class representing a SpeciesMutator
     */
    class SpeciesMutator : public Mutator
    {
    protected:

        /// ID of the program algorithm associated with the Species agents.
        uint64_t programAlgorithmID;

        /// Pre-existing teams elements used for mutation operations.
        std::vector<std::reference_wrapper<const EvoGraph::Vertex>> preExistingTeams;

        /// Pre-existing actions elements used for mutation operations.
        std::vector<std::reference_wrapper<const EvoGraph::Vertex>> preExistingActions;

        /// Pre-existing edges used for mutation operations.
        std::vector<std::reference_wrapper<const EvoGraph::Edge>> preExistingEdges;

        /// Archive used by this Species
        std::reference_wrapper<const Archive> archive;


        /// Set containing the new edges created specifically for this species.
        std::set<std::reference_wrapper<const EvoGraph::Edge>> newEdges;

        /// boolean to indicate if only the new edges created specifically for the species or not should be mutated
        bool doSpecificEdgesToMutate = false;

    public:

        /**
         * \brief Constructor for SpeciesMutator
         * 
         * \param[in] selector Reference to the current selector used by the algorithm.
         * \param[in] algorithmID id of the algorithm used.
         * \param[in] archive Archive used by this Species
         */
        SpeciesMutator(const Selector::Selector& selector, uint64_t algorithmID, const Archive& archive): Mutator(selector, algorithmID), archive{archive} {};

        /**
         * \brief set the set of new edges created specifically for this species.
         */
        void setNewEdges(const std::set<std::reference_wrapper<const EvoGraph::Edge>>& newEdges) {this->newEdges = newEdges;}

        /**
         * \brief Set doSpecificEdgesToMutate attribute
         */
        void setSpecificEdgesToMutate(bool doSpecificEdgesToMutate) {this->doSpecificEdgesToMutate = doSpecificEdgesToMutate;}

        /**
         * \brief return if the mutator has doSpecificEdgesToMutate set to true
         */
        bool isDoingSpecificEdgesToMutate() {return this->doSpecificEdgesToMutate;}

        /**
         * \brief Set the id of the program algorithm associated with the Species agents.
         * 
         * \param[in] id the id of the program algorithm.
         */
        void setProgramAlgorithmID(uint64_t id) { this->programAlgorithmID = id; }

        /**
         * \brief Get the name of the program algorithm associated with the Species agents.
         * 
         * \return the name of the program algorithm.
         */
        uint64_t getProgramAlgorithmID() const { return this->programAlgorithmID; }

        /**
         * \brief Check if the configuration of the mutator is valid according to the given parameters.
         * 
         * This method is called before initializing the population.
         * 
         * \param[in] params the Parameters for the mutation.
         * \param[in] outputs the OutputHandler of the manager.
         */
        virtual bool isConfigurationValid(const Learn::LearningParameters& params, const Output::OutputHandler& outputs) const override;

        /**
         * \brief Init the graph structure of the species.
         * 
         * return the root of the structure.
         * 
         * \param[in,out] graph the initialized Graph.
         * \param[in] manager the manager to change the agents.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual const EvoGraph::Team& initSpeciesGraphStructure(EvoGraph::Graph& graph, AgentManager& manager, const Learn::LearningParameters& params, RNG::RNG& rng);

        /**
         * \brief add an activationedge to the new species.
         * 
         * \param[in] newRoot The new root vertex of the species
         * \param[in] manager the manager to copy the graph.
         * \param[in,out] graph the Graph.
         * \param[in] rng Random Number Generator used in the mutation process.
         * \param[in,out] edgeMap map linking old to new edges
         */
        bool addActivationEdgeSpecies(const EvoGraph::Vertex& newRoot, AgentManager& manager, EvoGraph::Graph& graph, RNG::RNG& rng, std::map<std::reference_wrapper<const EvoGraph::Edge>, std::reference_wrapper<const EvoGraph::Edge>>& edgeMap);

        /**
         * \brief add an context edge to the new species.
         * 
         * \param[in] newRoot The new root vertex of the species
         * \param[in] manager the manager to copy the graph.
         * \param[in,out] graph the Graph.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         * \param[in,out] edgeMap map linking old to new edges
         * \param[in,out] newEdges set of new edges created by the speceis
         */
        bool addContextEdgeSpecies(const EvoGraph::Vertex& newRoot, AgentManager& manager, EvoGraph::Graph& graph, const Learn::LearningParameters& params, RNG::RNG& rng, std::map<std::reference_wrapper<const EvoGraph::Edge>, std::reference_wrapper<const EvoGraph::Edge>>& edgeMap, std::set<std::reference_wrapper<const EvoGraph::Edge>>& newEdges);

        /**
         * \brief remove an edge to the new species.
         * 
         * \param[in] newRoot The new root vertex of the species
         * \param[in] manager the manager to copy the graph.
         * \param[in,out] graph the Graph.
         * \param[in] rng Random Number Generator used in the mutation process.
         * \param[in,out] edgeMap map linking old to new edges
         */
        virtual bool removeEdgeSpecies(const EvoGraph::Vertex& newRoot, AgentManager& manager, EvoGraph::Graph& graph, RNG::RNG& rng, std::map<std::reference_wrapper<const EvoGraph::Edge>, std::reference_wrapper<const EvoGraph::Edge>>& edgeMap);

        
        /**
         * \brief Extend the species.
         * Extension select a random activation vertex A, and add a decision vertex to it, with two context edge leading to new activation vertex A' and A''. 
         * Some random activation edges of A are moved from A to A' and A''.
         * 
         * This mecanism adds emergency, while keeping the sub-task division at action level, and while ensuring that each action is always activated at max one time.
         * 
         * \param[in] newRoot The new root vertex of the species
         * \param[in] manager the manager to copy the graph.
         * \param[in,out] graph the Graph.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         * \param[in,out] edgeMap map linking old to new edges
         * \param[in,out] newEdges set of new edges created by the speceis
         */
        virtual bool extendSpecies(const EvoGraph::Vertex& newRoot, AgentManager& manager, EvoGraph::Graph& graph, const Learn::LearningParameters& params, RNG::RNG& rng, std::map<std::reference_wrapper<const EvoGraph::Edge>, std::reference_wrapper<const EvoGraph::Edge>>& edgeMap, std::set<std::reference_wrapper<const EvoGraph::Edge>>& newEdges);

        /**
         * Mutate the graph of a species to create a new species. 
         * This method first calls copyGraphSpecies(), and then do a random mutation on the graph.
         * 
         * \param[in,out] graph the Graph.
         * \param[in] manager the manager to copy the graph.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         * \param[in,out] edgeMap map linking old to new edges
         * \param[in,out] newEdges set of new edges created by the speceis
         */
        virtual const EvoGraph::Vertex& mutateSpeciesGraph(EvoGraph::Graph& graph, AgentManager& manager, const Learn::LearningParameters& params, RNG::RNG& rng, std::map<std::reference_wrapper<const EvoGraph::Edge>, std::reference_wrapper<const EvoGraph::Edge>>& edgeMap, std::set<std::reference_wrapper<const EvoGraph::Edge>>& newEdges);

        /**
         * \brief Dupplicate the graph from a specific root
         * 
         * \param[in] manager the manager to copy the graph.
         * \param[in,out] graph the initialized Graph.
         * \param[in,out] edgeMap map linking new to old edges
         */
        virtual const EvoGraph::Vertex& copyGraphSpecies(AgentManager& manager, EvoGraph::Graph& graph, std::map<std::reference_wrapper<const EvoGraph::Edge>, std::reference_wrapper<const EvoGraph::Edge>>& edgeMap);

        /**
         * \brief Initialize an agent from another agent of a different species.
         * 
         * The new agent will make copy of each program corresponding to the same edges
         *
         * \param[in] agent the agent initialized.
         * \param[in,out] graph the Graph.
         * \param[in] manager the manager to init the agents.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         * \param[in,out] edgeMap map linking new to old edges
         */
        virtual void initAgentFromSpecies(const Agent& agent, EvoGraph::Graph& graph, AgentManager& manager, const Learn::LearningParameters& params, RNG::RNG& rng, std::map<std::reference_wrapper<const EvoGraph::Edge>, std::reference_wrapper<const EvoGraph::Edge>>& edgeMap);

        /**
         * \brief Initialize Species Population.
         *
         * \param[in,out] graph the initialized Graph.
         * \param[in] manager the manager to change the agents.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void initRandomPopulation(EvoGraph::Graph& graph, AgentManager& manager, const Learn::LearningParameters& params, RNG::RNG& rng) override;


        /**
         * \brief Initialize a random Agent.
         *
         * \param[in] agent the agent initialized.
         * \param[in,out] graph the Graph.
         * \param[in] manager the manager to change the agents.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void initRandomSpecificAgent(const Agent& agent, EvoGraph::Graph& graph, AgentManager& manager, const Learn::LearningParameters& params, RNG::RNG& rng) override;

        
        /**
         * \brief Cross the two program at the specific edge
         * 
         * \param[in,out] agents the Agent to crossover.
         * \param[in] edge the edge specified
         * \param[in,out] graph the graph to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] newSubAgents vector of new agents of sub algorithm created while crossing over the agents
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void crossoverPrograms(
            std::array<std::reference_wrapper<const Agent>, 2> agents, const EvoGraph::Edge& edge, EvoGraph::Graph& graph, AgentManager& manager, std::vector<std::reference_wrapper<const Agent>>& newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng);


        /**
         * \brief Cross the two program at the specific edges
         * 
         * \param[in,out] agents the Agent to crossover.
         * \param[in] edge the edge specified
         * \param[in] manager the manager to change the agents.
         * \param[in] newSubAgents vector of new agents of sub algorithm created while crossing over the agents
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void crossoverEdges(
            std::array<std::reference_wrapper<const Agent>, 2> agents, const EvoGraph::Edge& edge, AgentManager& manager, std::vector<std::reference_wrapper<const Agent>>& newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng);


        /**
         * \brief Do a crossover over two maple agents, by either crossover the program on edges, or crossover the edges. Calling either crossoverPrograms or crossoverEdges methods
         * 
         * \param[in,out] agents the Agent to crossover.
         * \param[in,out] graph the graph to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] newSubAgents vector of new agents of sub algorithm created while crossing over the agents
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void crossoverAgents(
            std::array<std::reference_wrapper<const Agent>, 2> agents, EvoGraph::Graph& graph, AgentManager& manager, std::vector<std::reference_wrapper<const Agent>>& newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng
        ) override;

        /**
         * \brief swap two action values of an agent
         * 
         * \param[in,out] agent the Agent to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] rng Random Number
         * Generator used in the mutation process.
         */
        void swapActionValues(const SpeciesAgent& agent, AgentManager& manager, RNG::RNG& rng);

        /**
         * \brief swap two program position of an agent
         * 
         * \param[in,out] agent the Agent to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] rng Random Number
         * \param[in] params Probability parameters for the mutation.
         * Generator used in the mutation process.
         */
        void swapPrograms(const SpeciesAgent& agent, AgentManager& manager, RNG::RNG& rng, const Learn::LearningParameters& params);

        /**
         * \brief Prepares the mutation of a Edge.
         *
         * This function creates a copy of the program associated to the Edge
         * in preparation of its mutation. The methods also takes care of
         * stochastically mutating the destination of the Edge. The copied
         * program are referenced in the newProgram list, and their behavior
         * must be mutated after this function to complete the mutation process.
         *
         * \param[in,out] agent the Agent to mutate.
         * \param[in,out] graph the Graph within which the team and edge are
         *                stored.
         * \param[in] edge the Edge whose destination will be altered.
         * \param[in] manager the manager to change the agents.
         * \param[in] newSubAgents vector of new agents of sub algorithm created while mutating the agent
         * \param[in] params
         * Probability parameters for the mutation.
         * \param[in] rng Random Number
         * Generator used in the mutation process.
         */
        virtual void mutateOutgoingEdge(
            const Agent& agent, EvoGraph::Graph& graph, const EvoGraph::Edge& edge,
            AgentManager& manager,
            std::vector<std::reference_wrapper<const Agent>>& newSubAgents,
            const Learn::LearningParameters& params, RNG::RNG& rng);

        /**
         * \brief mutate a specific agent of an algorithm within a population
         * 
         * \param[in,out] agent the Agent to mutate.
         * \param[in,out] graph the graph to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] newSubAgents vector of new agents of sub algorithm created while mutating the agent
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void mutateAgent(
            const Agent& agent, EvoGraph::Graph& graph, AgentManager& manager, std::vector<std::reference_wrapper<const Agent>>& newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng
        ) override;

        
        /**
         * \brief Mutate the behavior of a Program and ensure its unicity
         * against the given Archive.
         *
         * \param[in] programAgent program agents of sub algorithm created while mutating the agent
         * \param[in,out] graph the graph to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void mutateProgramAgentAgainstArchive(
            const Agent& programAgent, EvoGraph::Graph& graph, 
            AgentManager& manager, const Learn::LearningParameters& params, 
            RNG::RNG& rng);

        /**
         * \brief Specialization of mutateSubAgents method.
         */
        virtual void mutateSubAgents(
            std::vector<std::reference_wrapper<const Agent>>& agents, EvoGraph::Graph& graph, 
            AgentManager& manager, const Learn::LearningParameters& params, 
            RNG::RNG& rng, uint64_t maxNbThreads) override;

        
        /**
         * Setter for the archive
         * 
         * \param[in] archive Archive used by the program agents.
         */
        void setArchive(const Archive& archive);
    };



}; // namespace Mutator

#endif // SPECIES_MUTATOR_H
