
#include "util/counterReset.h"

void CounterReset::counterReset()
{
    EvoGraph::Vertex::resetVertexIDCounter();
    EvoGraph::Edge::resetEdgeIDCounter();
    Algorithm::Agent::resetAgentIDCounter();
    Algorithm::Algorithm::resetAlgorithmIDCounter();
}