
#include "algorithm/species/speciesManager.h"

Algorithm::Species::SpeciesAgent& Algorithm::Species::SpeciesManager::getSpeciesAgentFromCst(const Agent& agent)
{
    return dynamic_cast<SpeciesAgent&>(**this->getAgentFromCst(agent));
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

const std::set<std::reference_wrapper<const EvoGraph::Action>>& Algorithm::Species::SpeciesManager::getActions() const
{
    return this->actions;
}

void Algorithm::Species::SpeciesManager::setVertexStructure(const EvoGraph::Vertex& vertex, size_t depth) {
    // Add the vertex to either the teams or the actions
    if(auto team = dynamic_cast<const EvoGraph::Team*>(&vertex)) {
        this->teams.insert(*team);
    } else if (auto action = dynamic_cast<const EvoGraph::Action*>(&vertex)) {
        this->actions.insert(*action);
    } else {
        throw std::runtime_error("SpeciesManager::setSpeciesGraphStructure vertex should be either a team or an action.");
    }

    for(const EvoGraph::Edge& edge: vertex.getOutgoingEdges()) { 
        // When depth is odd, destination should all be teams. All edge are context edges.
        if(depth % 2 == 1) {
            if(auto destinationTeam = dynamic_cast<const EvoGraph::Team*>(&edge.getDestination())) {
                this->edges.insert(edge);
                this->contextEdges.insert(edge);
            } else {
                throw std::runtime_error("SpeciesManager::setSpeciesGraphStructure When depth is odd, destination should always be a team.");
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

void Algorithm::Species::SpeciesManager::setSpeciesGraphStructure(const EvoGraph::Vertex& rootVertex)
{
    this->rootVertex = rootVertex;
    this->setVertexStructure(rootVertex, 0);

    std::set<std::reference_wrapper<const EvoGraph::Vertex>> visitedTeams{rootVertex};
    while(visitedTeams.size() > 0) {
        const EvoGraph::Vertex& currentVertex = *visitedTeams.begin();
        visitedTeams.erase(visitedTeams.begin());

        // Add the vertex to either the teams or the actions
        if(auto team = dynamic_cast<const EvoGraph::Team*>(&currentVertex)) {
            this->teams.insert(*team);
        } else if (auto action = dynamic_cast<const EvoGraph::Action*>(&currentVertex)) {
            this->actions.insert(*action);
        } else {
            throw std::runtime_error("SpeciesManager::setSpeciesGraphStructure vertex should be either a team or an action.");
        }
        
        // Add the edge to the edges if it contains a program, and to either the action or context edges, and its destination is respectively an action or a team.
        for(const EvoGraph::Edge& edge: currentVertex.getOutgoingEdges()) {
            visitedTeams.insert(edge.getDestination());

            if(edge.hasProgram()) {
                this->edges.insert(edge);
                if(dynamic_cast<const EvoGraph::Action*>(&edge.getDestination()) != nullptr) {
                    this->actionEdges.insert(edge);
                } else {
                    this->contextEdges.insert(edge);
                }
            }
        }
    }
}

const Algorithm::Agent& Algorithm::Species::SpeciesManager::createAgent(EvoGraph::Graph& graph)
{
    this->agents.insert(std::make_unique<SpeciesAgent>(this->getAlgorithmID(), this->edges));
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
        for(const auto& pair: castedAgent.getPrograms()){
            newAgent.setEdgeProgram(pair.first, *pair.second);
        }
    }

    return newAgent;
}

void Algorithm::Species::SpeciesManager::emptyAgent(const Agent& agent, EvoGraph::Graph& graph)
{
    SpeciesAgent& speciesAgent = this->getSpeciesAgentFromCst(agent);
    for(const auto& pair: speciesAgent.getPrograms()){
        speciesAgent.removeEdgeProgram(pair.first);
    }
}

void Algorithm::Species::SpeciesManager::setProgram(const Agent& agent, const EvoGraph::Edge& edge, const Agent& program)
{
    if(program.getAlgorithmID() != this->programAlgorithmID) {
        throw std::runtime_error("SpeciesManager::setProgram: ID of progrma is not the ID of the program Algorithm.");
    }

    this->getSpeciesAgentFromCst(agent).setEdgeProgram(edge, program);
}

std::unique_ptr<Algorithm::ExecutionEngine> Algorithm::Species::SpeciesManager::createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources, bool isTraining) const
{
    auto engine = std::make_unique<Species::SpeciesExecutionEngine>(*this->rootVertex, this->outputs, this->algorithmID, isTraining);

    engine->setProgramExecutionEngine(
        std::move(this->cGetSubManager(this->programAlgorithmID).createExecutionEngine(dataSources, isTraining))
    );

    return engine;
}