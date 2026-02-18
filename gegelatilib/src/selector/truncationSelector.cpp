

#include "selector/truncationSelector.h"

void Selector::TruncationSelector::doSelection(
    std::shared_ptr<EvoGraph::Graph> graph,
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  std::reference_wrapper<const Algorithm::Agent>>& results,
    RNG::RNG& rng)
{
    // Some actions may be encountered but not removed while scanning the
    // results map they should be re-inserted to the list before leaving the
    // method.
    // Teams and actions are not removed also if there is 1% of teams or actions
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  std::reference_wrapper<const Algorithm::Agent>>
        preservedAgents;

    // Estimate the number of expected agents to delete
    size_t nbExpectedAgents =
        (size_t)floor(this->params.selection.truncation.ratioDeletedRoots *
                      (double)results.size());

    auto i = 0;
    while (i < nbExpectedAgents && results.size() > 0) {

        auto it = results.begin();
        const Algorithm::Agent& agent = it->second;
        results.erase(it);

        // Removed stored result (if any)
        this->removeFromSavedResults(agent);

        this->manager->deleteAgent(agent, graph);

        // Increment loop counter
        i++;
    }
}