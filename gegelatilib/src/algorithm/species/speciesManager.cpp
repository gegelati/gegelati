
#include "algorithm/species/speciesManager.h"

Algorithm::Species::SpeciesAgent& Algorithm::Species::SpeciesManager::getSpeciesAgentFromCst(const Agent& agent)
{
    return dynamic_cast<SpeciesAgent&>(**this->getAgentFromCst(agent));
}

bool Algorithm::Species::SpeciesManager::hasRootVertex() const
{
    return this->rootVertex != std::nullopt;
}

const EvoGraph::Vertex& Algorithm::Species::SpeciesManager::getRootVertex() const
{
    return *this->rootVertex;
}

void Algorithm::Species::SpeciesManager::setRootVertex(const EvoGraph::Vertex& newRootVertex)
{
    this->rootVertex = newRootVertex;
}

const std::set<std::reference_wrapper<const EvoGraph::Edge>>& Algorithm::Species::SpeciesManager::getEdges() const
{
    return this->edges;
}

const std::set<std::reference_wrapper<const EvoGraph::Edge>>& Algorithm::Species::SpeciesManager::getContextEdges() const
{
    return this->contextEdges;
}

const std::set<std::reference_wrapper<const EvoGraph::Edge>>& Algorithm::Species::SpeciesManager::getActionEdges() const
{
    return this->actionEdges;
}

const std::set<std::reference_wrapper<const EvoGraph::Team>>& Algorithm::Species::SpeciesManager::getTeams() const
{
    return this->teams;
}
const std::set<std::reference_wrapper<const EvoGraph::Team>>& Algorithm::Species::SpeciesManager::getContextTeams() const
{
    return this->contextTeams;
}
const std::set<std::reference_wrapper<const EvoGraph::Team>>& Algorithm::Species::SpeciesManager::getActivationTeams() const
{
    return this->activationTeams;
}

const std::set<std::reference_wrapper<const EvoGraph::Action>>& Algorithm::Species::SpeciesManager::getActions() const
{
    return this->actions;
}

void Algorithm::Species::SpeciesManager::setVertexStructure(const EvoGraph::Vertex& vertex, size_t depth) {
    // Add the vertex to either the teams or the actions
    if(auto team = dynamic_cast<const EvoGraph::Team*>(&vertex)) {
        this->teams.insert(*team);
        if(depth % 2 == 1) {
            this->contextTeams.insert(*team);
        } else {
            this->activationTeams.insert(*team);
        }
    } else if (auto action = dynamic_cast<const EvoGraph::Action*>(&vertex)) {
        this->actions.insert(*action);
    } else {
        throw std::runtime_error("SpeciesManager::setVertexStructure vertex should be either a team or an action.");
    }
    for(const EvoGraph::Edge& edge: vertex.getOutgoingEdges()) { 
        // When depth is odd, destination should all be teams. All edge are context edges.
        if(depth % 2 == 1) {
            if(auto destinationTeam = dynamic_cast<const EvoGraph::Team*>(&edge.getDestination())) {
                this->edges.insert(edge);
                this->contextEdges.insert(edge);
            } else {
                throw std::runtime_error("SpeciesManager::setVertexStructure When depth is odd, destination should always be a team.");
            }
        } else {
            if(auto destinationAction = dynamic_cast<const EvoGraph::Action*>(&edge.getDestination())) {
                // When depth is even, edge pointing to an action is an action edge.
                // Edge pointing to a team are simple connexion edges without programs
                this->edges.insert(edge);
                this->actionEdges.insert(edge);
            }
        }
        this->setVertexStructure(edge.getDestination(), depth + 1);
    }
}

void Algorithm::Species::SpeciesManager::setSpeciesGraphStructure()
{
    if(!this->hasRootVertex()){
        throw std::runtime_error("SpeciesManager::setSpeciesGraphStructure no root vertex set.");
    }
    this->setVertexStructure(*this->rootVertex, 0);
}

const Algorithm::Agent& Algorithm::Species::SpeciesManager::createAgent(EvoGraph::Graph& graph)
{
    this->agents.insert(std::make_unique<SpeciesAgent>(this->getAlgorithmID(), this->outputs.size(), this->actionEdges, this->contextEdges));
    return **this->agents.rbegin();
}

const Algorithm::Agent& Algorithm::Species::SpeciesManager::copyAgent(const Agent& agent, EvoGraph::Graph& graph)
{
    const SpeciesAgent& castedAgent = dynamic_cast<const SpeciesAgent&>(agent);
    if(&castedAgent == nullptr){
        throw std::runtime_error("Algorithm::Species::SpeciesManager::copyAgent: trying to copy an agent that is not a SpeciesAgent.");
    }

    SpeciesAgent& newAgent = this->getSpeciesAgentFromCst((this->createAgent(graph)));

    if(agent.getAlgorithmID() != this->getAlgorithmID()){
        throw std::runtime_error("Algorithm::Species::SpeciesManager::copyAgent: impossible with different algorithm");
    } else {
        for(const auto& pair: castedAgent.getActionPrograms()){
            newAgent.setActionEdgeProgram(pair.first, *pair.second);
        }
        for(const auto& pair: castedAgent.getContextPrograms()){
            newAgent.setContextEdgeProgram(pair.first, *pair.second);
        }
    }

    return newAgent;
}

void Algorithm::Species::SpeciesManager::emptyAgent(const Agent& agent, EvoGraph::Graph& graph)
{
    SpeciesAgent& speciesAgent = this->getSpeciesAgentFromCst(agent);
    for(const auto& pair: speciesAgent.getActionPrograms()){
        speciesAgent.removeActionEdgeProgram(pair.first);
    }
    for(const auto& pair: speciesAgent.getContextPrograms()){
        speciesAgent.removeContextEdgeProgram(pair.first);
    }
}

void Algorithm::Species::SpeciesManager::setProgram(const Agent& agent, const EvoGraph::Edge& edge, const Agent& program)
{
    if(program.getAlgorithmID() != this->programAlgorithmID) {
        throw std::runtime_error("SpeciesManager::setProgram: ID of program is not the ID of the program Algorithm.");
    }

    if(this->actionEdges.find(edge) != this->actionEdges.end()) {
        this->getSpeciesAgentFromCst(agent).setActionEdgeProgram(edge, program);
    } else {
        this->getSpeciesAgentFromCst(agent).setContextEdgeProgram(edge, program);
    }
}

void Algorithm::Species::SpeciesManager::setActionValue(const Agent& agent, size_t actionID, size_t actionValue)
{
    this->getSpeciesAgentFromCst(agent).setActionLink(actionID, actionValue);
}

std::unique_ptr<Algorithm::ExecutionEngine> Algorithm::Species::SpeciesManager::createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources, bool isTraining) const
{
    auto engine = std::make_unique<Species::SpeciesExecutionEngine>(*this->rootVertex, this->outputs, this->algorithmID, isTraining);

    engine->setProgramExecutionEngine(
        std::move(this->cGetSubManager(this->programAlgorithmID).createExecutionEngine(dataSources, isTraining))
    );

    return engine;
}