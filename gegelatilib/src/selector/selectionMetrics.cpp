
#include "selector/selectionMetrics.h"

double Selector::SelectionMetrics::getScore() const
{
    return score;
}

double Selector::SelectionMetrics::getUtility() const
{
    return utility;
}

void Selector::SelectionMetrics::extractMetricsEpisode(
    const TPG::TPGVertex* agent, size_t nbStepsExecuted,
    const Learn::LearningEnvironment& learningEnvironment)
{
    // Update score
    this->score += learningEnvironment.getScore();
    // Update utility if used.
    if (learningEnvironment.isUsingUtility()) {
        utility += learningEnvironment.getUtility();
    }
}

void Selector::SelectionMetrics::weightedSum(
    std::shared_ptr<SelectionMetrics> other, size_t nbEvaluation,
    size_t nbEvaluationOther)
{

    if (typeid(*this) != typeid(*other)) {
        throw std::runtime_error("Type mismatch between SelectionMetrics.");
    }

    this->score = this->score * (double)nbEvaluation +
                  other->score * (double)nbEvaluationOther;
    this->score /= (double)(nbEvaluation + nbEvaluationOther);

    this->utility = this->score * (double)nbEvaluation +
                    other->utility * (double)nbEvaluationOther;
    this->utility /= (double)(nbEvaluation + nbEvaluationOther);
}

Selector::SelectionMetrics& Selector::SelectionMetrics::operator/=(
    double factor)
{
    this->score /= factor;
    this->utility /= factor;
    return *this;
}

bool Selector::operator<(std::shared_ptr<SelectionMetrics> a,
                         std::shared_ptr<SelectionMetrics> b)
{
    return a->getScore() < b->getScore();
}