
#include "selector/selectionMetrics.h"

double Selector::SelectionMetrics::getScore() const
{
    return score;
}


double Selector::SelectionMetrics::getUtility() const
{
    return utility;
}

void Selector::SelectionMetrics::extractMetricsStep(const TPG::TPGVertex* agent, const Learn::LearningEnvironment& learningEnvironment)
{
    // Does not do anything
}


void Selector::SelectionMetrics::extractMetricsEpisode(const TPG::TPGVertex* agent, const Learn::LearningEnvironment& learningEnvironment)
{
    // Update score
    this->score += learningEnvironment.getScore();
    // Update utility if used.
    if (learningEnvironment.isUsingUtility()) {
        utility += learningEnvironment.getUtility();
    }
}

Selector::SelectionMetrics& Selector::SelectionMetrics::operator+=(
    const Selector::SelectionMetrics& other)
{
    // Type Check (Must be done in all override)
    // This test will succeed in child class.
    const std::type_info& thisType = typeid(*this);
    if (typeid(other) != thisType) {
        throw std::runtime_error("Type mismatch between SelectionMetrics.");
    }

    // If the added type is Selector::SelectionMetrics
    if (thisType == typeid(Selector::SelectionMetrics)) {
        this->score += other.score;
        this->utility += other.utility;
    }

    return *this;
}

Selector::SelectionMetrics& Selector::SelectionMetrics::operator*=(double factor)
{
    this->score *= factor;
    this->utility *= factor;
    return *this;
}

Selector::SelectionMetrics& Selector::SelectionMetrics::operator/=(double factor)
{
    return this->operator*=(1.0 / factor);
}


bool Selector::operator<(std::shared_ptr<Selector::SelectionMetrics> a, std::shared_ptr<Selector::SelectionMetrics> b)
{
    return a->getScore() < b->getScore();
}