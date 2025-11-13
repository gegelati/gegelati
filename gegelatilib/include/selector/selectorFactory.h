

#ifndef SELECTOR_FACTORY_H
#define SELECTOR_FACTORY_H

#include "learn/classificationLearningEnvironment.h"
#include "learn/learningEnvironment.h"
#include "tpg/tpgGraph.h"

#include "selector/classificationSelector.h"
#include "selector/mapElites/mapElitesSelector.h"
#include "selector/selector.h"
#include "selector/tournamentSelector.h"
#include "selector/truncationSelector.h"

namespace Selector {

    /**
     * \brief Factory method to create the appropriate Selector
     *
     * \param[in] graph shared pointer of the TPGGraph on which the selection is
     * done. \param[in] le LearningEnvironment used by the LearningAgent.
     * \param[in] params LearningParameters used
     */
    std::shared_ptr<Selector> selectorFactory(
        const std::shared_ptr<TPG::TPGGraph> graph,
        const Learn::LearningEnvironment& le,
        const Learn::LearningParameters& params);

}; // namespace Selector

#endif // SELECTOR_FACTORY_H