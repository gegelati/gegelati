#include "evolution/survivingSelection.h"

std::map<std::reference_wrapper<const Evolution::Individual>, bool> Evolution::SurvivingSelection::select(const std::multimap<std::shared_ptr<Learn::EvaluationResult>, std::reference_wrapper<const Individual>>& scores) const
{
    std::map<std::reference_wrapper<const Evolution::Individual>, bool> selection;

    // Standard (mu+lambda) replacement
    size_t mu = 100;
    for(auto it = scores.begin(); it != scores.end(); it++) {
        bool selected = (scores.size() - selection.size()) <= mu;
        selection.insert({it->second, selected});
    }
    return selection;
}