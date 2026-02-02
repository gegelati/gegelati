
#include "algorithm/executionEngine.h"

std::shared_ptr<const Algorithm::Agent> Algorithm::ExecutionEngine::getExecutedAgent() const
{
    return this->executedAgent;
}

void Algorithm::ExecutionEngine::setExecutedAgent(std::shared_ptr<const Agent> newExecutedAgent)
{
    if(newExecutedAgent->getAlgorithmName() != this->algorithmName){
        throw std::runtime_error("Algorithm::ExecutionEngine::setExecutedAgent trying to set an agent from a different algorithm");
    }

    this->executedAgent = newExecutedAgent;
}


void Algorithm::ExecutionEngine::setupJob(const Algorithm::Job& job)
{
    this->setExecutedAgent(job.getAgent());
    
}

void Algorithm::ExecutionEngine::setDataSources(
    const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSrc)
{
    this->dataSources = dataSrc;
    auto itSubExec = this->subExecutionEngines.begin();
    while(itSubExec != this->subExecutionEngines.end()){
        itSubExec->second->setDataSources(dataSrc);
        itSubExec++;
    }
}

void Algorithm::ExecutionEngine::setExecutionMode(bool isTraining)
{
    this->isTraining = isTraining;

    for(const auto& pairSubExecEngine: this->subExecutionEngines){
        pairSubExecEngine.second->setExecutionMode(isTraining);
    }
}