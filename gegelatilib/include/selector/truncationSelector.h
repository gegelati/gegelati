

#ifndef TRUNCATION_SELECTOR_H
#define TRUNCATION_SELECTOR_H

#include "selector/selector.h"

namespace Selector {

    /**
     * \brief Selection class that will do a selection with a truncation. This is the classic selection methods used in the earliest works of TPGs
     */
    class TruncationSelector : public Selector
    {
        public:

            /**
             * \brief Constructor for Selector.
             * 
             * \param[in] graph shared pointer of the graph on which the selection is done.
             * \param[in] params parameters used by the Selector.
             */
            TruncationSelector(std::shared_ptr<TPG::TPGGraph> graph, const SelectionParameters& params)
            : Selector{graph, params} {}

            /**
             * \brief override of doSelection method
             * 
             * Removed the worst agents from the population with a truncation process where the worst proportion set in the parameters is deleted.
             * 
             * \param[in,out] results a multimap containing root TPGVertex
             * associated to their score during an evaluation.
             * \param[in] rng Random Number Generator used in the mutation process.
             */
            virtual void doSelection(std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                const TPG::TPGVertex*>& results, Mutator::RNG& rng) override;
    };
}; // nameSpace Select

#endif // TRUNCATION_SELECTOR_H