

#include "selector/truncationSelector.h"

void Selector::TruncationSelector::doSelection(
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const TPG::TPGVertex*>& results,
    Mutator::RNG& rng)
{
    // Some actions may be encountered but not removed while scanning the
    // results map they should be re-inserted to the list before leaving the
    // method.
    // Teams and actions are not removed also if there is 1% of teams or actions
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const TPG::TPGVertex*>
        preservedRoots;

    // Estimate the number of expected roots to delete
    size_t nbExpectedRoots =
        (size_t)floor(this->params.selection.truncation.ratioDeletedRoots *
                      (double)results.size());

    auto i = 0;
    while (i < nbExpectedRoots && results.size() > 0) {

        // If the root is an action, do not remove it in discrete environment!
        const TPG::TPGVertex* root = results.begin()->second;
        if (dynamic_cast<const TPG::TPGAction*>(root) != nullptr &&
            !this->params.mutation.tpg.useActionProgram) {
            preservedRoots.insert(*results.begin());
            i--; // no vertex was actually removed
        }
        else {
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