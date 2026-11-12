#include "evolution/steps/assignFitnessStep.h"


std::string Evolution::AssignFitnessStep::details() const
{
    return this->assigner->getName() + ", " + this->metrics + " to " + this->fitness;
}

void Evolution::AssignFitnessStep::apply(EvolutionData& state)
{
    state.set(this->fitness, std::make_shared<std::map<uint64_t, double>>(this->assigner->assignFitness(*state.get<std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>>(this->metrics), this->metricHash)));
}
