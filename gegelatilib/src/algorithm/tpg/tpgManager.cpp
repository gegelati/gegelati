
#include "algorithm/tpg/tpgManager.h"




const std::vector<std::shared_ptr<const Algorithm::Agent>> Algorithm::TPG::TPGManager::getAgents() const
{
    std::vector<std::shared_ptr<const Algorithm::Agent>> constAgents;

    // Transform each element from shared_ptr<Agent> to shared_ptr<const Agent>
    std::transform(
        this->agents.begin(),
        this->agents.end(),
        std::back_inserter(constAgents),
        [](const std::shared_ptr<Algorithm::Agent>& agent) {
            return std::const_pointer_cast<const Algorithm::Agent>(agent);
        }
    );

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
    const EvoGraph::Vertex& vertex = graph->addNewTeam();
    this->agents.insert(std::make_shared<TPGAgent>(vertex));
    return std::const_pointer_cast<const Algorithm::Agent>(*this->agents.rbegin());
}

std::shared_ptr<const Algorithm::Agent> Algorithm::TPG::TPGManager::copyAgent(std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph)
{
    if(!this->containsAgent(agent)) {
        throw std::runtime_error("TPGManager::copyAgent: trying to copy an agent that is not managed by this manager.");
    }

    auto clonedVertex = graph->cloneVertex(
        dynamic_cast<const TPGAgent&>(*agent).getVertex());
    this->agents.insert(std::make_shared<TPGAgent>(clonedVertex));
    return std::const_pointer_cast<const Algorithm::Agent>(*this->agents.rbegin());
}

void Algorithm::TPG::TPGManager::deleteAgent(std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph)
{
    if(!this->containsAgent(agent)) {
        throw std::runtime_error("TPGManager::deleteAgent: trying to delete an agent that is not managed by this manager.");
    }

    // Do not remove action agents from the graph
    auto tpgAgent = std::dynamic_pointer_cast<const TPGAgent>(agent);
    graph->removeVertex(tpgAgent->getVertex());

    auto iterator = this->agents.find(agent);
    this->agents.erase(iterator);   
}
