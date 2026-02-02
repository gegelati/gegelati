
#include "algorithm/maple/mapleAgent.h"

bool Algorithm::Maple::MapleAgent::isValid() const
{
    // Maple agent are valid if they represent an team vertex with at least one outgoing edge
    if(auto vertex = std::dynamic_pointer_cast<const EvoGraph::Team>(this->vertex)){
        return vertex->getOutgoingEdges().size() > 0;
    } else {
        return false;
    }   
}