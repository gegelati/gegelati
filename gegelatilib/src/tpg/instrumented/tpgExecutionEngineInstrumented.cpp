#include "tpg/instrumented/tpgExecutionEngineInstrumented.h"
#include "tpg/instrumented/tpgActionInstrumented.h"
#include "tpg/instrumented/tpgEdgeInstrumented.h"
#include "tpg/instrumented/tpgTeamInstrumented.h"

double TPG::TPGExecutionEngineInstrumented::evaluateEdge(const TPGEdge& edge)
{
    dynamic_cast<const TPGEdgeInstrumented&>(edge).incrementNbVisits();
    return TPGExecutionEngine::evaluateEdge(edge);
}

const TPG::TPGEdge& TPG::TPGExecutionEngineInstrumented::evaluateTeam(
    const TPGTeam& team, const std::vector<const TPGVertex*>& excluded)
{
    dynamic_cast<const TPGTeamInstrumented&>(team).incrementNbVisits();

    const TPGEdge& winningEdge =
        TPGExecutionEngine::evaluateTeam(team, excluded);
    dynamic_cast<const TPGEdgeInstrumented&>(winningEdge)
        .incrementNbTraversed();
    return winningEdge;
}

const std::vector<const TPG::TPGVertex*> TPG::TPGExecutionEngineInstrumented::
    executeFromRoot(const TPG::TPGVertex& root)
{
    const std::vector<const TPG::TPGVertex*> result =
        TPGExecutionEngine::executeFromRoot(root);

    // Increment action visit
    dynamic_cast<const TPGActionInstrumented*>(result.back())
        ->incrementNbVisits();

    return result;
}
