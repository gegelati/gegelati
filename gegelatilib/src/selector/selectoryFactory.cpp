

#include "selector/selectorFactory.h"

std::shared_ptr<Selector::Selector> Selector::selectorFactory(
    const std::shared_ptr<TPG::TPGGraph> graph,
    const Learn::LearningParameters& params)
{

    // Create the corresponding selector
    if (params.selection._selectionMode == "classification") {
        return std::make_shared<ClassificationSelector>(graph, params, 1);
    }
    else if (params.selection._selectionMode == "truncation") {
        return std::make_shared<TruncationSelector>(graph, params);
    }
    else if (params.selection._selectionMode == "tournament") {
        return std::make_shared<TournamentSelector>(graph, params);
    }
    else if (params.selection._selectionMode == "mapElites") {
        if (params.mutation.tpg.ratioTeamsOverActions != 0.0 &&
            params.mutation.tpg.ratioTeamsOverActions != 1.0) {
            throw std::runtime_error(
                "MapElitesSelector currently does not support dual population");
        }
        return std::make_shared<MapElitesSelector>(graph, params);
    }
    else {
        throw std::runtime_error("Selection mode not found");
    }
}