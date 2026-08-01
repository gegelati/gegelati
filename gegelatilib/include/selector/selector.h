

#ifndef SELECTOR_H
#define SELECTOR_H

#include "representation/population.h"
#include "learn/evaluationResult.h"
#include "mutator/rng.h"
#include "learn/learningParameters.h"
#include "selector/selectionContext.h"
#include "selector/selectionMetrics.h"
#include "selector/selectionParameters.h"
#include "evoGraph/graph.h"

namespace Selector {

    /**
     * \brief Abstract class for the selection classes, that will choose which
     * individuals survived or not at each generation
     */
    class Selector
    {
      protected:

        /// @brief number of individuals controlled by the selector
        size_t nbIndividuals;

        /// Parameters for the selection
        std::unique_ptr<SelectionParameters> params;

        /// Pointer to the best individual encountered during training, together with
        /// its EvaluationResult.
        std::pair<std::optional<std::reference_wrapper<const Representation::Individual>>,
                  std::shared_ptr<Learn::EvaluationResult>>
            bestIndividual{std::nullopt, nullptr};

        /**
         * \brief Map associating individual EvoGraph::Vertex to their EvaluationResult.
         *
         * If a given Vertex is evaluated several times, its
         * EvaluationResult may be updated with the newer results.
         *
         * Whenever a Vertex is removed from the Graph, its
         * EvaluationResult should also be removed from this map.
         *
         * This map may be used to avoid reevaluating a individual that was already
         * evaluated more than LearningParameters::maxNbEvaluationPerPolicy
         * times.
         */
        std::map<std::reference_wrapper<const Representation::Individual>,
                 std::shared_ptr<Learn::EvaluationResult>>
            resultsPerIndividual;


        /**
         * \brief population of the used representation. The population can delete or create individuals in the representation population
         */
        std::optional<std::reference_wrapper<Representation::Population>> population;

        /**
         * \brief get the population
         * 
         * Throw if population is not set (in optional type)
         */
        Representation::Population& getPopulation();

        /**
         * \brief get the population
         * 
         * Throw if population is not set (in optional type)
         */
        const Representation::Population& cGetPopulation() const;

      public:


        // Disable copying to avoid accidental copies (use references or pointers instead).
        Selector(const Selector&) = delete;
        Selector& operator=(const Selector&) = delete;

        /**
         * \brief Constructor for Selector.
         *
         * \param[in] parameters parameters used by the Selector.
         */
        Selector(std::unique_ptr<SelectionParameters> parameters = std::make_unique<SelectionParameters>())
            : params{std::make_unique<SelectionParameters>(*parameters)}
        {
        }

        /// @brief setter for the number of individuals
        /// @param nbIndividuals number of individuals set
        void setNbIndividuals(size_t nbIndividuals);

        /// @brief Getter for the number of individuals. 
        size_t getNbIndividuals();

        /**
         * \brief set the population of the selector
         * 
         * \param[in] population set to the selector
         */
        void setPopulation(Representation::Population& population);

        /**
         * \brief return true if the population is set
         */
        bool hasPopulation() const;

        /**
         * \brief Removes from the Graph the individual Vertex.
         *
         * The given multimap is updated by removing entries corresponding to
         * decimated vertices.
         *
         * The resultsPerIndividual attribute is updated to remove results associated
         * to removed vertices.
         *
         * \param[in] graph the Graph on which selection is performed.
         * \param[in,out] results a multimap containing individual Vertex
         * associated to their score during an evaluation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void doSelection(
            EvoGraph::Graph& graph,
            std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                          std::reference_wrapper<const Representation::Individual>>& results,
            RNG::RNG& rng);

        /**
         * Creates and return an instance of SelectionMetrics
         *
         * The purpose of this method is to be override by new selection
         * representations to use specific metrics.
         */
        virtual std::shared_ptr<SelectionMetrics> createSelectionMetrics() const;

        /**
         * \brief This method keeps only the best individual policy in the Graph.
         *
         * If the Vertex referenced in the bestIndividual attribute is no longer
         * a Vertex of the Graph, nothing happens.
         * 
         * \param[in] graph the Graph on which selection is performed.
         */
        virtual void keepBestPolicy(EvoGraph::Graph& graph);

        /**
         * \brief Remove the individual from resultsPerIndividual and BestIndividual if already saved.
         * 
         * \param[in] individual Individual removed from the data.
         */
        virtual void removeFromSavedResults(const Representation::Individual& individual);

