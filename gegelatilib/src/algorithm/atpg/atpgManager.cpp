
#include "algorithm/atpg/atpgManager.h"

std::shared_ptr<Algorithm::ATPG::ATPGAgent> Algorithm::ATPG::ATPGManager::getATPGAgentFromCst(std::shared_ptr<const Agent> agent)
{
    auto iterator = this->agents.find(agent);
    if(iterator == this->agents.end() || *iterator != agent){
        throw std::invalid_argument("ATPGManager::getATPGAgentFromCst: the given agent is not managed by this manager.");
    }

    return std::dynamic_pointer_cast<ATPGAgent>(*iterator);
}

std::shared_ptr<const Algorithm::Agent> Algorithm::ATPG::ATPGManager::createAgent(std::shared_ptr<const EvoGraph::Vertex> vertex)
{
    this->agents.insert(std::make_shared<ATPGAgent>(vertex, this->getAlgorithmName()));
    return *this->agents.rbegin();
}


void Algorithm::ATPG::ATPGManager::deleteAgent(std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph) 
{
    // Remove the action vertex from the graph
    graph->removeVertex(*this->getATPGAgentFromCst(agent)->getVertex());

    auto iterator = this->agents.find(agent);
    this->agents.erase(iterator);   
}

std::unique_ptr<Algorithm::ExecutionEngine> Algorithm::ATPG::ATPGManager::createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources, bool isTraining) const
{
    auto engine = std::make_unique<ATPG::ATPGExecutionEngine>(this->outputs, this->algorithmName, this->archive, isTraining);

    engine->setProgramExecutionEngine(
        std::move(this->cGetSubManager(this->programAlgorithmName)->createExecutionEngine(dataSources, isTraining))
    );

    engine->setActionProgramExecutionEngine(
        std::move(this->cGetSubManager(this->actionProgramAlgorithmName)->createExecutionEngine(dataSources, isTraining))
    );

    return engine;
}