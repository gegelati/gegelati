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

#include "tpg/instrumented/tpgInstrumentedFactory.h"
#include "tpg/instrumented/tpgActionInstrumented.h"
#include "tpg/instrumented/tpgEdgeInstrumented.h"
#include "tpg/instrumented/tpgExecutionEngineInstrumented.h"
#include "tpg/instrumented/tpgTeamInstrumented.h"

std::shared_ptr<TPG::TPGGraph> TPG::TPGInstrumentedFactory::createTPGGraph(
    const Environment& env) const
{
    return std::make_shared<TPG::TPGGraph>(
        env, std::make_unique<TPGInstrumentedFactory>());
}

TPG::TPGTeam* TPG::TPGInstrumentedFactory::createTPGTeam() const
{
    return new TPGTeamInstrumented();
}

TPG::TPGAction* TPG::TPGInstrumentedFactory::createTPGAction(
    const uint64_t actID, const uint64_t actClass) const
{
    return new TPGActionInstrumented(actID, actClass);
}

std::unique_ptr<TPG::TPGEdge> TPG::TPGInstrumentedFactory::createTPGEdge(
    const TPGVertex* src, const TPGVertex* dest,
    const std::shared_ptr<Program::Program> prog) const
{
    auto ptr = std::make_unique<TPG::TPGEdgeInstrumented>(src, dest, prog);
    return ptr;
}

std::unique_ptr<TPG::TPGExecutionEngine> TPG::TPGInstrumentedFactory::
    createTPGExecutionEngine(const Environment& env, Archive* arch) const
{
    return std::make_unique<TPGExecutionEngineInstrumented>(env, arch);
}

void TPG::TPGInstrumentedFactory::resetTPGGraphCounters(
    const TPG::TPGGraph& tpg) const
{
    // Reset all vertices
    for (const TPG::TPGVertex* vertex : tpg.getVertices()) {
        const TPG::TPGVertexInstrumentation* vertexI =
            dynamic_cast<const TPG::TPGVertexInstrumentation*>(vertex);
        if (vertexI != nullptr) {
            vertexI->reset();
        }
    }

    // Reset all edges
    for (const auto& edge : tpg.getEdges()) {
        const TPG::TPGEdgeInstrumented* edgeI =
            dynamic_cast<const TPG::TPGEdgeInstrumented*>(edge.get());
        if (edgeI != nullptr) {
            edgeI->reset();
        }
    }
}

void TPG::TPGInstrumentedFactory::clearUnusedTPGGraphElements(
    TPG::TPGGraph& tpg) const
{
    // Remove unused vertices first
    // (this will remove a few edges as a side-effect)
    // Work on a copy of vertex list as the graph is modified during the for
    // loop.
    std::vector<const TPG::TPGVertex*> vertices(tpg.getVertices());
    for (const TPG::TPGVertex* vertex : vertices) {
        const TPG::TPGVertexInstrumentation* vertexI =
            dynamic_cast<const TPG::TPGVertexInstrumentation*>(vertex);
        // If the vertex is instrumented AND was never visited
        if (vertexI != nullptr && vertexI->getNbVisits() == 0) {
            // remove it
            tpg.removeVertex(*vertex);
        }
    }

    // Remove un-traversed edges
    std::vector<const TPG::TPGEdge*> edges;
    // Copy the edge list before iteration
    for (auto& edge : tpg.getEdges()) {
        edges.push_back(edge.get());
    }
    // Iterate on the edge list
    for (auto edge : edges) {
        const TPG::TPGEdgeInstrumented* edgeI =
            dynamic_cast<const TPG::TPGEdgeInstrumented*>(edge);
        if (edgeI != nullptr && edgeI->getNbTraversal() == 0) {
            tpg.removeEdge(*edge);
        }
    }
}
