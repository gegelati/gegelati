
#ifndef TPG_MUTATOR_H
#define TPG_MUTATOR_H

#include <vector>
#include <numeric>
#include <queue>
#include <mutex>
#include <array>


#include "representation/mutator.h"
#include "representation/tpg/tpgAgent.h"
#include "representation/tpg/tpgManager.h"

namespace Representation::TPG {

    /**
     * \brief Class representing a TPGMutator
     */
    class TPGMutator : public Mutator
    {
    protected:

        /// ID of the program representation associated with the TPG agents.
        uint64_t programRepresentationID;

        /// Pre-existing teams elements used for mutation operations.
        std::vector<std::reference_wrapper<const EvoGraph::Vertex>> preExistingTeams;

        /// Pre-existing actions elements used for mutation operations.
        std::vector<std::reference_wrapper<const EvoGraph::Vertex>> preExistingActions;

        /// Pre-existing edges used for mutation operations.
        std::vector<std::reference_wrapper<const EvoGraph::Edge>> preExistingEdges;

        /// TPGArchive used by this TPG
        std::reference_wrapper<const TPGArchive> archive;

        /**
         * \brief Method called during initRandomPopulation
         * This methods add additionnal edges between the root vertices and the leaf vertices created.
         * 
         * \param[in,out] graph the initialized Graph.
         * \param[in] leafVertices the leaf vertices to connect.
         * \param[in] rootVertices the root vertices to connect.
         * \param[in] programAgent the program agents available in the graph.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void addAditionnalEdges(
            EvoGraph::Graph& graph,
            std::vector<std::reference_wrapper<const EvoGraph::Vertex>> leafVertices,
            std::vector<std::reference_wrapper<const EvoGraph::Vertex>> rootVertices,
            std::vector<std::reference_wrapper<const Agent>> programAgent,
            const RepresentationParameters& params, RNG::RNG& rng);

    public:

        /**
         * \brief Constructor for TPGMutator
         * 
         * \param[in] selector Reference to the current selector used by the representation.
         * \param[in] representationID id of the representation used.
         * \param[in] archive TPGArchive used by this TPG
         */
        TPGMutator(const Selector::Selector& selector, uint64_t representationID, const TPGArchive& archive): Mutator(selector, representationID), archive{archive} {};

        /**
         * \brief Update the context used by the TPGMutator to populate the Graph.
         * 
         * \param[in] graph the Graph.
         * \param[in] manager the manager to change the agents.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void updateSpecificContext(
            EvoGraph::Graph& graph, AgentManager& manager, 
            const RepresentationParameters& params,
            RNG::RNG& rng) override;


        /**
         * \brief Set the id of the program representation associated with the TPG agents.
         * 
         * \param[in] id the id of the program representation.
         */
        void setProgramRepresentationID(uint64_t id) { this->programRepresentationID = id; }

        /**
         * \brief Get the name of the program representation associated with the TPG agents.
         * 
         * \return the name of the program representation.
         */
        uint64_t getProgramRepresentationID() const { return this->programRepresentationID; }

        /**
         * \brief Check if the configuration of the mutator is valid according to the given parameters.
         * 
         * This method is called before initializing the population.
         * 
         * \param[in] params the Parameters for the mutation.
         * \param[in] outputs the OutputHandler of the manager.
         */
        virtual bool isConfigurationValid(const RepresentationParameters& params, const Output::OutputHandler& outputs) const override;

        /**
         * \brief Initialize TPG Population.
         *
         * \param[in,out] graph the initialized Graph.
         * \param[in] manager the manager to change the agents.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void initRandomPopulation(EvoGraph::Graph& graph, AgentManager& manager, const RepresentationParameters& params, RNG::RNG& rng) override;


        /**
         * \brief Initialize a random Agent.
         *
         * \param[in] agent the agent initialized.
         * \param[in,out] graph the Graph.
         * \param[in] manager the manager to change the agents.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void initRandomSpecificAgent(const Agent& agent, EvoGraph::Graph& graph, AgentManager& manager, const RepresentationParameters& params, RNG::RNG& rng) override;


        /**
         * \brief Select a random outgoingEdge of the given Vertex and removes
         * it from the Graph.
         *
         * \param[in,out] graph the Graph within which the team is stored.
         * \param[in] vertex the Vertex whose outgoingEdges will be altered.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void removeRandomEdge(EvoGraph::Graph& graph, const EvoGraph::Vertex& vertex,
                                RNG::RNG& rng);

        /**
         * \brief Add a new outgoing Edge to the Team within the Graph.
         *
         * This function adds a new outgoing Edge to the Team by cloning
         * a preExisting Edge of the Graph. Since the graph may contain
         * Edge from previous mutations, the function receives a list of
         * preExisting Edge from which the Edge to copy should be chosen
         * randomly. Any Edge already connected to the Team is also
         * excluded from the candidates. If there is no valid Edge candidate
         * this function will throw an exception (check code for more details).
         * The new Edge will have the same destination Vertex and Program
         * as the cloned one, but its source will be the give Team.
         *
         * \param[in,out] graph the Graph within which the team is stored.
         * \param[in] team the Team whose outgoingEdges will be altered.
         * for mutations.
         * \param[in] rng Random Number Generator used in the
         * mutation process.
         */
        virtual void addRandomEdge(EvoGraph::Graph& graph, const EvoGraph::Team& team,
                            RNG::RNG& rng);

