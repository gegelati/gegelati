

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
     * \param[in] graph shared pointer of the Graph on which the selection is
     * done.
     * \param[in] manager shared pointer of the manager used by the algorithm.
     * \param[in] params LearningParameters used
     */
    std::shared_ptr<Selector> selectorFactory(
        const std::shared_ptr<EvoGraph::Graph> graph,
        const std::shared_ptr<Algorithm::AgentManager> manager,
        const Learn::LearningParameters& params);

}; // namespace Selector

#endif // SELECTOR_FACTORY_H