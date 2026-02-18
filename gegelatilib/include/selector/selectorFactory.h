

#ifndef SELECTOR_FACTORY_H
#define SELECTOR_FACTORY_H

#include "algorithm/agentManager.h"
#include "learn/classificationLearningEnvironment.h"
#include "learn/learningEnvironment.h"
#include "evoGraph/graph.h"

#include "selector/classificationSelector.h"
#include "selector/mapElites/mapElitesSelector.h"
#include "selector/selector.h"
#include "selector/tournamentSelector.h"
#include "selector/truncationSelector.h"

namespace Selector {

    /**
     * \brief Factory method to create the appropriate Selector
     *
     * \param[in] manager shared pointer of the manager used by the algorithm.
     * \param[in] params LearningParameters used
     */
    std::unique_ptr<Selector> selectorFactory(
        Algorithm::AgentManager& manager,
        const Learn::LearningParameters& params);

}; // namespace Selector

#endif // SELECTOR_FACTORY_H