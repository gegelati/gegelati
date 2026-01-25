
#include "algorithm/tpg/tpgManager.h"


std::shared_ptr<Algorithm::TPG::TPGAgent> Algorithm::TPG::TPGManager::getTPGAgentFromCst(std::shared_ptr<const Agent> agent)
{
    auto iterator = this->agents.find(agent);
    if(iterator == this->agents.end() || *iterator != agent){
        throw std::invalid_argument("TPGManager::getTPPAgentFromCst: the given agent is not managed by this manager.");
    }

    return std::dynamic_pointer_cast<TPGAgent>(*iterator);
}

const std::vector<std::shared_ptr<const Algorithm::Agent>> Algorithm::TPG::TPGManager::getAgents() const
{
    std::vector<std::shared_ptr<const Algorithm::Agent>> constAgents;

    // Transform each root from shared_ptr<Agent> to shared_ptr<const Agent>
    for(auto agent: this->agents){
        auto tpgAgent = std::dynamic_pointer_cast<const TPGAgent>(agent);
        if(tpgAgent == nullptr){
            throw std::runtime_error("TPGManager::getAgents: an agent managed by the TPGManager is not a TPGAgent.");
        } else if (tpgAgent->isRoot()){
            constAgents.push_back(std::const_pointer_cast<const Algorithm::Agent>(agent));
        }
    }

    return constAgents;
}


std::shared_ptr<const Algorithm::Agent> Algorithm::TPG::TPGManager::createAgent(std::shared_ptr<EvoGraph::Graph> graph)
{
    std::shared_ptr<const EvoGraph::Team> vertex = graph->addNewTeam();
    this->agents.insert(std::make_shared<TPGAgent>(vertex, this->getAlgorithmName()));
    return *this->agents.rbegin();
}

std::shared_ptr<const Algorithm::Agent> Algorithm::TPG::TPGManager::createAgent(std::shared_ptr<const EvoGraph::Vertex> vertex)
{
    this->agents.insert(std::make_shared<TPGAgent>(vertex, this->getAlgorithmName()));
    return *this->agents.rbegin();
}

std::shared_ptr<const Algorithm::Agent> Algorithm::TPG::TPGManager::copyAgent(std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph)
{
    if(!this->containsAgent(agent)) {
        throw std::runtime_error("TPGManager::copyAgent: trying to copy an agent that is not managed by this manager.");
    }

    auto clonedVertex = graph->cloneVertex(*std::dynamic_pointer_cast<const TPGAgent>(agent)->getVertex());
    this->agents.insert(std::make_shared<TPGAgent>(clonedVertex, this->getAlgorithmName()));
    return *this->agents.rbegin();
}

void Algorithm::TPG::TPGManager::deleteAgent(std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph)
{
    if(!this->containsAgent(agent)) {
        throw std::runtime_error("TPGManager::deleteAgent: trying to delete an agent that is not managed by this manager.");
    }

    // Do not remove action agents from the graph
    auto tpgAgent = std::dynamic_pointer_cast<const TPGAgent>(agent);
    if(std::dynamic_pointer_cast<const EvoGraph::Team>(tpgAgent->getVertex()) != nullptr){
        graph->removeVertex(*tpgAgent->getVertex());
    }

    auto iterator = this->agents.find(agent);
    this->agents.erase(iterator);   
}

void Algorithm::TPG::TPGManager::setVertex(std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<const EvoGraph::Vertex> vertex)
{
    if(!graph->hasVertex(*vertex)){
        throw std::runtime_error("TPGManager::setVertex: trying to set the vertex to an agent, but the vertex is not in the graph.");
    }

    // Set the vertex
    this->getTPGAgentFromCst(agent)->setVertex(vertex);
}


std::unique_ptr<Algorithm::ExecutionEngine> Algorithm::TPG::TPGManager::createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources, bool isTraining) const
{
    auto engine = std::make_unique<TPG::TPGExecutionEngine>(this->outputs, this->algorithmName, this->archive, isTraining);

    engine->setProgramExecutionEngine(
        std::move(this->cGetSubManager(this->programAlgorithmName)->createExecutionEngine(dataSources, isTraining))
    );

    return engine;
}