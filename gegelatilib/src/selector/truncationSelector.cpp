

#include "selector/truncationSelector.h"

void Selector::TruncationSelector::doSelection(std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                const TPG::TPGVertex*>& results, Mutator::RNG& rng)
{
    // Some actions may be encountered but not removed while scanning the
    // results map they should be re-inserted to the list before leaving the
    // method.
    // Teams and actions are not removed also if there is 1% of teams or actions
    std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>
        preservedRoots;

    // Estimate the number of expected roots to delete
    size_t nbExpectedRoots = (size_t)floor(this->params.selection.truncation.ratioDeletedRoots *
                                           (double)this->params.mutation.tpg.nbRoots);

    // Get the maximum number of teams and actions deletable
    size_t nbTeamsDeleted = 0;
    size_t nbActionsDeleted = 0;
    size_t maxNbTeamsToDelete =
        (size_t)((double)nbExpectedRoots *
                 this->params.mutation.tpg.ratioTeamsOverActions);
    size_t maxNbActionsoDelete = nbExpectedRoots - maxNbTeamsToDelete;

    auto i = 0;
    while (i < nbExpectedRoots && results.size() > 0) {

        // If the root is an action, do not remove it in discrete environment!
        const TPG::TPGVertex* root = results.begin()->second;
        if (dynamic_cast<const TPG::TPGAction*>(root) != nullptr &&
            !this->params.mutation.tpg.useActionProgram) {
            preservedRoots.insert(*results.begin());
            i--; // no vertex was actually removed

            // This conditions avoid deleting all the teams or all the actions
            // This is usefull is the ratioTeamsOverActions is between 0 and 1.
        }
        else if (dynamic_cast<const TPG::TPGTeam*>(results.begin()->second) !=
                     nullptr &&
                 nbTeamsDeleted >= maxNbTeamsToDelete) {

            preservedRoots.insert(*results.begin());
            i--; // no vertex was actually removed
        }
        else if (dynamic_cast<const TPG::TPGAction*>(results.begin()->second) !=
                     nullptr &&
                 nbActionsDeleted >= maxNbActionsoDelete) {

            preservedRoots.insert(*results.begin());
            i--; // no vertex was actually removed
        }
        else {
            // Incremente the number of actions and teams deleted.
            if (dynamic_cast<const TPG::TPGTeam*>(results.begin()->second) !=
                nullptr) {
                nbTeamsDeleted++;
            }
            else {
                nbActionsDeleted++;
            }

            graph->removeVertex(*results.begin()->second);
            // Removed stored result (if any)
            this->resultsPerRoot.erase(results.begin()->second);
        }
        results.erase(results.begin());

        // Increment loop counter
        i++;
    }

    // Restore root actions
    results.insert(preservedRoots.begin(), preservedRoots.end());
}