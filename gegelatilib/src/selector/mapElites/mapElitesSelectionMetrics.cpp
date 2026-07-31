

#include "selector/mapElites/mapElitesSelectionMetrics.h"

const std::map<std::shared_ptr<const Selector::MapElites::MapElitesDescriptor>,
               std::vector<double>>&
Selector::MapElites::MapElitesSelectionMetrics::getMapDescriptors() const
{
    return mapDescriptors;
}

void Selector::MapElites::MapElitesSelectionMetrics::initMetrics(
    const Representation::Individual& agent,
    const Learn::LearningEnvironment& learningEnvironment)
{
    for (auto& pair : mapDescriptors) {
        pair.second.resize(pair.first->getNbDescriptors(), 0.0);
    }
}

void Selector::MapElites::MapElitesSelectionMetrics::extractMetricsStep(
    const Representation::Individual& agent, std::vector<double> actionValues,
    const Learn::LearningEnvironment& learningEnvironment)
{
    for (auto& pair : mapDescriptors) {
        pair.first->extractMetricsStep(pair.second, agent, actionValues,
                                       learningEnvironment);
    }
}

void Selector::MapElites::MapElitesSelectionMetrics::extractMetricsEpisode(
    const Representation::Individual& agent, size_t nbStepsExecuted,
    const Learn::LearningEnvironment& learningEnvironment)
{
    SelectionMetrics::extractMetricsEpisode(agent, nbStepsExecuted,
                                            learningEnvironment);

    for (auto& pair : mapDescriptors) {
        pair.first->extractMetricsEpisode(pair.second, agent, nbStepsExecuted,
                                          learningEnvironment);
    }
}

void Selector::MapElites::MapElitesSelectionMetrics::weightedSum(
    std::shared_ptr<SelectionMetrics> other, size_t nbEvaluation,
    size_t nbEvaluationOther)
{

    // To update main score and utility, and check type issues
    SelectionMetrics::weightedSum(other, nbEvaluation, nbEvaluationOther);

    const auto& castedOther =
        static_cast<const MapElitesSelectionMetrics&>(*other);
    auto it = this->mapDescriptors.begin();
    auto itOther = castedOther.mapDescriptors.begin();

    while (it != this->mapDescriptors.end() &&
           itOther != castedOther.mapDescriptors.end()) {
        if (it->second.size() != itOther->second.size()) {
            throw std::runtime_error("Number scores is not the same.");
        }
        for (size_t idx = 0; idx < it->second.size(); idx++) {
            // Weighted sum of the score per class.
            it->second[idx] = it->second[idx] * (double)nbEvaluation +
                              itOther->second[idx] * (double)nbEvaluationOther;
            it->second[idx] /= (double)(nbEvaluation + nbEvaluationOther);
            
           
        }
        it++;
        itOther++;
    }
}
