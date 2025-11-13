

#include "selector/mapElites/mapElitesDefaultDescriptors.h"

void Selector::MapElites::DefaultDescriptors::ActionValues::initDescriptor(
    const TPG::TPGGraph& graph,
    const Learn::LearningEnvironment& learningEnvironment)
{
    this->init = true;
    this->minRange = 0.0;
    this->maxRange = 1.0;
    this->nbDescriptors = learningEnvironment.getNbActions();
}

void Selector::MapElites::DefaultDescriptors::ActionValues::extractMetricsStep(
    std::vector<double>& metrics, const TPG::TPGVertex* agent,
    std::vector<double> actionValues,
    const Learn::LearningEnvironment& learningEnvironment) const
{
    for (size_t idx = 0; idx < nbDescriptors; idx++) {
        metrics[idx] += std::abs(actionValues[idx]);
    }
}

void Selector::MapElites::DefaultDescriptors::ActionValues::
    extractMetricsEpisode(
        std::vector<double>& metrics, const TPG::TPGVertex* agent,
        size_t nbStepsExecuted,
        const Learn::LearningEnvironment& learningEnvironment) const
{
    for (size_t idx = 0; idx < nbDescriptors; idx++) {
        metrics[idx] /= nbStepsExecuted;
    }
}