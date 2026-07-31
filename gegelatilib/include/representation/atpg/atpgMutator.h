
#ifndef ATPG_MUTATOR_H
#define ATPG_MUTATOR_H

#include <vector>
#include <numeric>
#include <queue>
#include <mutex>


#include "representation/tpg/tpgMutator.h"
#include "representation/atpg/atpgPopulation.h"

namespace Representation::ATPG {

    /**
     * \brief Class representing a ActionTPGMutator
     */
    class ATPGMutator : public TPG::TPGMutator
    {
    protected:

        /// Name of the action program representation associated with the TPG agents.
        uint64_t actionProgramRepresentationID;

        /// Pre-existing program teams elements used for mutation operations.
        std::vector<std::reference_wrapper<const Individual>> preExistingActionProgram;

    public:

        /**
         * \brief Constructor for ATPGMutator
         * 
         * \param[in] selector Reference to the current selector used by the representation.
         * \param[in] representationID id of the representation used.
         * \param[in] archive Archive used by this ActionTPG
         */
        ATPGMutator(Selector::Selector& selector, uint64_t representationID, const TPG::TPGArchive& archive): TPGMutator(selector, representationID, archive) {};

        
        /**
         * \brief Update the context used by the TPGMutator to populate the Graph.
         * 
         * \param[in] graph the Graph.
         * \param[in] population the population to change the agents.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void updateSpecificContext(
            EvoGraph::Graph& graph, Population& population,
            const RepresentationParameters& params,
            RNG::RNG& rng) override;

        /**
         * \brief Set the id of the action program representation associated with the TPG agents.
         * 
         * \param[in] id the id of the action program representation.
         */
        void setActionProgramRepresentationID(uint64_t id) { this->actionProgramRepresentationID = id; }

        /**
         * \brief Get the id of the action program representation associated with the TPG agents.
         * 
         * \return the id of the action program representation.
         */
        uint64_t getActionProgramRepresentationID() const { return this->actionProgramRepresentationID; }


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
         * \param[in] population the population to change the agents.
         * \param[in] newSubAgents vector of new agents of sub representation created while mutating the agent
         * \param[in] params Probability parameters for the
         * mutation.
         * \param[in] rng Random Number Generator used in the mutation
         * process.
         */
        virtual void mutateEdgeDestination(EvoGraph::Graph& graph,
                                    const EvoGraph::Edge& edge,
                                    Population& population,
                                    std::vector<std::reference_wrapper<const Individual>>& newSubAgents,
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
         * \param[in] population the population to change the agents.
         * \param[in] newSubAgents vector of new agents of sub representation created while mutating the agent
         * \param[in] params  Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void mutateOutgoingEdge(
            EvoGraph::Graph& graph, const EvoGraph::Edge& edge,
            Population& population,
            std::vector<std::reference_wrapper<const Individual>>& newSubAgents,
            const RepresentationParameters& params, RNG::RNG& rng) override;


            
        /**
         * \brief Specialization of mutateSubAgents method.
         */
        virtual void mutateSubAgents(
            std::vector<std::reference_wrapper<const Individual>>& agents, EvoGraph::Graph& graph, 
            Population& population, const RepresentationParameters& params, 
            RNG::RNG& rng, uint64_t maxNbThreads) override;
    };



}; // namespace Mutator

#endif // TPG_MUTATOR_H
