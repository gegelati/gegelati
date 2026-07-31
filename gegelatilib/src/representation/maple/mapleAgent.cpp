
#include "representation/maple/mapleAgent.h"

bool Representation::Maple::MapleAgent::isValid() const
{
    // Maple agent are valid if they represent an team vertex with at least one outgoing edge
    if(!this->hasVertex()){
        return false;
    } else if(auto vertex = dynamic_cast<const EvoGraph::Team*>(&this->vertex->get())){
        return vertex->getOutgoingEdges().size() > 0;
    } else {
        return false;
    }   
}