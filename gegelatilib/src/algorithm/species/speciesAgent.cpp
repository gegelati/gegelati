
#include "algorithm/species/speciesAgent.h"

void Algorithm::Species::SpeciesAgent::setVertex(const EvoGraph::Vertex& newVertex)
{
    auto teamVertex = dynamic_cast<const EvoGraph::Team*>(&newVertex);
    if(teamVertex == nullptr){
        throw std::runtime_error("SpeciesAgent::setVertex vertex to set is not a Team");
    }

    this->vertex = newVertex;
}

bool Algorithm::Species::SpeciesAgent::isValid() const
{
    if(!this->hasVertex()){
        return false;
    } else if(auto vertex = dynamic_cast<const EvoGraph::Team*>(&this->vertex->get())){
        return vertex->getOutgoingEdges().size() > 1;
    } else {
        return true;
    }
}

bool Algorithm::Species::SpeciesAgent::hasVertex() const
{
    return this->vertex != std::nullopt;
}

void Algorithm::Species::SpeciesAgent::removeVertex()
{
    this->vertex == std::nullopt;
}


bool Algorithm::Species::SpeciesAgent::isRoot() const
{
    return this->vertex->get().getIncomingEdges().size() == 0;
}

const EvoGraph::Vertex& Algorithm::Species::SpeciesAgent::getVertex() const
{
    if(!this->vertex){
        throw std::runtime_error("SpeciesAgent::getVertex no vertex set");
    }
    const EvoGraph::Team* vertex = dynamic_cast<const EvoGraph::Team*>(&this->vertex->get());
    if(vertex == nullptr){
        throw std::runtime_error("SpeciesAgent::getVertex vertex is not a Team");
    }
    
    return *vertex;
}
