
#include "util/counterReset.h"

void CounterReset::counterReset()
{
    EvoGraph::Vertex::resetVertexIDCounter();
    EvoGraph::Edge::resetEdgeIDCounter();
    Representation::Individual::resetIndividualIDCounter();
    Representation::Representation::resetRepresentationIDCounter();
    GraphBased::GPNode::resetGPNodeIDCounter();
    Individual::resetIndividualIDCounter();
    Population::resetPopulationIDCounter();
}