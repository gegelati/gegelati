
#include "representation/executionEngine.h"

const Representation::Individual& Representation::ExecutionEngine::getExecutedIndividual() const
{
    return *this->executedIndividual;
}

void Representation::ExecutionEngine::setExecutedIndividual(const Individual& newExecutedIndividual)
{
    if(newExecutedIndividual.getRepresentationID() != this->representationID){
        throw std::runtime_error("Representation::ExecutionEngine::setExecutedIndividual trying to set an individual from a different representation");
    }

    this->executedIndividual = newExecutedIndividual;
}


void Representation::ExecutionEngine::setupJob(const Representation::Job& job)
{
    this->setExecutedIndividual(job.getIndividual());
    
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