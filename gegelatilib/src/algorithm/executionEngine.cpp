
#include "algorithm/executionEngine.h"

std::shared_ptr<const Algorithm::Agent> Algorithm::ExecutionEngine::getExecutedAgent() const
{
    return this->executedAgent;
}

void Algorithm::ExecutionEngine::setExecutedAgent(std::shared_ptr<const Agent> newExecutedAgent)
{
    this->executedAgent = newExecutedAgent;
}


void Algorithm::ExecutionEngine::addSubExecutionEngine(std::shared_ptr<ExecutionEngine> subExecutionEngine)
{
    this->subExecutionEngines.insert({subExecutionEngine->getAlgorithmName(), subExecutionEngine});
}

std::shared_ptr<Algorithm::ExecutionEngine> Algorithm::ExecutionEngine::getSubExecutionEngine(std::string nameAlgorithm){
    auto it = this->subExecutionEngines.find(nameAlgorithm);
    if(it == this->subExecutionEngines.end()){
        throw std::runtime_error("Algorithm::Mutator::getSubMutator subManager not found for the specific name");
    }
    return it->second;
}
