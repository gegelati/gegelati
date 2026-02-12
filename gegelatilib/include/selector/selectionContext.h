

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

        /// @brief vector sjared pointer of agent that are clonable by the
        /// Mutator to fill the new population.
        std::vector<std::reference_wrapper<const Algorithm::Agent>> agentsClonable;


        /// @brief vector of shared pointer of agent that can be new
        /// destination for the new agents created by the Mutator.
        std::vector<std::reference_wrapper<const Algorithm::Agent>> preExistingAgents;

        /// @brief number of agent to be created by the mutator
        uint64_t nbAgentsToCreate;
    };
}; // namespace Selector

#endif // SELECTION_CONTEXT_H