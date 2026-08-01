

#ifndef SELECTION_CONTEXT_H
#define SELECTION_CONTEXT_H

#include "representation/individual.h"
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

        /// @brief vector sjared pointer of individual that are clonable by the
        /// Mutator to fill the new population.
        std::vector<std::reference_wrapper<const Representation::Individual>> individualsClonable;


        /// @brief vector of shared pointer of individual that can be new
        /// destination for the new individuals created by the Mutator.
        std::vector<std::reference_wrapper<const Representation::Individual>> preExistingIndividuals;

        /// @brief number of individual to be created by the mutator
        uint64_t nbIndividualsToCreate;
    };
}; // namespace Selector

#endif // SELECTION_CONTEXT_H