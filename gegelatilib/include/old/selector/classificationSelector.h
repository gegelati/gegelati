

#ifndef CLASSIFICATION_SELECTOR_H
#define CLASSIFICATION_SELECTOR_H

#include "selector/selector.h"

namespace Selector {

    /**
     * \brief Specialization of the selection class for classification purposes.
     */
    class ClassificationSelector : public Selector
    {
      protected:
        /// Number of actions in the LearningEnvironment
        uint64_t nbActions;

      public:
        /**
         * \brief Constructor for ClassificationSelector.
         *
         * \param[in] parameters parameters used by the Selector.
         * \param[in] nbActions number of actions in the LearningEnvironment.
         */
        ClassificationSelector(std::unique_ptr<SelectionParameters> parameters = std::make_unique<SelectionParameters>(),
                               uint64_t nbActions = 0)
            : Selector{std::move(parameters)}, nbActions{nbActions}
        {
        }

        /**
         * Specialization of createSelectionMetrics
         *
         * Creates and return an instance of ClassificationSelectionMetrics
         */
        virtual std::shared_ptr<SelectionMetrics> createSelectionMetrics() const
            override;

        /**
         * \brief Specialization of the doSelection method for
         * classification purposes.
         *
         * During the decimation process, roughly half of the roots are kept
         * based on their score for individual class of the
         * ClassificationLearningEnvironment. To do so, for each class of the
         * ClassificationLearningEnvironment, the roots provided the best score
         * are preserved during the decimation process even if their global
         * score over all classes is not among the best.
         *
         * The remaining half of preserved roots is selected using the general
         * score obtained over all classes.
         *
         * This per-class preservation is activated only if there is a
         * sufficient number of root vertices in the Graph after decimation
         * to guarantee that all classes are preserved equally. In other word,
         * the same number of root is marked for preservation for each class,
         * which can only be achieved if the number of roots to preserve during
         * the decimation process is superior or equal to twice the number of
         * actions of the ClassificationLearningEnvironment. If an insufficient
         * number of root is preserved during the decimation process, all roots
         * are preserved based on their general score.
         *
         * The results map is updated by the method to keep only the results of
         * non-decimated roots.
         */
        virtual void doSelection(
            EvoGraph::Graph& graph,
            std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                          std::reference_wrapper<const Representation::Individual>>& results,
            RNG::RNG& rng) override;
    };
}; // namespace Selector

#endif // CLASSIFICATION_SELECTOR_H