
#include "util/counterReset.h"

void CounterReset::counterReset()
{
    TPG::TPGVertex::resetVertexIDCounter();
    TPG::TPGEdge::resetEdgeIDCounter();
    Program::Program::resetProgramIDCounter();
}