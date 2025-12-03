
#include "algorithm/tpg/tpgAgent.h"


const EvoGraph::Vertex& Algorithm::TPG::TPGAgent::getVertex() const
{
    return *this->vertex;
}

bool Algorithm::TPG::TPGAgent::isValid() const
{
    return this->vertex->getOutgoingEdges().size() > 2;
}

void Algorithm::TPG::TPGAgent::setVertex(const EvoGraph::Vertex& newVertex)
{
    this->vertex = &newVertex;
}

bool Algorithm::TPG::TPGAgent::isRoot() const
{
    return this->vertex->getIncomingEdges().size() == 0;
}
