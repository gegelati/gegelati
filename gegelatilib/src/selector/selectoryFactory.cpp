

#include "selector/selectorFactory.h"

std::unique_ptr<Selector::Selector> Selector::selectorFactory(
    Algorithm::AgentManager& manager,
    const Learn::LearningParameters& params)
{
    // Create the corresponding selector
    if (params.selection._selectionMode == "classification") {
        return std::make_unique<ClassificationSelector>(manager, params, 1);
    }
    else if (params.selection._selectionMode == "truncation") {
        return std::make_unique<TruncationSelector>(manager, params);
    }
    else if (params.selection._selectionMode == "tournament") {
        return std::make_unique<TournamentSelector>(manager, params);
    }
    else if (params.selection._selectionMode == "mapElites") {
        if (params.mutation.tpg.ratioTeamsOverActions != 0.0 &&
            params.mutation.tpg.ratioTeamsOverActions != 1.0) {
            throw std::runtime_error(
                "MapElitesSelector currently does not support dual population");
        }
        return std::make_unique<MapElitesSelector>(manager, params);
    }
    else {
        throw std::runtime_error("Selection mode not found");
    }
}