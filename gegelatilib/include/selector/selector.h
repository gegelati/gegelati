

#ifndef SELECTOR_H
#define SELECTOR_H

#include "learn/evaluationResult.h"
#include "mutator/rng.h"
#include "selector/selectionContext.h"
#include "selector/selectionMetrics.h"
#include "selector/selectionParameters.h"
#include "tpg/tpgGraph.h"

namespace Selector {

    /**
     * \brief Abstract class for the selection classes, that will choose which
     * agents survived or not at each generation
     */
    class Selector
    {
      protected:
        /// TPGGraph on which the TPGVertex can be selected or deleted.
        std::shared_ptr<TPG::TPGGraph> graph;

        /// Parameters for the selection
        const Learn::LearningParameters& params;

        /// Pointer to the best root encountered during training, together with
        /// its EvaluationResult.
        std::pair<const TPG::TPGVertex*,
                  std::shared_ptr<Learn::EvaluationResult>>
            bestRoot{nullptr, nullptr};

        /**
         * \brief Map associating root TPG::TPGVertex to their EvaluationResult.
         *
         * If a given TPGVertex is evaluated several times, its
         * EvaluationResult may be updated with the newer results.
         *
         * Whenever a TPGVertex is removed from the TPGGraph, its
         * EvaluationResult should also be removed from this map.
         *
         * This map may be used to avoid reevaluating a root that was already
         * evaluated more than LearningParameters::maxNbEvaluationPerPolicy
         * times.
         */
        std::map<const TPG::TPGVertex*,
                 std::shared_ptr<Learn::EvaluationResult>>
            resultsPerRoot;

        /**
         * \brief context used by the TPGMutator to populate the TPGGraph.
         *
         * The context contains various data needed for the creation of the new
         * population and is update in the updateContext methods.
         *
         * This method can be override by the different selectors to specify the
         * data to each case.
         */
        SelectionContext context;

      public:
        /**
         * \brief Constructor for Selector.
         *
         * \param[in] graph shared pointer of the graph on which the selection
         * is done.
         * \param[in] params parameters used by the Selector.
         */
        Selector(std::shared_ptr<TPG::TPGGraph> graph,
                 const Learn::LearningParameters& params)
            : graph{graph}, params{params}
        {
        }

        /**
         * \brief This method execute the doSelection method.
         *
         * However if a double population of team and action roots is used (for
         * MATPG case basically), it launch the doSelection two times, once with
         * the team results, and once with the action results
         *
         * \param[in,out] results a multimap containing root TPGVertex
         * associated to their score during an evaluation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void launchSelection(
            std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                          const TPG::TPGVertex*>& results,
            RNG::RNG& rng);

        /**
         * \brief Removes from the TPGGraph the root TPGVertex.
         *
         * The given multimap is updated by removing entries corresponding to
         * decimated vertices.
         *
         * The resultsPerRoot attribute is updated to remove results associated
         * to removed vertices.
         *
         * \param[in,out] results a multimap containing root TPGVertex
         * associated to their score during an evaluation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void doSelection(
            std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                          const TPG::TPGVertex*>& results,
            RNG::RNG& rng);

        /**
         * Creates and return an instance of SelectionMetrics
         *
         * The purpose of this method is to be override by new selection
         * algorithms to use specific metrics.
         */
        virtual std::shared_ptr<SelectionMetrics> createSelectionMetrics() const;

        /**
         * \brief This method keeps only the bes tRoot policy in the TPGGraph.
         *
         * If the TPGVertex referenced in the bestRoot attribute is no longer
         * a TPGVertex of the TPGGraph, nothing happens.
         */
        virtual void keepBestPolicy();

        /**
         * \brief Update the bestRoot and resultsPerRoot attributes.
         *
         * This method updates the value of the bestRoot attribute with the
         * TPG::Vertex given as an argument in the following cases:
         * - The given EvaluationResult is greater than the one of the current
         *   bestRoot.
         * - The current bestRoot is a nullptr.
         * - The current bestRoot has been removed from the TPG::TPGGraph
         *   managed by the LearningAgent.
         *
         * It should be noted that the last case alone (i.e. without validating
         * the first one) indicates a great variability of the evaluation
         * process as it means that a vertex currently known as the best root
         * from previous generations, with an EvaluationResult never beaten,
         * was removed from the graph in a following generation, beaten by root
         * vertex with lower scores than the current record.
         *
         * \param[in] results Map from the evaluateAllRoots method.
         */
        virtual void updateEvaluationRecords(
            const std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                                const TPG::TPGVertex*>& results);

        /**
         * \brief Update the resultsPerRoot.
         *
         * \param[in] results Map from the evaluateAllRoots method.
         */
        virtual void updateResultsPerRoot(
            const std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                                const TPG::TPGVertex*>& results);

        /**
         * \brief Update the bestRoot attribute.
         *
         * This method updates the value of the bestRoot attribute with the
         * TPG::Vertex given as an argument in the following cases:
         * - The given EvaluationResult is greater than the one of the current
         *   bestRoot.
         * - The current bestRoot is a nullptr.
         * - The current bestRoot has been removed from the TPG::TPGGraph
         *   managed by the LearningAgent.
         *
         * It should be noted that the last case alone (i.e. without validating
         * the first one) indicates a great variability of the evaluation
         * process as it means that a vertex currently known as the best root
         * from previous generations, with an EvaluationResult never beaten,
         * was removed from the graph in a following generation, beaten by root
         * vertex with lower scores than the current record.
         *
         * \param[in] results Map from the evaluateAllRoots method.
         */
        virtual void updateBestRoot(
            const std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                                const TPG::TPGVertex*>& results);

        /**
         * \brief Get the best root TPG::Vertex encountered since the last init.
         *
         * The returned pointers may be nullptr if no generation was trained
         * since the last init.
         *
         * \return a reference to the bestRoot attribute.
         */
        virtual const std::pair<const TPG::TPGVertex*,
                                std::shared_ptr<Learn::EvaluationResult>>&
        getBestRoot() const;

        /**
         * \brief Getter for the TPGGraph built by the LearningAgent.
         *
         * \return Get a shared_pointer to the TPGGraph.
         */
        virtual std::shared_ptr<TPG::TPGGraph> getGraph();

        /**
         * \brief This method resets the previous registered scores per root.
         *
         * Resets resultsPerRoot so that, in the next training,
         * the current roots will be considered as if they had never
         * been tested. To use for example when there is a scoring policy
         * change.
         */
        virtual void forgetPreviousResults();

        /**
         * \brief Return the resultsPerRoot map.
         */
        virtual const std::map<const TPG::TPGVertex*,
                               std::shared_ptr<Learn::EvaluationResult>>&
        getResultsPerRoot() const;

        /**
         * \brief Update the SelectionContext structure and return it.
         *
         * The context contains data needed for the creation of the new
         * population. This method create the vectors of clonable vertices,
         * preExistingVertices, preExistingEdges and the number of Vertices to
         * create.
         */
        virtual const SelectionContext& updateContext();

        /**
         * \brief Method to call at the end of TPGMutator::populateTPG
         *
         * This method does nothing with the default selector.
         */
        virtual void updateAfterPopulate() {
            /* Empty because sub-class does not need to inherrit from it.*/
        };
    };
}; // namespace Selector

#endif // SELECTION_H