
#ifndef ATPG_MUTATOR_H
#define ATPG_MUTATOR_H

#include <vector>
#include <numeric>
#include <queue>
#include <mutex>


#include "algorithm/tpg/tpgMutator.h"
#include "algorithm/atpg/atpgManager.h"

namespace Algorithm::ATPG {

    /**
     * \brief Class representing a ActionTPGMutator
     */
    class ATPGMutator : public TPG::TPGMutator
    {
    protected:

        /// Name of the action program algorithm associated with the TPG agents.
        std::string actionProgramAlgorithmName;

        /// Pre-existing program teams elements used for mutation operations.
        std::vector<std::shared_ptr<const EvoGraph::Team>> preExistingProgramTeams;

    public:

        /**
         * \brief Constructor for ATPGMutator
         * 
         * \param[in] selector Reference to the current selector used by the algorithm.
         * \param[in] archive Archive used by this ActionTPG
         */
        ATPGMutator(Selector::Selector& selector, std::shared_ptr<const Archive> archive): TPGMutator(selector, archive) {};

        
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
         * \brief Set the name of the action program algorithm associated with the TPG agents.
         * 
         * \param[in] name the name of the action program algorithm.
         */
        void setActionProgramAlgorithmName(const std::string& name) { this->actionProgramAlgorithmName = name; }

        /**
         * \brief Get the name of the action program algorithm associated with the TPG agents.
         * 
         * \return the name of the action program algorithm.
         */
        std::string getActionProgramAlgorithmName() const { return this->actionProgramAlgorithmName; }


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
        virtual void initRandomSpecificAgent(std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Learn::LearningParameters& params, RNG::RNG& rng) override;


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
         * \param[in] manager the manager to change the agents.
         * \param[in] newSubAgents vector of new agents of sub algorithm created while mutating the agent
         * \param[in] params Probability parameters for the
         * mutation.
         * \param[in] rng Random Number Generator used in the mutation
         * process.
         */
        virtual void mutateEdgeDestination(std::shared_ptr<EvoGraph::Graph> graph,
                                    std::shared_ptr<const EvoGraph::Edge> edge,
                                    std::shared_ptr<AgentManager> manager,
                                    std::vector<std::shared_ptr<const Agent>>& newSubAgents,
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
         * \param[in] params  Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void mutateOutgoingEdge(
            std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<const EvoGraph::Edge> edge,
            std::shared_ptr<AgentManager> manager,
            std::vector<std::shared_ptr<const Agent>>& newSubAgents,
            const Learn::LearningParameters& params, RNG::RNG& rng) override;


            
        /**
         * \brief Specialization of mutateSubAgents method.
         */
        virtual void mutateSubAgents(
            std::vector<std::shared_ptr<const Agent>>& agents, std::shared_ptr<EvoGraph::Graph> graph, 
            std::shared_ptr<AgentManager> manager, const Learn::LearningParameters& params, 
            RNG::RNG& rng, uint64_t maxNbThreads) override;
    };



}; // namespace Mutator

#endif // TPG_MUTATOR_H
