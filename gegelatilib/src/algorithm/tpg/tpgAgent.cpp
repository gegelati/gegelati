
#include "algorithm/tpg/tpgAgent.h"

void Algorithm::TPG::TPGAgent::setVertex(std::shared_ptr<const EvoGraph::Vertex> newVertex)
{
    auto teamVertex = std::dynamic_pointer_cast<const EvoGraph::Team>(newVertex);
    if(teamVertex == nullptr){
        throw std::runtime_error("TPGAgent::setVertex vertex to set is not a Team");
    }

    this->vertex = teamVertex;
}

bool Algorithm::TPG::TPGAgent::isValid() const
{
    if(auto vertex = std::dynamic_pointer_cast<const EvoGraph::Team>(this->vertex)){
        return vertex->getOutgoingEdges().size() > 1;
    } else {
        return true;
    }
}


bool Algorithm::TPG::TPGAgent::isRoot() const
{
    return this->vertex->getIncomingEdges().size() == 0;
}

std::shared_ptr<const EvoGraph::Vertex> Algorithm::TPG::TPGAgent::getVertex() const
{
    std::shared_ptr<const EvoGraph::Team> vertex = std::dynamic_pointer_cast<const EvoGraph::Team>(this->vertex);
    if(vertex == nullptr){
        throw std::runtime_error("TPGAgent::getVertex element is not a Team");
    }
    
    return vertex;
}
