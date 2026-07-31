
#include "representation/executionEngine.h"

const Representation::Individual& Representation::ExecutionEngine::getExecutedAgent() const
{
    return *this->executedAgent;
}

void Representation::ExecutionEngine::setExecutedAgent(const Individual& newExecutedAgent)
{
    if(newExecutedAgent.getRepresentationID() != this->representationID){
        throw std::runtime_error("Representation::ExecutionEngine::setExecutedAgent trying to set an agent from a different representation");
    }

    this->executedAgent = newExecutedAgent;
}


void Representation::ExecutionEngine::setupJob(const Representation::Job& job)
{
    this->setExecutedAgent(job.getAgent());
    
}

void Representation::ExecutionEngine::setDataSources(
    const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSrc)
{
    this->dataSources = dataSrc;
    auto itSubExec = this->subExecutionEngines.begin();
    while(itSubExec != this->subExecutionEngines.end()){
        itSubExec->second->setDataSources(dataSrc);
        itSubExec++;
    }
}

void Representation::ExecutionEngine::setExecutionMode(bool isTraining)
{
    this->isTraining = isTraining;

    for(const auto& pairSubExecEngine: this->subExecutionEngines){
        pairSubExecEngine.second->setExecutionMode(isTraining);
    }
}