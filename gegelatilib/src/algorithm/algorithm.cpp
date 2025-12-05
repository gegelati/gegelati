
#include "algorithm/algorithm.h"

void Algorithm::Algorithm::addSubAlgorithm(std::shared_ptr<Algorithm> subAlgorithm)
{
    this->subAlgorithms.push_back(subAlgorithm);
    this->manager->addSubManager(subAlgorithm->getManager());
    this->mutator->addSubMutator(subAlgorithm->getMutator());
}

std::shared_ptr<const Algorithm::AgentManager> Algorithm::Algorithm::getManagerCst() const
{
    return this->manager;
}

std::shared_ptr<const Selector::Selector> Algorithm::Algorithm::getSelectorCst() const
{
    return this->selector;
}

std::shared_ptr<Algorithm::AgentManager> Algorithm::Algorithm::getManager()
{
    return this->manager;
}

std::shared_ptr<Selector::Selector> Algorithm::Algorithm::getSelector()
{
    return this->selector;
}

std::shared_ptr<Algorithm::Mutator> Algorithm::Algorithm::getMutator()
{
    return this->mutator;
}

size_t Algorithm::Algorithm::getNbAgents() const
{
    return this->manager->getAgents().size();   
}

const std::vector<std::shared_ptr<const Algorithm::Agent>> Algorithm::Algorithm::getAgents() const
{
    return this->manager->getAgents();
}

bool Algorithm::Algorithm::containsAgent(std::shared_ptr<const Agent> agent) const
{
    return this->manager->containsAgent(agent);
}

void Algorithm::Algorithm::init(RNG::RNG& rng)
{
    // Initialize a random population
    this->mutator->initRandomPopulation(this->graph, this->manager, this->params, rng, this->nbOutputs);

    this->mutator->mutatePopulation(this->graph, this->manager, this->selector, this->params, rng, this->nbOutputs);

    // Clear the best agent in the selector
    this->selector->forgetPreviousResults();
}

void Algorithm::Algorithm::populate(RNG::RNG& rng, size_t maxNbThreads)
{
    this->mutator->mutatePopulation(this->graph, this->manager, this->selector, this->params, rng, maxNbThreads);
}


bool Algorithm::Algorithm::isAgentEvalSkipped(
    std::shared_ptr<const Agent> agent,
    std::shared_ptr<Learn::EvaluationResult>& previousResult) const
{
    // Has the root already been evaluated more times than
    // params.maxNbEvaluationPerPolicy
    const auto& iter = this->selector->getResultsPerAgent().find(agent);
    if (iter != this->selector->getResultsPerAgent().end()) {
        // The root has already been evaluated
        previousResult = iter->second;
        return iter->second->getNbEvaluation() >=
               this->params.maxNbEvaluationPerPolicy;
    }
    else {
        previousResult = nullptr;
        return false;
    }
}

std::shared_ptr<Algorithm::ExecutionEngine> Algorithm::Algorithm::createExecutionEngine()
{
    // Create the execution engine
    auto executionEngine = std::make_shared<ExecutionEngine>(this->algorithmName);

    // Add the sub execution engine of the corresponding sub algorithms
    for(auto subAlgorithm: this->subAlgorithms){
        executionEngine->addSubExecutionEngine(subAlgorithm->createExecutionEngine());
    }

    return executionEngine;
}