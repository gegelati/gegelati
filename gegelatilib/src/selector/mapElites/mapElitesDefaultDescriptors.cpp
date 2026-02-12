

#include "selector/mapElites/mapElitesDefaultDescriptors.h"

void Selector::MapElites::DefaultDescriptors::ActionValues::initDescriptor(
    const EvoGraph::Graph& graph,
    const Learn::LearningEnvironment& learningEnvironment)
{
    this->init = true;
    this->minRange = 0.0;
    this->maxRange = 1.0;
    this->nbDescriptors = learningEnvironment.getActions()->size();
}

std::string Selector::MapElites::DefaultDescriptors::ActionValues::getName() const
{
    return "ActionValues";
}

void Selector::MapElites::DefaultDescriptors::ActionValues::extractMetricsStep(
    std::vector<double>& metrics, const Algorithm::Agent& agent,
    std::vector<double> actionValues,
    const Learn::LearningEnvironment& learningEnvironment) const
{
    for (size_t idx = 0; idx < nbDescriptors; idx++) {
        metrics[idx] += std::abs(actionValues[idx]);
    }
}

void Selector::MapElites::DefaultDescriptors::ActionValues::
    extractMetricsEpisode(
        std::vector<double>& metrics, const Algorithm::Agent& agent,
        size_t nbStepsExecuted,
        const Learn::LearningEnvironment& learningEnvironment) const
{
    for (size_t idx = 0; idx < nbDescriptors; idx++) {
        metrics[idx] /= nbStepsExecuted;
    }
}