        /**
         * \brief Update the bestIndividual and resultsPerIndividual attributes.
         *
         * This method updates the value of the bestIndividual attribute with the
         * EvoGraph::Vertex given as an argument in the following cases:
         * - The given EvaluationResult is greater than the one of the current
         *   bestIndividual.
         * - The current bestIndividual is a nullptr.
         * - The current bestIndividual has been removed from the EvoGraph::Graph
         *   managed by the LearningAgent.
         *
         * It should be noted that the last case alone (i.e. without validating
         * the first one) indicates a great variability of the evaluation
         * process as it means that a vertex currently known as the best individual
         * from previous generations, with an EvaluationResult never beaten,
         * was removed from the graph in a following generation, beaten by individual
         * vertex with lower scores than the current record.
         *
         * \param[in] results Map from the evaluateAllIndividuals method.
         */
        virtual void updateEvaluationRecords(
            const std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                                std::reference_wrapper<const Representation::Individual>>& results);

        /**
         * \brief Update the resultsPerIndividual.
         *
         * \param[in] results Map from the evaluateAllIndividuals method.
         */
        virtual void updateResultsPerIndividual(
            const std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                                std::reference_wrapper<const Representation::Individual>>& results);

        /**
         * \brief Update the bestIndividual attribute.
         *
         * This method updates the value of the bestIndividual attribute with the
         * EvoGraph::Vertex given as an argument in the following cases:
         * - The given EvaluationResult is greater than the one of the current
         *   bestIndividual.
         * - The current bestIndividual is a nullptr.
         * - The current bestIndividual has been removed from the EvoGraph::Graph
         *   managed by the LearningAgent.
         *
         * It should be noted that the last case alone (i.e. without validating
         * the first one) indicates a great variability of the evaluation
         * process as it means that a vertex currently known as the best individual
         * from previous generations, with an EvaluationResult never beaten,
         * was removed from the graph in a following generation, beaten by individual
         * vertex with lower scores than the current record.
         *
         * \param[in] results Map from the evaluateAllIndividuals method.
         */
        virtual void updateBestIndividual(
            const std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                                std::reference_wrapper<const Representation::Individual>>& results);

                                
        /**
         * \brief Getter for a specific result of an individual.
         *
         * \param[in] individual The individual whose number of evaluation is
         * checked.
         * \return true if the individual has been evaluated enough times, false
         * otherwise.
         */
        virtual std::shared_ptr<Learn::EvaluationResult> getResultsOf(
            const Representation::Individual& individual) const;

        /**
         * \brief Getter for the number of evaluation of a specific individual
         *
         * \param[in] individual The individual whose number of evaluation is
         * checked.
         * \return true if the individual has been evaluated enough times, false
         * otherwise.
         */
        virtual size_t getNbEvaluation(
            const Representation::Individual& individual) const;

        /**
         * \brief Get the best individual EvoGraph::Vertex encountered since the last init.
         *
         * The returned pointers may be nullptr if no generation was trained
         * since the last init.
         *
         * \return a reference to the bestIndividual attribute.
         */
        virtual const std::pair<std::optional<std::reference_wrapper<const Representation::Individual>>,
                                std::shared_ptr<Learn::EvaluationResult>>&
        getBestIndividual() const;

        /**
         * \brief This method resets the previous registered scores per individual.
         *
         * Resets resultsPerIndividual so that, in the next training,
         * the current individuals will be considered as if they had never
         * been tested. To use for example when there is a scoring policy
         * change.
         */
        virtual void forgetPreviousResults();

        /**
         * \brief Return the resultsPerIndividual map.
         */
        virtual const std::map<std::reference_wrapper<const Representation::Individual>,
                               std::shared_ptr<Learn::EvaluationResult>>&
        getResultsPerIndividual() const;

        /**
         * \brief Update the SelectionContext structure and return it.
         *
         * The context contains data needed for the creation of the new
         * population. This method create the vectors of clonable vertices,
         * preExistingVertices, preExistingEdges and the number of Vertices to
         * create.
         */
        virtual std::unique_ptr<SelectionContext> updateContext() const;

        /**
         * \brief Method to call at the end of TPGMutator::populateTPG
         *
         * This method does nothing with the default selector.
         * 
         * \param[in] graph the Graph on which selection is performed.
         */
        virtual void updateAfterPopulate(EvoGraph::Graph& graph) {
            /* Empty because sub-class does not need to inherrit from it.*/
        };
    };
}; // namespace Selector

#endif // SELECTION_H