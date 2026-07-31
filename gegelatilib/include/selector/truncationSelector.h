

#ifndef TRUNCATION_SELECTOR_H
#define TRUNCATION_SELECTOR_H

#include "selector/selector.h"

namespace Selector {

    /**
     * \brief Selection class that will do a selection with a truncation. This
     * is the classic selection methods used in the earliest works of TPGs
     */
    class TruncationSelector : public Selector
    {
      public:
        /**
         * \brief Constructor for Selector.
         *
         * is done.
         * \param[in] parameters parameters used by the Selector.
         */
        TruncationSelector(std::unique_ptr<SelectionParameters> parameters = std::make_unique<SelectionParameters>())
            : Selector{std::move(parameters)}
        {
        }

        /**
         * \brief override of doSelection method
         *
         * Removed the worst agents from the population with a truncation
         * process where the worst proportion set in the parameters is deleted.
         *
         * \param[in] graph the Graph on which selection is performed.
         * \param[in,out] results a multimap containing agent
         * associated to their score during an evaluation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void doSelection(
            EvoGraph::Graph& graph,
            std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                          std::reference_wrapper<const Representation::Individual>>& results,
            RNG::RNG& rng) override;
    };
}; // namespace Selector

#endif // TRUNCATION_SELECTOR_H