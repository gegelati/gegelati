
#include "algorithm/species/speciesManager.h"

Algorithm::Species::SpeciesAgent& Algorithm::Species::SpeciesManager::getSpeciesAgentFromCst(const Agent& agent)
{
    return dynamic_cast<SpeciesAgent&>(**this->getAgentFromCst(agent));
}

const std::vector<std::reference_wrapper<const Algorithm::Agent>> Algorithm::Species::SpeciesManager::getAgents() const
{
    std::vector<std::reference_wrapper<const Algorithm::Agent>> constAgents;

    // Transform each root from shared_ptr<Agent> to shared_ptr<const Agent>
    for(auto it = this->agents.begin(); it != this->agents.end(); ){
        const SpeciesAgent* speciesAgent = dynamic_cast<const SpeciesAgent*>((*it).get());
        if(speciesAgent == nullptr){
            throw std::runtime_error("SpeciesManager::getAgents: an agent managed by the SpeciesManager is not a SpeciesAgent.");
        } else if (speciesAgent->isRoot()){
            constAgents.push_back(**it);
        }
        it++;
    }

    return constAgents;
}


const Algorithm::Agent& Algorithm::Species::SpeciesManager::createAgent(EvoGraph::Graph& graph)
{
    return this->createAgent(graph.addNewTeam());
}

const Algorithm::Agent& Algorithm::Species::SpeciesManager::createAgent(std::optional<std::reference_wrapper<const EvoGraph::Vertex>> vertex)
{
    this->agents.insert(std::make_unique<SpeciesAgent>(vertex, this->getAlgorithmID()));
    return **this->agents.rbegin();
}

const Algorithm::Agent& Algorithm::Species::SpeciesManager::createEmptyAgent()
{
    std::optional<std::reference_wrapper<const EvoGraph::Vertex>> vertex = std::nullopt;
    return this->createAgent(vertex);
}

const Algorithm::Agent& Algorithm::Species::SpeciesManager::copyAgent(const Agent& agent, EvoGraph::Graph& graph)
{
    const SpeciesAgent& castedAgent = dynamic_cast<const SpeciesAgent&>(agent);
    if(&castedAgent == nullptr){
        throw std::runtime_error("Algorithm::Species::SpeciesManager::copyAgent: trying to copy an agent that is not a SpeciesAgent.");
    }

    // Set to castedAgent to avoid unset references
    std::reference_wrapper<const EvoGraph::Vertex> newVertex = castedAgent.getVertex();

    if(agent.getAlgorithmID() != this->getAlgorithmID()){
        // Since the agent dupplicated is not from the same algorithm, we also need to dupplicate the sub agents on the edge of the vertex.
        newVertex = graph.addNewTeam();
        for(const EvoGraph::Edge& edge: castedAgent.getVertex().getOutgoingEdges()){
            const Algorithm::Agent& newSubAgent = this->getSubManager(this->programAlgorithmID).copyAgent(edge.getProgram(), graph);
            graph.addNewEdge(newVertex, edge.getDestination(), newSubAgent);
        }
        
    } else {
        newVertex = graph.cloneVertex(castedAgent.getVertex());
    }

    return this->createAgent(newVertex);
}

void Algorithm::Species::SpeciesManager::deleteAgent(const Agent& agent, EvoGraph::Graph& graph)
{
    this->emptyAgent(agent, graph);
    // Do not remove action agents from the graph
    if(auto vertex = dynamic_cast<const EvoGraph::Team*>(&this->getSpeciesAgentFromCst(agent).getVertex())){
        graph.removeVertex(*vertex);
    }

    auto iterator = this->agents.find(&agent);
    this->agents.erase(iterator);   
}
void Algorithm::Species::SpeciesManager::emptyAgent(const Agent& agent, EvoGraph::Graph& graph)
{
    // Do not remove action agents from the graph
    if(auto vertex = dynamic_cast<const EvoGraph::Team*>(&this->getSpeciesAgentFromCst(agent).getVertex())){
        while(vertex->getOutgoingEdges().size() > 0){
            graph.removeEdge(vertex->getOutgoingEdges().front());
        }
    }
}

void Algorithm::Species::SpeciesManager::setVertex(const Agent& agent, const EvoGraph::Vertex& vertex)
{
    const EvoGraph::Team& team = dynamic_cast<const EvoGraph::Team&>(vertex);
    if(&team == nullptr){
        throw std::runtime_error("SpeciesManager::setVertex: trying to set an agent on a vertex from the graph that is not a team.");
    }

    // Set the element
    this->getSpeciesAgentFromCst(agent).setVertex(vertex);
}


std::unique_ptr<Algorithm::ExecutionEngine> Algorithm::Species::SpeciesManager::createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources, bool isTraining) const
{
    auto engine = std::make_unique<Species::SpeciesExecutionEngine>(this->outputs, this->algorithmID, isTraining);

    engine->setProgramExecutionEngine(
        std::move(this->cGetSubManager(this->programAlgorithmID).createExecutionEngine(dataSources, isTraining))
    );

    return engine;
}