
#include "algorithm/tpg/tpgAgent.h"


std::shared_ptr<const EvoGraph::Vertex> Algorithm::TPG::TPGAgent::getVertex() const
{
    return this->vertex;
}

bool Algorithm::TPG::TPGAgent::isValid() const
{
    if(std::dynamic_pointer_cast<const EvoGraph::Team>(this->vertex) != nullptr){
        return this->vertex->getOutgoingEdges().size() > 1;
    } else {
        return true;
    }
}

void Algorithm::TPG::TPGAgent::setVertex(std::shared_ptr<const EvoGraph::Vertex> newVertex)
{
    this->vertex = newVertex;
}

bool Algorithm::TPG::TPGAgent::isRoot() const
{
    return this->vertex->getIncomingEdges().size() == 0;
}
