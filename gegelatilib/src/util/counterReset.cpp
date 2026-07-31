
#include "util/counterReset.h"

void CounterReset::counterReset()
{
    EvoGraph::Vertex::resetVertexIDCounter();
    EvoGraph::Edge::resetEdgeIDCounter();
    Representation::Agent::resetAgentIDCounter();
    Representation::Representation::resetRepresentationIDCounter();
}