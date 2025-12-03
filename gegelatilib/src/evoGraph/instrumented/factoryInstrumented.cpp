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

#include "evoGraph/instrumented/factoryInstrumented.h"
#include "evoGraph/instrumented/actionInstrumented.h"
#include "evoGraph/instrumented/edgeInstrumented.h"
#include "evoGraph/instrumented/executionEngineInstrumented.h"
#include "evoGraph/instrumented/teamInstrumented.h"

std::shared_ptr<EvoGraph::Graph> EvoGraph::TPGInstrumentedFactory::createGraph(
    const Environment& env) const
{
    return std::make_shared<EvoGraph::Graph>(
        env, std::make_unique<TPGInstrumentedFactory>());
}

std::unique_ptr<EvoGraph::Team> EvoGraph::TPGInstrumentedFactory::createTeam() const
{
    return std::make_unique<TeamInstrumented>();
}

std::unique_ptr<EvoGraph::Action> EvoGraph::TPGInstrumentedFactory::createAction(
    const uint64_t id) const
{
    return std::make_unique<ActionInstrumented>(id);
}

std::unique_ptr<EvoGraph::Edge> EvoGraph::TPGInstrumentedFactory::createEdge(
    const Vertex* src, const Vertex* dest,
    const std::shared_ptr<Program::Program> prog) const
{
    auto ptr = std::make_unique<EvoGraph::EdgeInstrumented>(src, dest, prog);
    return ptr;
}

std::unique_ptr<EvoGraph::ExecutionEngine> EvoGraph::TPGInstrumentedFactory::
    createExecutionEngine(const Environment& env, Archive* arch) const
{
    return std::make_unique<ExecutionEngineInstrumented>(env, arch);
}

void EvoGraph::TPGInstrumentedFactory::resetGraphCounters(
    const EvoGraph::Graph& tpg) const
{
    // Reset all vertices
    for (const EvoGraph::Vertex* vertex : tpg.getVertices()) {
        const EvoGraph::VertexInstrumentation* vertexI =
            dynamic_cast<const EvoGraph::VertexInstrumentation*>(vertex);
        if (vertexI != nullptr) {
            vertexI->reset();
        }
    }

    // Reset all edges
    for (const auto& edge : tpg.getEdges()) {
        const EvoGraph::EdgeInstrumented* edgeI =
            dynamic_cast<const EvoGraph::EdgeInstrumented*>(edge.get());
        if (edgeI != nullptr) {
            edgeI->reset();
        }
    }
}

void EvoGraph::TPGInstrumentedFactory::clearUnusedGraphElements(
    EvoGraph::Graph& tpg) const
{
    // Remove unused vertices first
    // (this will remove a few edges as a side-effect)
    // Work on a copy of vertex list as the graph is modified during the for
    // loop.
    std::vector<const EvoGraph::Vertex*> vertices(tpg.getVertices());
    for (const EvoGraph::Vertex* vertex : vertices) {
        const EvoGraph::VertexInstrumentation* vertexI =
            dynamic_cast<const EvoGraph::VertexInstrumentation*>(vertex);
        // If the vertex is instrumented AND was never visited
        if (vertexI != nullptr && vertexI->getNbVisits() == 0) {
            // remove it
            tpg.removeVertex(*vertex);
        }
    }

    // Remove un-traversed edges
    std::vector<const EvoGraph::Edge*> edges;
    // Copy the edge list before iteration
    for (auto& edge : tpg.getEdges()) {
        edges.push_back(edge.get());
    }
    // Iterate on the edge list
    for (auto edge : edges) {
        const EvoGraph::EdgeInstrumented* edgeI =
            dynamic_cast<const EvoGraph::EdgeInstrumented*>(edge);
        if (edgeI != nullptr && edgeI->getNbTraversal() == 0) {
            tpg.removeEdge(*edge);
        }
    }
}
