
#include "algorithm/algorithm.h"

void Algorithm::Algorithm::init(RNG::RNG& rng)
{
    this->mutator->initRandomPopulation(this->graph, this->params, rng);

    this->mutator->mutatePopulation(this->graph, this->manager, this->selector, this->archive, this->params, rng);

    // Clear the best agent in the selector
    this->selector->forgetPreviousResults();

    // Clear the archive
    this->archive.clear();
}

void Algorithm::Algorithm::populate(RNG::RNG& rng, size_t maxNbThreads)
{
    this->mutator->mutatePopulation(this->graph, this->manager, this->selector, this->archive, this->params, rng, maxNbThreads);
}


bool Algorithm::Algorithm::isAgentEvalSkipped(
    const Agent& agent,
    std::shared_ptr<Learn::EvaluationResult>& previousResult) const
{
    // Has the root already been evaluated more times than
    // params.maxNbEvaluationPerPolicy
    const auto& iter = this->selector->getResultsPerRoot().find(&agent);
    if (iter != this->selector->getResultsPerRoot().end()) {
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