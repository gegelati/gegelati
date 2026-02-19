
#include "algorithm/tpg/tpgManager.h"

Algorithm::TPG::TPGAgent& Algorithm::TPG::TPGManager::getTPGAgentFromCst(const Agent& agent)
{
    return dynamic_cast<TPGAgent&>(**this->getAgentFromCst(agent));
}

const std::vector<std::reference_wrapper<const Algorithm::Agent>> Algorithm::TPG::TPGManager::getAgents() const
{
    std::vector<std::reference_wrapper<const Algorithm::Agent>> constAgents;

    // Transform each root from shared_ptr<Agent> to shared_ptr<const Agent>
    for(auto it = this->agents.begin(); it != this->agents.end(); ){
        const TPGAgent* tpgAgent = dynamic_cast<const TPGAgent*>((*it).get());
        if(tpgAgent == nullptr){
            throw std::runtime_error("TPGManager::getAgents: an agent managed by the TPGManager is not a TPGAgent.");
        } else if (tpgAgent->isRoot()){
            constAgents.push_back(**it);
        }
        it++;
    }

    return constAgents;
}


const Algorithm::Agent& Algorithm::TPG::TPGManager::createAgent(EvoGraph::Graph& graph)
{
    return this->createAgent(graph.addNewTeam());
}

const Algorithm::Agent& Algorithm::TPG::TPGManager::createAgent(std::optional<std::reference_wrapper<const EvoGraph::Vertex>> vertex)
{
    this->agents.insert(std::make_unique<TPGAgent>(vertex, this->getAlgorithmID()));
    return **this->agents.rbegin();
}

const Algorithm::Agent& Algorithm::TPG::TPGManager::createEmptyAgent()
{
    std::optional<std::reference_wrapper<const EvoGraph::Vertex>> vertex = std::nullopt;
    return this->createAgent(vertex);
}

const Algorithm::Agent& Algorithm::TPG::TPGManager::copyAgent(const Agent& agent, EvoGraph::Graph& graph)
{
    const TPGAgent& castedAgent = dynamic_cast<const TPGAgent&>(agent);
    if(&castedAgent == nullptr){
        throw std::runtime_error("Algorithm::TPG::TPGManager::copyAgent: trying to copy an agent that is not a TPGAgent.");
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

void Algorithm::TPG::TPGManager::deleteAgent(const Agent& agent, EvoGraph::Graph& graph)
{
    this->emptyAgent(agent, graph);
    // Do not remove action agents from the graph
    if(auto vertex = dynamic_cast<const EvoGraph::Team*>(&this->getTPGAgentFromCst(agent).getVertex())){
        graph.removeVertex(*vertex);
    }

    auto iterator = this->agents.find(&agent);
    this->agents.erase(iterator);   
}
void Algorithm::TPG::TPGManager::emptyAgent(const Agent& agent, EvoGraph::Graph& graph)
{
    // Do not remove action agents from the graph
    if(auto vertex = dynamic_cast<const EvoGraph::Team*>(&this->getTPGAgentFromCst(agent).getVertex())){
        while(vertex->getOutgoingEdges().size() > 0){
            graph.removeEdge(vertex->getOutgoingEdges().front());
        }
    }
}

void Algorithm::TPG::TPGManager::setVertex(const Agent& agent, const EvoGraph::Vertex& vertex)
{
    const EvoGraph::Team& team = dynamic_cast<const EvoGraph::Team&>(vertex);
    if(&team == nullptr){
        throw std::runtime_error("TPGManager::setVertex: trying to set an agent on a vertex from the graph that is not a team.");
    }

    // Set the element
    this->getTPGAgentFromCst(agent).setVertex(vertex);
}


std::unique_ptr<Algorithm::ExecutionEngine> Algorithm::TPG::TPGManager::createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources, bool isTraining) const
{
    auto engine = std::make_unique<TPG::TPGExecutionEngine>(this->outputs, this->algorithmID, isTraining);

    engine->setProgramExecutionEngine(
        std::move(this->cGetSubManager(this->programAlgorithmID).createExecutionEngine(dataSources, isTraining))
    );

    return engine;
}