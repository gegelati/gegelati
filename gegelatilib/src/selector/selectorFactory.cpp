

#include "selector/selectorFactory.h"

std::unique_ptr<Selector::Selector> Selector::selectorFactory(
    SelectionParameters params)
{
    // Create the corresponding selector
    if (params._selectionMode == "classification") {
        return std::make_unique<ClassificationSelector>(params, 1);
    }
    else if (params._selectionMode == "truncation") {
        return std::make_unique<TruncationSelector>(params);
    }
    else if (params._selectionMode == "tournament") {
        return std::make_unique<TournamentSelector>( params);
    }
    else if (params._selectionMode == "mapElites") {
        return std::make_unique<MapElitesSelector>(params, 0);
    }
    else {
        throw std::runtime_error("Selection mode not found");
    }
}