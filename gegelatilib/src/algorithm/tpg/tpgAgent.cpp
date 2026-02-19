
#include "algorithm/tpg/tpgAgent.h"

void Algorithm::TPG::TPGAgent::setVertex(const EvoGraph::Vertex& newVertex)
{
    auto teamVertex = dynamic_cast<const EvoGraph::Team*>(&newVertex);
    if(teamVertex == nullptr){
        throw std::runtime_error("TPGAgent::setVertex vertex to set is not a Team");
    }

    this->vertex = newVertex;
}

bool Algorithm::TPG::TPGAgent::isValid() const
{
    if(auto vertex = dynamic_cast<const EvoGraph::Team*>(&this->vertex.get())){
        return vertex->getOutgoingEdges().size() > 1;
    } else {
        return true;
    }
}


bool Algorithm::TPG::TPGAgent::isRoot() const
{
    return this->vertex.get().getIncomingEdges().size() == 0;
}

const EvoGraph::Vertex& Algorithm::TPG::TPGAgent::getVertex() const
{
    const EvoGraph::Team* vertex = dynamic_cast<const EvoGraph::Team*>(&this->vertex.get());
    if(vertex == nullptr){
        throw std::runtime_error("TPGAgent::getVertex element is not a Team");
    }
    
    return *vertex;
}
