

#ifndef TOURNAMENT_SELECTOR_H
#define TOURNAMENT_SELECTOR_H

#include "selector/selector.h"

namespace Selector {

    /**
     * \brief Selection class that will do a selection with a tournament.
     * 
     * A small proportion X of the best agents is kept and saved from the tournament, X is an hyperparameter.
     * 
     * The remaining of the population is randomly put in tournament tables of size Y, Y is an hyperparameter. 
     * The best agents of each tournment is selected, the others are deleted.
     * 
     * During mutation process, the best agents saved are not used in the mutation process, only the survivors of the tournament.
     * However this survivors are deleted after the offspring creation.
     */
    class TournamentSelector : public Selector
    {
        public:

            /**
             * \brief Constructor for Selector.
             * 
             * \param[in] graph shared pointer of the graph on which the selection is done.
             * \param[in] params parameters used by the Selector.
             */
            TournamentSelector(std::shared_ptr<TPG::TPGGraph> graph, const Learn::LearningParameters& params)
            : Selector{graph, params} {}

            /**
             * \brief override of doSelection method
             * 
             * A small proportion X of the best agents is kept and saved from the tournament, X is an hyperparameter.
             * 
             * The remaining of the population is randomly put in tournament tables of size Y, Y is an hyperparameter. 
             * The best agents of each tournment is selected, the others are deleted.
             * 
             * During mutation process, the best agents saved are not used in the mutation process, only the survivors of the tournament.
             * However this survivors are deleted after the offspring creation.
             * 
             * The tournament selection is not secured when the ratioTeamOverAction
             * is between 0 and 1 It can be used, but it could happen that one
             * "population" take over the other one
             * 
             * \param[in,out] results a multimap containing root TPGVertex
             * associated to their score during an evaluation.
             * \param[in] rng Random Number Generator used in the mutation process.
             */
            virtual void doSelection(std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                const TPG::TPGVertex*>& results, Mutator::RNG& rng) override;

            /**
             * \brief Specialization of updateContext for tournament puposes
             * 
             * The method will remove the elite agents from the clonableVertices vectors, and will remove the not elite agents from the preExistingVertices vectors
             */
            virtual const SelectionContext& updateContext() override;
    };
}; // namespace Selector

#endif // TOURNAMENT_SELECTOR_H