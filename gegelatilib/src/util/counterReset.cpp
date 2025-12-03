
#include "util/counterReset.h"

void CounterReset::counterReset()
{
    EvoGraph::Vertex::resetVertexIDCounter();
    EvoGraph::Edge::resetEdgeIDCounter();
    Program::Program::resetProgramIDCounter();
    Algorithm::Agent::resetAgentIDCounter();
}