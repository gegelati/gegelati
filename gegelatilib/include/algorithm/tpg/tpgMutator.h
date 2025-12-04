
#ifndef TPG_MUTATOR_H
#define TPG_MUTATOR_H

#include <vector>
#include <numeric>

#include "algorithm/mutator.h"
#include "algorithm/tpg/tpgAgent.h"
#include "mutator/programMutator.h"

namespace Algorithm::TPG {

    /**
     * \brief Abstract class representing a Mutator used by an Algorithm.
     * 
     * Available algorithms are TPG, MAPLE, and LGP
     */
    class TPGMutator : public Mutator
    {
    protected:

        /// Attribute that specify if the mutator implements crossover, depending on its algorithm.
        bool isUsingCrossover = false;

    public:

        TPGMutator(): Mutator() {};

        /**
         * \brief Initialize TPG Population.
         *
         * \param[in,out] graph the initialized Graph.
         * \param[in] manager the manager to change the agents.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         * \param[in] nbOutputs number of outputs that will be usable for
         * interacting with this LearningEnviromnent.
         */
        virtual void initRandomPopulation(std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Learn::LearningParameters& params, RNG::RNG& rng, size_t nbOutputs) override;


        /**
         * \brief Initialize a random Agent.
         *
         * \param[in,out] graph the Graph.
         * \param[in] manager the manager to change the agents.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         * \param[in] nbOutputs number of outputs that will be usable for
         * interacting with this LearningEnviromnent.
         */
        virtual std::shared_ptr<const Agent> initRandomAgent(std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Learn::LearningParameters& params, RNG::RNG& rng, size_t nbOutputs) override;

        /**
         * \brief mutate a specific agent of an algorithm within a population
         * 
         * \param[in,out] agents the Agent to crossover.
         * \param[in,out] graph the graph to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] context context from the selection algorithm.
         * \param[in] newSubAgents vector of new agents of sub algorithm created while crossing over the agents
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void crossoverAgents(
            std::vector<std::shared_ptr<const Agent>> agents, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Selector::SelectionContext& context, std::vector<std::shared_ptr<const Agent>> newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng
        ) override;


    /**
     * \brief Select a random outgoingEdge of the given Team and removes
     * it from the Graph.
     *
     * \param[in,out] graph the Graph within which the team is stored.
     * \param[in] team the Team whose outgoingEdges will be altered.
     * \param[in] rng Random Number Generator used in the mutation process.
     */
    void removeRandomEdge(std::shared_ptr<EvoGraph::Graph>, const EvoGraph::Team& team,
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
     * \param[in] context SelectorContext containing necessary information
     * for mutations.
     * \param[in] rng Random Number Generator used in the
     * mutation process.
     */
    void addRandomEdge(std::shared_ptr<EvoGraph::Graph>, const EvoGraph::Team& team,
                        const Selector::SelectionContext& context,
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
     * \param[in] context SelectorContext containing necessary information
     * for mutations.
     * \param[in] params Probability parameters for the
     * mutation.
     * \param[in] rng Random Number Generator used in the mutation
     * process.
     */
    void mutateEdgeDestination(std::shared_ptr<EvoGraph::Graph> graph,
                                const EvoGraph::Edge* edge,
                                const Selector::SelectionContext& context,
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
     * \param[in] context SelectorContext containing necessary information
     * for mutations.
     * \param[in] newSubAgents vector of new agents of sub algorithm created while mutating the agent
     * \param[in] params
     * Probability parameters for the mutation.
     * \param[in] rng Random Number
     * Generator used in the mutation process.
     */
    void mutateOutgoingEdge(
        std::shared_ptr<EvoGraph::Graph> graph, const EvoGraph::Edge* edge,
        const Selector::SelectionContext& context,
        std::vector<std::shared_ptr<const Agent>> newSubAgents,
        const Learn::LearningParameters& params, RNG::RNG& rng);

        /**
         * \brief mutate a specific agent of an algorithm within a population
         * 
         * \param[in,out] agent the Agent to mutate.
         * \param[in,out] graph the graph to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] context context from the selection algorithm.
         * \param[in] newSubAgents vector of new agents of sub algorithm created while mutating the agent
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void mutateAgent(
            std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Selector::SelectionContext& context, std::vector<std::shared_ptr<const Agent>> newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng
        ) override;
    };


}; // namespace Mutator

#endif // TPG_MUTATOR_H
