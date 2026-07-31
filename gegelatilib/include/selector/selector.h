

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
     * agents survived or not at each generation
     */
    class Selector
    {
      protected:

        /// @brief number of agents controlled by the selector
        size_t nbAgents;

        /// Parameters for the selection
        std::unique_ptr<SelectionParameters> params;

        /// Pointer to the best agent encountered during training, together with
        /// its EvaluationResult.
        std::pair<std::optional<std::reference_wrapper<const Representation::Individual>>,
                  std::shared_ptr<Learn::EvaluationResult>>
            bestAgent{std::nullopt, nullptr};

        /**
         * \brief Map associating agent EvoGraph::Vertex to their EvaluationResult.
         *
         * If a given Vertex is evaluated several times, its
         * EvaluationResult may be updated with the newer results.
         *
         * Whenever a Vertex is removed from the Graph, its
         * EvaluationResult should also be removed from this map.
         *
         * This map may be used to avoid reevaluating a agent that was already
         * evaluated more than LearningParameters::maxNbEvaluationPerPolicy
         * times.
         */
        std::map<std::reference_wrapper<const Representation::Individual>,
                 std::shared_ptr<Learn::EvaluationResult>>
            resultsPerAgent;


        /**
         * \brief population of the used representation. The population can delete or create agents in the representation population
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

        /// @brief setter for the number of agents
        /// @param nbAgents number of agents set
        void setNbAgents(size_t nbAgents);

        /// @brief Getter for the number of agents. 
        size_t getNbAgents();

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
         * \brief Removes from the Graph the agent Vertex.
         *
         * The given multimap is updated by removing entries corresponding to
         * decimated vertices.
         *
         * The resultsPerAgent attribute is updated to remove results associated
         * to removed vertices.
         *
         * \param[in] graph the Graph on which selection is performed.
         * \param[in,out] results a multimap containing agent Vertex
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
         * \brief This method keeps only the best agent policy in the Graph.
         *
         * If the Vertex referenced in the bestAgent attribute is no longer
         * a Vertex of the Graph, nothing happens.
         * 
         * \param[in] graph the Graph on which selection is performed.
         */
        virtual void keepBestPolicy(EvoGraph::Graph& graph);

        /**
         * \brief Remove the agent from resultsPerAgent and BestAgent if already saved.
         * 
         * \param[in] agent Individual removed from the data.
         */
        virtual void removeFromSavedResults(const Representation::Individual& agent);

        /**
         * \brief Update the bestAgent and resultsPerAgent attributes.
         *
         * This method updates the value of the bestAgent attribute with the
         * EvoGraph::Vertex given as an argument in the following cases:
         * - The given EvaluationResult is greater than the one of the current
         *   bestAgent.
         * - The current bestAgent is a nullptr.
         * - The current bestAgent has been removed from the EvoGraph::Graph
         *   managed by the LearningAgent.
         *
         * It should be noted that the last case alone (i.e. without validating
         * the first one) indicates a great variability of the evaluation
         * process as it means that a vertex currently known as the best agent
         * from previous generations, with an EvaluationResult never beaten,
         * was removed from the graph in a following generation, beaten by agent
         * vertex with lower scores than the current record.
         *
         * \param[in] results Map from the evaluateAllAgents method.
         */
        virtual void updateEvaluationRecords(
            const std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                                std::reference_wrapper<const Representation::Individual>>& results);

        /**
         * \brief Update the resultsPerAgent.
         *
         * \param[in] results Map from the evaluateAllAgents method.
         */
        virtual void updateResultsPerAgent(
            const std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                                std::reference_wrapper<const Representation::Individual>>& results);

        /**
         * \brief Update the bestAgent attribute.
         *
         * This method updates the value of the bestAgent attribute with the
         * EvoGraph::Vertex given as an argument in the following cases:
         * - The given EvaluationResult is greater than the one of the current
         *   bestAgent.
         * - The current bestAgent is a nullptr.
         * - The current bestAgent has been removed from the EvoGraph::Graph
         *   managed by the LearningAgent.
         *
         * It should be noted that the last case alone (i.e. without validating
         * the first one) indicates a great variability of the evaluation
         * process as it means that a vertex currently known as the best agent
         * from previous generations, with an EvaluationResult never beaten,
         * was removed from the graph in a following generation, beaten by agent
         * vertex with lower scores than the current record.
         *
         * \param[in] results Map from the evaluateAllAgents method.
         */
        virtual void updateBestAgent(
            const std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                                std::reference_wrapper<const Representation::Individual>>& results);

                                
        /**
         * \brief Getter for a specific result of an agent.
         *
         * \param[in] agent The agent whose number of evaluation is
         * checked.
         * \return true if the agent has been evaluated enough times, false
         * otherwise.
         */
        virtual std::shared_ptr<Learn::EvaluationResult> getResultsOf(
            const Representation::Individual& agent) const;

        /**
         * \brief Getter for the number of evaluation of a specific agent
         *
         * \param[in] agent The agent whose number of evaluation is
         * checked.
         * \return true if the agent has been evaluated enough times, false
         * otherwise.
         */
        virtual size_t getNbEvaluation(
            const Representation::Individual& agent) const;

        /**
         * \brief Get the best agent EvoGraph::Vertex encountered since the last init.
         *
         * The returned pointers may be nullptr if no generation was trained
         * since the last init.
         *
         * \return a reference to the bestAgent attribute.
         */
        virtual const std::pair<std::optional<std::reference_wrapper<const Representation::Individual>>,
                                std::shared_ptr<Learn::EvaluationResult>>&
        getBestAgent() const;

        /**
         * \brief This method resets the previous registered scores per agent.
         *
         * Resets resultsPerAgent so that, in the next training,
         * the current agents will be considered as if they had never
         * been tested. To use for example when there is a scoring policy
         * change.
         */
        virtual void forgetPreviousResults();

        /**
         * \brief Return the resultsPerAgent map.
         */
        virtual const std::map<std::reference_wrapper<const Representation::Individual>,
                               std::shared_ptr<Learn::EvaluationResult>>&
        getResultsPerAgent() const;

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