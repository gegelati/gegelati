
#include "util/counterReset.h"

void CounterReset::counterReset()
{
    EvoGraph::Vertex::resetVertexIDCounter();
    EvoGraph::Edge::resetEdgeIDCounter();
    Representation::Individual::resetIndividualIDCounter();
    Representation::Representation::resetRepresentationIDCounter();
    Node::GPNode::resetGPNodeIDCounter();
    Evolution::Individual::resetIndividualIDCounter();
    Evolution::Population::resetPopulationIDCounter();
}