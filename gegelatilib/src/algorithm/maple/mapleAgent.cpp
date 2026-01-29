
#include "algorithm/maple/mapleAgent.h"

bool Algorithm::Maple::MapleAgent::isValid() const
{
    // Maple agent are valid if they represent an team vertex with at least one edge
    if(std::dynamic_pointer_cast<const EvoGraph::Team>(this->vertex) != nullptr) {
        return this->vertex->getOutgoingEdges().size() > 0;
    } else {
        return false;
    }
    
}

void Algorithm::Maple::MapleAgent::setVertex(std::shared_ptr<const EvoGraph::Vertex> newVertex)
{
    this->vertex = newVertex;
}

bool Algorithm::Maple::MapleAgent::isRoot() const
{
    // Maple agents should always be a root, as long as it is valid.
    if(!this->isValid()) {
        throw std::runtime_error("Cannot determine if an invalid MapleAgent is root.");
    }
    return true;
}