/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2022) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2022)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

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
        .incrementNbTraversal();
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

    this->traceHistory.push_back(result);

    return result;
}

const std::vector<std::vector<const TPG::TPGVertex*>>& TPG::
    TPGExecutionEngineInstrumented::getTraceHistory()
{
    return this->traceHistory;
}

void TPG::TPGExecutionEngineInstrumented::clearTraceHistory()
{
    this->traceHistory.clear();
}
