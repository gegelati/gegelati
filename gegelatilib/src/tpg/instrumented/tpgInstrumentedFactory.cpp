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
    const uint64_t id) const
{
    return new TPGActionInstrumented(id);
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
