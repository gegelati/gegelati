
#ifndef TPG_MUTATOR_H
#define TPG_MUTATOR_H

#include <vector>
#include <numeric>
#include <queue>
#include <mutex>


#include "algorithm/mutator.h"
#include "algorithm/tpg/tpgAgent.h"
#include "algorithm/tpg/tpgManager.h"

namespace Algorithm::TPG {

    /**
     * \brief Class representing a TPGMutator
     */
    class TPGMutator : public Mutator
    {
    protected:

        /// Name of the program algorithm associated with the TPG agents.
        std::string programAlgorithmName;

        /// Pre-existing teams elements used for mutation operations.
        std::vector<std::shared_ptr<const EvoGraph::Team>> preExistingTeams;

        /// Pre-existing actions elements used for mutation operations.
        std::vector<std::shared_ptr<const EvoGraph::Action>> preExistingActions;

        /// Pre-existing edges used for mutation operations.
        std::vector<std::shared_ptr<const EvoGraph::Edge>> preExistingEdges;

        /// Archive used by this TPG
        std::shared_ptr<const Archive> archive;

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
            std::shared_ptr<EvoGraph::Graph> graph,
            std::vector<std::shared_ptr<const EvoGraph::Vertex>> leafVertices,
            std::vector<std::shared_ptr<const EvoGraph::Vertex>> rootVertices,
            std::vector<std::reference_wrapper<const Agent>> programAgent,
            const Learn::LearningParameters& params, RNG::RNG& rng);

    public:

        /**
         * \brief Constructor for TPGMutator
         * 
         * \param[in] selector Reference to the current selector used by the algorithm.
         * \param[in] archive Archive used by this TPG
         */
        TPGMutator(Selector::Selector& selector, std::shared_ptr<const Archive> archive): Mutator(selector), archive{archive} {};

        /**
         * \brief Update the context used by the TPGMutator to populate the Graph.
         * 
         * \param[in] graph the Graph.
         * \param[in] manager the manager to change the agents.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void updateSpecificContext(
            std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, 
            const Learn::LearningParameters& params,
            RNG::RNG& rng) override;


        /**
         * \brief Set the name of the program algorithm associated with the TPG agents.
         * 
         * \param[in] name the name of the program algorithm.
         */
        void setProgramAlgorithmName(const std::string& name) { this->programAlgorithmName = name; }

        /**
         * \brief Get the name of the program algorithm associated with the TPG agents.
         * 
         * \return the name of the program algorithm.
         */
        std::string getProgramAlgorithmName() const { return this->programAlgorithmName; }

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
         * \brief Initialize TPG Population.
         *
         * \param[in,out] graph the initialized Graph.
         * \param[in] manager the manager to change the agents.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void initRandomPopulation(std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Learn::LearningParameters& params, RNG::RNG& rng) override;


        /**
         * \brief Initialize a random Agent.
         *
         * \param[in] agent the agent initialized.
         * \param[in,out] graph the Graph.
         * \param[in] manager the manager to change the agents.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void initRandomSpecificAgent(const Agent& agent, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Learn::LearningParameters& params, RNG::RNG& rng) override;


        /**
         * \brief Select a random outgoingEdge of the given Vertex and removes
         * it from the Graph.
         *
         * \param[in,out] graph the Graph within which the team is stored.
         * \param[in] vertex the Vertex whose outgoingEdges will be altered.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void removeRandomEdge(std::shared_ptr<EvoGraph::Graph> graph, const EvoGraph::Vertex& vertex,
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
        virtual void addRandomEdge(std::shared_ptr<EvoGraph::Graph> graph, const EvoGraph::Team& team,
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
        virtual void mutateEdgeDestination(std::shared_ptr<EvoGraph::Graph> graph,
                                    std::shared_ptr<const EvoGraph::Edge> edge,
                                    const Learn::LearningParameters& params,
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
         * \param[in] newSubAgents vector of new agents of sub algorithm created while mutating the agent
         * \param[in] params
         * Probability parameters for the mutation.
         * \param[in] rng Random Number
         * Generator used in the mutation process.
         */
        virtual void mutateOutgoingEdge(
            std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<const EvoGraph::Edge> edge,
            std::shared_ptr<AgentManager> manager,
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
            const Agent& agent, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, std::vector<std::reference_wrapper<const Agent>>& newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng
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
            const Agent& programAgent, std::shared_ptr<EvoGraph::Graph> graph, 
            std::shared_ptr<AgentManager> manager, const Learn::LearningParameters& params, 
            RNG::RNG& rng);

        /**
         * \brief Specialization of mutateSubAgents method.
         */
        virtual void mutateSubAgents(
            std::vector<std::reference_wrapper<const Agent>>& agents, std::shared_ptr<EvoGraph::Graph> graph, 
            std::shared_ptr<AgentManager> manager, const Learn::LearningParameters& params, 
            RNG::RNG& rng, uint64_t maxNbThreads) override;

        
        /**
         * Setter for the archive
         * 
         * \param[in] archive Archive used by the program agents.
         */
        void setArchive(std::shared_ptr<Archive> archive);
    };



}; // namespace Mutator

#endif // TPG_MUTATOR_H
