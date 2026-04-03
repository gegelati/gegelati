

#include "selector/selectorFactory.h"

std::unique_ptr<Selector::Selector> Selector::selectorFactory(
    std::unique_ptr<SelectionParameters> params)
{
    // Create the corresponding selector
    if (params->_selectionMode == "classification") {
        return std::make_unique<ClassificationSelector>(std::move(params), 1);
    }
    else if (params->_selectionMode == "truncation") {
        return std::make_unique<TruncationSelector>(std::move(params));
    }
    else if (params->_selectionMode == "tournament") {
        return std::make_unique<TournamentSelector>( std::move(params));
    }
    else if (params->_selectionMode == "mapElites") {
        return std::make_unique<MapElitesSelector>(std::move(params), 0);
    }
    else {
        throw std::runtime_error("Selection mode not found");
    }
}