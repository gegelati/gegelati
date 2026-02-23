
#include "algorithm/species/speciesAgent.h"

std::map<std::reference_wrapper<const EvoGraph::Edge>, std::optional<std::reference_wrapper<const Algorithm::Agent>>>::iterator Algorithm::Species::SpeciesAgent::getIteratorEdge(const EvoGraph::Edge& edge)
{
    auto it = this->programs.find(edge);
    if(it == this->programs.end()){
        throw std::runtime_error("SpeciesAgent::getIteratorEdge: edge not found in the map of edge/program");
    }
    return it;
}

bool Algorithm::Species::SpeciesAgent::hasEdge(const EvoGraph::Edge& edge) const
{
    auto it = this->programs.find(edge);
    return it != this->programs.end();
}

bool Algorithm::Species::SpeciesAgent::hasProgram(const EvoGraph::Edge& edge) const
{
    if(!this->hasEdge(edge)){
        throw std::runtime_error("SpeciesAgent::hasProgram: cannot check the program on an non existing edge");
    }
    return this->programs.at(edge) != std::nullopt;
}

void Algorithm::Species::SpeciesAgent::setEdgeProgram(const EvoGraph::Edge& edge, const Agent& program)
{
    this->getIteratorEdge(edge)->second = program;
}

void Algorithm::Species::SpeciesAgent::removeEdgeProgram(const EvoGraph::Edge& edge)
{
    this->getIteratorEdge(edge)->second = std::nullopt;
}

const Algorithm::Agent& Algorithm::Species::SpeciesAgent::getProgram(const EvoGraph::Edge& edge) const
{
    if(!hasProgram(edge)) {
        throw std::runtime_error("SpeciesAgent::getProgram: program is not set");
    }
    return *this->programs.at(edge);
}

const std::map<std::reference_wrapper<const EvoGraph::Edge>, std::optional<std::reference_wrapper<const Algorithm::Agent>>>& Algorithm::Species::SpeciesAgent::getPrograms() const
{
    return this->programs;
}

bool Algorithm::Species::SpeciesAgent::isValid() const
{
    for(const auto& pair: this->programs){
        if(!this->hasProgram(pair.first)){
            return false;
        }
    }
    return true;
}