        /**
         * \brief Change the destination of a Edge to an randomly chosen
         * target.
         *
         * This function selects a random Vertex among given pre-existing
         * vector of Team and Action.
         * The function randomly choses between a Action and a Team, with
         * the probabilities within the given MutationParameters. No
         * verification is made on the content of pre-existing Vertex list.
         * If one of this list contains the team itself, a self-loop may be
         * created. A Vertex not belonging to the graph in these lists will
         * cause an exception within the Graph class though. If the current
         * destination of the edge is among the candidates, the new destination
         * may be the same as the old.
         *
         * \param[in,out] graph the Graph within which the team and edge are
         *                stored.
         * \param[in] edge the Edge whose destination will be altered.
         * \param[in] params Probability parameters for the
         * mutation.
         * \param[in] rng Random Number Generator used in the mutation
         * process.
         */
        virtual void mutateEdgeDestination(EvoGraph::Graph& graph,
                                    const EvoGraph::Edge& edge,
                                    const RepresentationParameters& params,
                                    RNG::RNG& rng);

        /**
         * \brief Prepares the mutation of a Edge.
         *
         * This function creates a copy of the program associated to the Edge
         * in preparation of its mutation. The methods also takes care of
         * stochastically mutating the destination of the Edge. The copied
         * program are referenced in the newProgram list, and their behavior
         * must be mutated after this function to complete the mutation process.
         *
         * \param[in,out] graph the Graph within which the team and edge are
         *                stored.
         * \param[in] edge the Edge whose destination will be altered.
         * \param[in] manager the manager to change the agents.
         * \param[in] newSubAgents vector of new agents of sub representation created while mutating the agent
         * \param[in] params
         * Probability parameters for the mutation.
         * \param[in] rng Random Number
         * Generator used in the mutation process.
         */
        virtual void mutateOutgoingEdge(
            EvoGraph::Graph& graph, const EvoGraph::Edge& edge,
            AgentManager& manager,
            std::vector<std::reference_wrapper<const Agent>>& newSubAgents,
            const RepresentationParameters& params, RNG::RNG& rng);

        /**
         * \brief mutate a specific agent of an representation within a population
         * 
         * \param[in,out] agent the Agent to mutate.
         * \param[in,out] graph the graph to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] newSubAgents vector of new agents of sub representation created while mutating the agent
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void mutateAgent(
            const Agent& agent, EvoGraph::Graph& graph, AgentManager& manager, std::vector<std::reference_wrapper<const Agent>>& newSubAgents, const RepresentationParameters& params, RNG::RNG& rng
        ) override;

        
        /**
         * \brief Mutate the behavior of a Program and ensure its unicity
         * against the given TPGArchive.
         *
         * \param[in] programAgent program agents of sub representation created while mutating the agent
         * \param[in,out] graph the graph to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void mutateProgramAgentAgainstArchive(
            const Agent& programAgent, EvoGraph::Graph& graph, 
            AgentManager& manager, const RepresentationParameters& params, 
            RNG::RNG& rng);

        /**
         * \brief Specialization of mutateSubAgents method.
         */
        virtual void mutateSubAgents(
            std::vector<std::reference_wrapper<const Agent>>& agents, EvoGraph::Graph& graph, 
            AgentManager& manager, const RepresentationParameters& params, 
            RNG::RNG& rng, uint64_t maxNbThreads) override;

        
        /**
         * Setter for the archive
         * 
         * \param[in] archive TPGArchive used by the program agents.
         */
        void setArchive(const TPGArchive& archive);
    };



}; // namespace Mutator

#endif // TPG_MUTATOR_H
