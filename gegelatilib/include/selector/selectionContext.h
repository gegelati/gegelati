

#ifndef SELECTION_CONTEXT_H
#define SELECTION_CONTEXT_H

#include "tpg/tpgGraph.h"

namespace Selector {

    /**
     * \brief Structure to interface between the selector and the TPGMutator. 
     * 
     * The SelectionContext structure is created and updated by the selector and is filled with the data needed by the TPGMutator.
     */
    struct SelectionContext
    {
        /// @brief vector of pointers of const TPGTeam that are clonable by the TPGMutator to fill the new population.
        std::vector<const TPG::TPGTeam*> teamsClonable;

        /// @brief vector of pointers of const TPGAction that are clonable by the TPGMutator to fill the new population.
        std::vector<const TPG::TPGAction*> actionsClonable;


        /// @brief vector of pointers of const TPGTeams that can be new destination for the new roots created by the TPGMutator.
        std::vector<const TPG::TPGTeam*> preExistingTeams;

        /// @brief vector of pointers of const TPGAction that can be new destination for the new roots created by the TPGMutator.
        std::vector<const TPG::TPGAction*> preExistingActions;


        /// @brief list of pointers of const TPGEdges that can be used during the mutation process of the TPGMutator
        std::list<const TPG::TPGEdge*> preExistingEdges;


        /// @brief number of TPGTeams to be created by the TPGMutator
        uint64_t nbTeamsToCreate; 

        /// @brief number of TPGActions to be created by the TPGMutator
        uint64_t nbActionsToCreate;
    };
}; // namespace Selector

#endif // SELECTION_CONTEXT_H