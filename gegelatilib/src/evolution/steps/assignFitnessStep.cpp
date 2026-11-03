#include "evolution/steps/assignFitnessStep.h"


std::set<std::string> Evolution::AssignFitnessStep::requiredData() const
{
    return {this->metrics.name()};
}

std::set<std::string> Evolution::AssignFitnessStep::providedData() const
{
    return {this->fitness.name()};
}

std::string Evolution::AssignFitnessStep::details() const
{
    return this->metrics.name() + " to " + this->fitness.name();
}

void Evolution::AssignFitnessStep::apply(EvolutionData& state)
{
    state.set(this->fitness, std::make_shared<std::map<uint64_t, double>>(this->assigner->assignFitness(*state.get(this->metrics), this->metricHash)));
}
