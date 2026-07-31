
#include "representation/tpg/tpgIndividual.h"

void Representation::TPG::TpgIndividual::setVertex(const EvoGraph::Vertex& newVertex)
{
    auto teamVertex = dynamic_cast<const EvoGraph::Team*>(&newVertex);
    if(teamVertex == nullptr){
        throw std::runtime_error("TpgIndividual::setVertex vertex to set is not a Team");
    }

    this->vertex = newVertex;
}

bool Representation::TPG::TpgIndividual::isValid() const
{
    if(!this->hasVertex()){
        return false;
    } else if(auto vertex = dynamic_cast<const EvoGraph::Team*>(&this->vertex->get())){
        return vertex->getOutgoingEdges().size() > 1;
    } else {
        return true;
    }
}

bool Representation::TPG::TpgIndividual::hasVertex() const
{
    return this->vertex != std::nullopt;
}

void Representation::TPG::TpgIndividual::removeVertex()
{
    this->vertex == std::nullopt;
}


bool Representation::TPG::TpgIndividual::isRoot() const
{
    return this->vertex->get().getIncomingEdges().size() == 0;
}

const EvoGraph::Vertex& Representation::TPG::TpgIndividual::getVertex() const
{
    if(!this->vertex){
        throw std::runtime_error("TpgIndividual::getVertex no vertex set");
    }
    const EvoGraph::Team* vertex = dynamic_cast<const EvoGraph::Team*>(&this->vertex->get());
    if(vertex == nullptr){
        throw std::runtime_error("TpgIndividual::getVertex vertex is not a Team");
    }
    
    return *vertex;
}
