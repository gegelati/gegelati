#include "evolution/steps/assignFitnessStep.h"


std::set<std::string> Evolution::AssignFitnessStep::requiredData() const
{
    return {this->metrics};
}

std::set<std::string> Evolution::AssignFitnessStep::providedData() const
{
    return {this->fitness};
}

std::string Evolution::AssignFitnessStep::details() const
{
    return this->metrics + " to " + this->fitness;
}

void Evolution::AssignFitnessStep::apply(EvolutionState& state)
{
    state.set(this->fitness, std::make_shared<std::map<uint64_t, double>>(this->assigner->assignFitness(*state.get<std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>>(this->metrics), this->metricHash)));
}
