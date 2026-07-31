
#include "util/counterReset.h"

void CounterReset::counterReset()
{
    EvoGraph::Vertex::resetVertexIDCounter();
    EvoGraph::Edge::resetEdgeIDCounter();
    Representation::Individual::resetAgentIDCounter();
    Representation::Representation::resetRepresentationIDCounter();
}