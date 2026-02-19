
#include "algorithm/maple/mapleAgent.h"

bool Algorithm::Maple::MapleAgent::isValid() const
{
    // Maple agent are valid if they represent an team vertex with at least one outgoing edge
    if(auto vertex = dynamic_cast<const EvoGraph::Team*>(&this->vertex.get())){
        return vertex->getOutgoingEdges().size() > 0;
    } else {
        return false;
    }   
}