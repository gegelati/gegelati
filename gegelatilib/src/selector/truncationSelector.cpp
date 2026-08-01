

#include "selector/truncationSelector.h"

void Selector::TruncationSelector::doSelection(
    EvoGraph::Graph& graph,
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  std::reference_wrapper<const Representation::Individual>>& results,
    RNG::RNG& rng)
{
    // Some actions may be encountered but not removed while scanning the
    // results map they should be re-inserted to the list before leaving the
    // method.
    // Teams and actions are not removed also if there is 1% of teams or actions
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  std::reference_wrapper<const Representation::Individual>>
        preservedIndividuals;

    // Estimate the number of expected individuals to delete
    size_t nbExpectedIndividuals =
        (size_t)floor(this->params->truncation.ratioDeletedRoots *
                      (double)results.size());

    auto i = 0;
    while (i < nbExpectedIndividuals && results.size() > 0) {

        auto it = results.begin();
        const Representation::Individual& individual = it->second;
        results.erase(it);

        // Removed stored result (if any)
        this->removeFromSavedResults(individual);

        this->getPopulation().deleteIndividual(individual, graph);

        // Increment loop counter
        i++;
    }
}