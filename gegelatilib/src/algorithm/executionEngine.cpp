
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