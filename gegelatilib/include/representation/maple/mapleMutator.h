
#ifndef MAPLE_MUTATOR_H
#define MAPLE_MUTATOR_H

#include <vector>
#include <numeric>

#include "representation/tpg/tpgMutator.h"
#include "representation/maple/mapleIndividual.h"
#include "representation/maple/maplePopulation.h"

namespace Representation::Maple {

    /**
     * \brief Class representing a MapleMutator
     */
    class MapleMutator : public TPG::TPGMutator
    {

    public:

        /**
         * \brief Constructor for MapleMutator
         * 
         * \param[in] selector Reference to the current selector used by the representation.
         * \param[in] representationID id of the representation used.
         * \param[in] archive Archive used by this representation
         */
        MapleMutator(const Selector::Selector& selector, uint64_t representationID, const TPG::TPGArchive& archive): TPGMutator(selector, representationID, archive){};


        /**
         * \brief Check if the configuration of the mutator is valid according to the given parameters.
         * 
         * This method is called before initializing the population.
         * 
         * \param[in] params the Parameters for the mutation.
         * \param[in] outputs the OutputHandler of the population.
         */
        virtual bool isConfigurationValid(const RepresentationParameters& params, const Output::OutputHandler& outputs) const override;

        /**
         * \brief Initialize TPG Population.
         *
         * \param[in,out] graph the initialized Graph.
         * \param[in] population the population to change the agents.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void initRandomPopulation(EvoGraph::Graph& graph, Population& population, const RepresentationParameters& params, RNG::RNG& rng) override;


        /**
         * \brief Initialize a random Individual.
         *
         * \param[in] agent the agent initialized.
         * \param[in,out] graph the Graph.
         * \param[in] population the population to change the agents.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void initRandomSpecificAgent(const Individual& agent, EvoGraph::Graph& graph, Population& population, const RepresentationParameters& params, RNG::RNG& rng) override;
        
        /**
         * \brief Cross the two teams' program at the specific index of program given
         * 
         * \param[in,out] teams the Team to crossover.
         * \param[in] indexCross the index at which the programs are crossed
         * \param[in,out] graph the graph to mutate.
         * \param[in] population the population to change the agents.
         * \param[in] newSubAgents vector of new agents of sub representation created while crossing over the agents
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void crossoverPrograms(
            std::array<std::reference_wrapper<const EvoGraph::Team>, 2> teams, uint64_t indexCross, EvoGraph::Graph& graph, Population& population, std::vector<std::reference_wrapper<const Individual>>& newSubAgents, const RepresentationParameters& params, RNG::RNG& rng);


        /**
         * \brief Cross the two teams' edges at the specific index of edges given
         * 
         * \param[in,out] teams the Team to crossover.
         * \param[in] indexCross the index at which the edges are crossed
         * \param[in,out] graph the graph to mutate.
         * \param[in] population the population to change the agents.
         * \param[in] newSubAgents vector of new agents of sub representation created while crossing over the agents
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void crossoverEdges(
            std::array<std::reference_wrapper<const EvoGraph::Team>, 2> teams, uint64_t indexCross, EvoGraph::Graph& graph, Population& population, std::vector<std::reference_wrapper<const Individual>>& newSubAgents, const RepresentationParameters& params, RNG::RNG& rng);

        /**
         * \brief Do a crossover over two maple agents, by either crossover the program on edges, or crossover the edges. Calling either crossoverPrograms or crossoverEdges methods
         * 
         * \param[in,out] agents the Individual to crossover.
         * \param[in,out] graph the graph to mutate.
         * \param[in] population the population to change the agents.
         * \param[in] newSubAgents vector of new agents of sub representation created while crossing over the agents
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void crossoverAgents(
            std::array<std::reference_wrapper<const Individual>, 2> agents, EvoGraph::Graph& graph, Population& population, std::vector<std::reference_wrapper<const Individual>>& newSubAgents, const RepresentationParameters& params, RNG::RNG& rng
        ) override;



        /**
         * \brief Add a new outgoing Edge to the team within the Graph.
         *
         * This function adds a new outgoing Edge to the team by cloning
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
         * \param[in] team the team whose outgoingEdges will be altered.
         * for mutations.
         * \param[in] rng Random Number Generator used in the
         * mutation process.
         */
        void addRandomEdge(EvoGraph::Graph& graph, const EvoGraph::Team& team,
                            RNG::RNG& rng) override;


        /**
         * \brief Swap two edges of Team.
         *
         * \param[in,out] graph the Graph within which the team and edge are
         *                stored.
         * \param[in] team the Team whose edges will be altered.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        void swapEdges(EvoGraph::Graph& graph, const EvoGraph::Team& team,
                             RNG::RNG& rng);


        /**
         * \brief Change the destination of a Edge to an randomly chosen
         * action, among the available ones.
         * 
         * 
         * \param[in,out] graph the Graph within which the team and edge are
         *                stored.
         * \param[in] edge the Edge whose destination will be altered.
         * \param[in] actionClasses the actionClasses used by the action responsible for the edge.
         * \param[in] params
         * Probability parameters for the mutation.
         * \param[in] rng Random Number
         * Generator used in the mutation process.
         */
        virtual void mutateEdgeDestination(EvoGraph::Graph& graph,
                                    const EvoGraph::Edge& edge,
                                    const std::set<size_t>& actionClasses,
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
         * \param[in] actionClasses the actionClasses used by the action responsible for the edge.
         * \param[in] population the population to change the agents.
         * \param[in] newSubAgents vector of new agents of sub representation created while mutating the agent
         * \param[in] params
         * Probability parameters for the mutation.
         * \param[in] rng Random Number
         * Generator used in the mutation process.
         */
        void mutateOutgoingEdge(
            EvoGraph::Graph& graph, const EvoGraph::Edge& edge,
            const std::set<size_t>& actionClasses, Population& population,
            std::vector<std::reference_wrapper<const Individual>>& newSubAgents,
            const RepresentationParameters& params, RNG::RNG& rng);

        /**
         * \brief mutate a specific agent of an representation within a population
         * 
         * \param[in,out] agent the Individual to mutate.
         * \param[in,out] graph the graph to mutate.
         * \param[in] population the population to change the agents.
         * \param[in] newSubAgents vector of new agents of sub representation created while mutating the agent
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void mutateAgent(
            const Individual& agent, EvoGraph::Graph& graph, Population& population, std::vector<std::reference_wrapper<const Individual>>& newSubAgents, const RepresentationParameters& params, RNG::RNG& rng
        ) override;
    };



}; // namespace Mutator

#endif // TPG_MUTATOR_H
