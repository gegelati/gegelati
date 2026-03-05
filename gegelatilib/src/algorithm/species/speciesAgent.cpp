
#include "algorithm/species/speciesAgent.h"

std::map<std::reference_wrapper<const EvoGraph::Edge>, std::optional<std::reference_wrapper<const Algorithm::Agent>>>::iterator Algorithm::Species::SpeciesAgent::getIteratorActionEdge(const EvoGraph::Edge& edge)
{
    auto it = this->actionPrograms.find(edge);
    if(it == this->actionPrograms.end()){
        throw std::runtime_error("SpeciesAgent::getIteratorEdge: edge not found in the map of edge/program");
    }
    return it;
}
std::map<std::reference_wrapper<const EvoGraph::Edge>, std::optional<std::reference_wrapper<const Algorithm::Agent>>>::iterator Algorithm::Species::SpeciesAgent::getIteratorContextEdge(const EvoGraph::Edge& edge)
{
    auto it = this->contextPrograms.find(edge);
    if(it == this->contextPrograms.end()){
        throw std::runtime_error("SpeciesAgent::getIteratorEdge: edge not found in the map of edge/program");
    }
    return it;
}

bool Algorithm::Species::SpeciesAgent::hasActionEdge(const EvoGraph::Edge& edge) const
{
    auto it = this->actionPrograms.find(edge);
    return it != this->actionPrograms.end();
}

bool Algorithm::Species::SpeciesAgent::hasActionProgram(const EvoGraph::Edge& edge) const
{
    if(!this->hasActionEdge(edge)){
        throw std::runtime_error("SpeciesAgent::hasActionProgram: cannot check the program on an non existing edge");
    }
    return this->actionPrograms.at(edge) != std::nullopt;
}

void Algorithm::Species::SpeciesAgent::setActionEdgeProgram(const EvoGraph::Edge& edge, const Agent& program)
{
    this->getIteratorActionEdge(edge)->second = program;
}

void Algorithm::Species::SpeciesAgent::removeActionEdgeProgram(const EvoGraph::Edge& edge)
{
    this->getIteratorActionEdge(edge)->second = std::nullopt;
}

const Algorithm::Agent& Algorithm::Species::SpeciesAgent::getActionProgram(const EvoGraph::Edge& edge) const
{
    if(!hasActionProgram(edge)) {
        throw std::runtime_error("SpeciesAgent::getProgram: program is not set");
    }
    return *this->actionPrograms.at(edge);
}

const std::map<std::reference_wrapper<const EvoGraph::Edge>, std::optional<std::reference_wrapper<const Algorithm::Agent>>>& Algorithm::Species::SpeciesAgent::getActionPrograms() const
{
    return this->actionPrograms;
}

bool Algorithm::Species::SpeciesAgent::hasContextEdge(const EvoGraph::Edge& edge) const
{
    auto it = this->contextPrograms.find(edge);
    return it != this->contextPrograms.end();
}

bool Algorithm::Species::SpeciesAgent::hasContextProgram(const EvoGraph::Edge& edge) const
{
    if(!this->hasContextEdge(edge)){
        throw std::runtime_error("SpeciesAgent::hasContextProgram: cannot check the program on an non existing edge");
    }
    return this->contextPrograms.at(edge) != std::nullopt;
}

void Algorithm::Species::SpeciesAgent::setContextEdgeProgram(const EvoGraph::Edge& edge, const Agent& program)
{
    this->getIteratorContextEdge(edge)->second = program;
}

void Algorithm::Species::SpeciesAgent::removeContextEdgeProgram(const EvoGraph::Edge& edge)
{
    this->getIteratorContextEdge(edge)->second = std::nullopt;
}

const Algorithm::Agent& Algorithm::Species::SpeciesAgent::getContextProgram(const EvoGraph::Edge& edge) const
{
    if(!hasContextProgram(edge)) {
        throw std::runtime_error("SpeciesAgent::getProgram: program is not set");
    }
    return *this->contextPrograms.at(edge);
}

const Algorithm::Agent& Algorithm::Species::SpeciesAgent::getProgram(const EvoGraph::Edge& edge) const
{
    if(hasContextEdge(edge) && hasContextProgram(edge)) { 
        return *this->contextPrograms.at(edge);
    }
    else if (hasActionEdge(edge) && hasActionProgram(edge)) {
        return *this->actionPrograms.at(edge);
    } else {
        throw std::runtime_error("SpeciesAgent::getProgram: program is not set");
    }
}

const std::map<std::reference_wrapper<const EvoGraph::Edge>, std::optional<std::reference_wrapper<const Algorithm::Agent>>>& Algorithm::Species::SpeciesAgent::getContextPrograms() const
{
    return this->contextPrograms;
}

bool Algorithm::Species::SpeciesAgent::isValid() const
{
    for(const auto& pair: this->actionPrograms){
        if(!this->hasActionProgram(pair.first)){
            return false;
        }
    }
    for(const auto& pair: this->contextPrograms){
        if(!this->hasContextProgram(pair.first)){
            return false;
        }
    }
    return true;
}