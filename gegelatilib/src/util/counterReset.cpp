
#include "util/counterReset.h"

void CounterReset::counterReset()
{
    EvoGraph::TPGVertex::resetVertexIDCounter();
    EvoGraph::TPGEdge::resetEdgeIDCounter();
    Program::Program::resetProgramIDCounter();
}