

#ifndef TOURNAMENT_SELECTOR_H
#define TOURNAMENT_SELECTOR_H

#include "selector/selector.h"

namespace Selector {

    /**
     * \brief Selection class that will do a selection with a tournament.
     *
     * A small proportion X of the best individuals is kept and saved from the
     * tournament, X is an hyperparameter.
     *
     * The remaining of the population is randomly put in tournament tables of
     * size Y, Y is an hyperparameter. The best individuals of each tournment is
     * selected, the others are deleted.
     *
     * During mutation process, the best individuals saved are not used in the
     * mutation process, only the survivors of the tournament. However this
     * survivors are deleted after the offspring creation.
     */
    class TournamentSelector : public Selector
    {
      protected:

        /**
         * @brief set of Vertex filled during the selection process,
         * containing the vertices that went through the tournament en survived
         * it. The Vertex in the set will be deleted at the end of the
         * TPGMutator::PopulateTPG method.
         */
        std::set<std::reference_wrapper<const Representation::Individual>> individualsToDelete;

      public:
        /**
         * \brief Constructor for Selector.
         *
         * \param[in] parameters parameters used by the Selector.
         */
        TournamentSelector(std::unique_ptr<SelectionParameters> parameters = std::make_unique<SelectionParameters>())
            : Selector{std::move(parameters)}
        {
        }
        /**
         * \brief override of doSelection method
         *
         * A small proportion X of the best individuals is kept and saved from the
         * tournament, X is an hyperparameter.
         *
         * The remaining of the population is randomly put in tournament tables
         * of size Y, Y is an hyperparameter. The best individuals of each tournment
         * is selected, the others are deleted.
         *
         * During mutation process, the best individuals saved are not used in the
         * mutation process, only the survivors of the tournament. However this
         * survivors are deleted after the offspring creation.
         *
         * The tournament selection is not secured when the ratioTeamOverAction
         * is between 0 and 1 It can be used, but it could happen that one
         * "population" take over the other one
         *
         * \param[in] graph the Graph on which selection is performed.
         * \param[in,out] results a multimap containing individual
         * associated to their score during an evaluation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void doSelection(
            EvoGraph::Graph& graph,
            std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                          std::reference_wrapper<const Representation::Individual>>& results,
            RNG::RNG& rng) override;

        /**
         * \brief add a vertex to the verticesToDelete set.
         *
         * \param[in] vertex Vertex added to the vertices to remove
         */
        void addToVerticesToDelete(const Representation::Individual& vertex);

        /**
         * \brief Specialization of updateContext for tournament purposes
         *
         * The method will remove the elite individuals from the clonableVertices
         * vectors, and will remove the not elite individuals from the
         * preExistingVertices vectors
         */
        virtual std::unique_ptr<SelectionContext> updateContext() const override;

        /**
         * \brief Specialization of updateAfterPopulate for tournament purposes
         *
         * This method erase the individuals that have survived the tournaments and
         * have generated new offsprings.
         * 
         * \param[in] graph the Graph on which selection is performed.
         */
        virtual void updateAfterPopulate(EvoGraph::Graph& graph) override;

        /**
         * \brief getter of the verticesToDelete set.
         */
        virtual const std::set<std::reference_wrapper<const Representation::Individual>>& getIndividualsToDelete();
    };
}; // namespace Selector

#endif // TOURNAMENT_SELECTOR_H