

#ifndef SELECTION_CONTEXT_H
#define SELECTION_CONTEXT_H

#include "algorithm/agent.h"
#include "evoGraph/graph.h"

namespace Selector {

    /**
     * \brief Structure to interface between the selector and the TPGMutator.
     *
     * The SelectionContext structure is created and updated by the selector and
     * is filled with the data needed by the TPGMutator.
     */
    struct SelectionContext
    {
        /*
        /// @brief vector of pointers of const Team that are clonable by the
        /// TPGMutator to fill the new population.
        std::vector<const EvoGraph::Team*> teamsClonable;

        /// @brief vector of pointers of const Action that are clonable by
        /// the TPGMutator to fill the new population.
        std::vector<const EvoGraph::Action*> actionsClonable;

        /// @brief vector of pointers of const Teams that can be new
        /// destination for the new roots created by the TPGMutator.
        std::vector<const EvoGraph::Team*> preExistingTeams;

        /// @brief vector of pointers of const Action that can be new
        /// destination for the new roots created by the TPGMutator.
        std::vector<const EvoGraph::Action*> preExistingActions;

        /// @brief list of pointers of const Edges that can be used during
        /// the mutation process of the TPGMutator
        std::list<const EvoGraph::Edge*> preExistingEdges;

        /// @brief number of Teams to be created by the TPGMutator
        uint64_t nbTeamsToCreate;

        /// @brief number of Actions to be created by the TPGMutator
        uint64_t nbActionsToCreate;*/

        

        /// @brief vector sjared pointer of agent that are clonable by the
        /// Mutator to fill the new population.
        std::vector<std::shared_ptr<const Algorithm::Agent>> agentsClonable;


        /// @brief vector of shared pointer of agent that can be new
        /// destination for the new agents created by the Mutator.
        std::vector<std::shared_ptr<const Algorithm::Agent>> preExistingAgents;

        /// @brief number of agent to be created by the mutator
        uint64_t nbAgentsToCreate;
    };
}; // namespace Selector

#endif // SELECTION_CONTEXT_H