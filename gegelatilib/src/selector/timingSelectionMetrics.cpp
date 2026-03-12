#include "selector/timingSelectionMetrics.h"

void Selector::TimingSelectionMetrics::extractMetricsStep(
    const TPG::TPGVertex* agent, std::vector<double> actionValues,
    const Learn::LearningEnvironment& learningEnvironment)
{
    wrapped->extractMetricsStep(agent, actionValues, learningEnvironment);
};

void Selector::TimingSelectionMetrics::extractMetricsEpisodeWithTiming(
    const TPG::TPGVertex* agent, size_t nbStepsExecuted,
    const Learn::LearningEnvironment& learningEnvironment, double agentTimeEpisode,
    double leTimeEpisode)
{
    wrapped->extractMetricsEpisode(agent, nbStepsExecuted, learningEnvironment);

    nbActions += nbStepsExecuted;
    agentTime += agentTimeEpisode;
    leTime += leTimeEpisode;
}

void Selector::TimingSelectionMetrics::weightedSum(
    std::shared_ptr<SelectionMetrics> other, size_t nbEvaluation,
    size_t nbEvaluationOther)
{
    if (auto tother = dynamic_cast<TimingSelectionMetrics*>(other.get())) {
        wrapped->weightedSum(tother->wrapped, nbEvaluation, nbEvaluationOther);

        agentTime = SelectionMetrics::weightedSum(
            agentTime, tother->agentTime, nbEvaluation, nbEvaluationOther);
        leTime = SelectionMetrics::weightedSum(leTime, tother->leTime,
                                               nbEvaluation, nbEvaluationOther);
        nbActions = SelectionMetrics::weightedSum(
            nbActions, tother->nbActions, nbEvaluation, nbEvaluationOther);
    }
    else {
        throw std::runtime_error("Type mismatch between SelectionMetrics.");
    }
}

bool Selector::operator<(std::shared_ptr<TimingSelectionMetrics> a,
                         std::shared_ptr<TimingSelectionMetrics> b)
{
    return *a < *b;
}

bool Selector::operator<(std::shared_ptr<SelectionMetrics> a,
                         std::shared_ptr<TimingSelectionMetrics> b)
{
    return *a < *b;
}

bool Selector::operator<(std::shared_ptr<TimingSelectionMetrics> a,
                         std::shared_ptr<SelectionMetrics> b)
{
    return *a < *b;
}
