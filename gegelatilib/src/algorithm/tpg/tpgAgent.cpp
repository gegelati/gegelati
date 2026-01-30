
#include "algorithm/tpg/tpgAgent.h"

void Algorithm::TPG::TPGAgent::setElement(std::shared_ptr<const EvoGraph::Element> newElement)
{
    auto newVertex = std::dynamic_pointer_cast<const EvoGraph::Team>(newElement);
    if(newVertex == nullptr){
        throw std::runtime_error("TPGAgent::setElement element to set is not a Team");
    }

    this->element = newVertex;
}

bool Algorithm::TPG::TPGAgent::isValid() const
{
    if(auto vertex = std::dynamic_pointer_cast<const EvoGraph::Team>(this->element)){
        return vertex->getOutgoingEdges().size() > 1;
    } else {
        return true;
    }
}


bool Algorithm::TPG::TPGAgent::isRoot() const
{
    return std::dynamic_pointer_cast<const EvoGraph::Team>(this->element)->getIncomingEdges().size() == 0;
}

std::shared_ptr<const EvoGraph::Vertex> Algorithm::TPG::TPGAgent::getVertex() const
{
    std::shared_ptr<const EvoGraph::Team> vertex = std::dynamic_pointer_cast<const EvoGraph::Team>(this->element);
    if(vertex == nullptr){
        throw std::runtime_error("TPGAgent::getVertex element is not a Team");
    }
    
    return vertex;
}
