

#ifndef TOURNAMENT_SELECTOR_H
#define TOURNAMENT_SELECTOR_H

#include "selector/selector.h"

namespace Selector {

    /**
     * \brief Selection class that will do a selection with a tournament.
     *
     * A small proportion X of the best agents is kept and saved from the
     * tournament, X is an hyperparameter.
     *
     * The remaining of the population is randomly put in tournament tables of
     * size Y, Y is an hyperparameter. The best agents of each tournment is
     * selected, the others are deleted.
     *
     * During mutation process, the best agents saved are not used in the
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
        std::set<const EvoGraph::Vertex*> verticesToDelete;

        /**
         * @brief set of Vertex filled during the selection process,
         * containing the vertices that went through the tournament en survived
         * it. The Vertex in the set will be deleted at the end of the
         * TPGMutator::PopulateTPG method.
         */
        std::set<std::reference_wrapper<const Algorithm::Agent>> agentsToDelete;

      public:
        /**
         * \brief Constructor for Selector.
         *
         * \param[in] manager Manager used by the algorithm
         * \param[in] params parameters used by the Selector.
         */
        TournamentSelector(std::shared_ptr<Algorithm::AgentManager> manager, 
                           const Learn::LearningParameters& params)
            : Selector{manager, params}
        {
        }
        /**
         * \brief override of doSelection method
         *
         * A small proportion X of the best agents is kept and saved from the
         * tournament, X is an hyperparameter.
         *
         * The remaining of the population is randomly put in tournament tables
         * of size Y, Y is an hyperparameter. The best agents of each tournment
         * is selected, the others are deleted.
         *
         * During mutation process, the best agents saved are not used in the
         * mutation process, only the survivors of the tournament. However this
         * survivors are deleted after the offspring creation.
         *
         * The tournament selection is not secured when the ratioTeamOverAction
         * is between 0 and 1 It can be used, but it could happen that one
         * "population" take over the other one
         *
         * \param[in] graph the Graph on which selection is performed.
         * \param[in,out] results a multimap containing agent
         * associated to their score during an evaluation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void doSelection(
            std::shared_ptr<EvoGraph::Graph> graph,
            std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                          std::weak_ptr<const Algorithm::Agent>>& results,
            RNG::RNG& rng) override;

        /**
         * \brief add a vertex to the verticesToDelete set.
         *
         * \param[in] vertex Vertex added to the vertices to remove
         */
        void addToVerticesToDelete(const Algorithm::Agent& vertex);

        /**
         * \brief Specialization of updateContext for tournament purposes
         *
         * The method will remove the elite agents from the clonableVertices
         * vectors, and will remove the not elite agents from the
         * preExistingVertices vectors
         */
        virtual std::unique_ptr<SelectionContext> updateContext() const override;

        /**
         * \brief Specialization of updateAfterPopulate for tournament purposes
         *
         * This method erase the agents that have survived the tournaments and
         * have generated new offsprings.
         * 
         * \param[in] graph the Graph on which selection is performed.
         */
        virtual void updateAfterPopulate(std::shared_ptr<EvoGraph::Graph> graph) override;

        /**
         * \brief getter of the verticesToDelete set.
         */
        virtual const std::set<std::reference_wrapper<const Algorithm::Agent>>& getAgentsToDelete();
    };
}; // namespace Selector

#endif // TOURNAMENT_SELECTOR_H