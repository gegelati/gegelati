#include "evolution/steps/assignFitnessStep.h"

void Evolution::AssignFitnessStep::apply(EvolutionData& state)
{
    const FitnessAssignment::Assigner& usedAssigner = *state.get<FitnessAssignment::Assigner>(this->assigner);
    state.set(this->fitness, std::make_shared<std::map<uint64_t, double>>(
        usedAssigner.assignFitness(*state.get<std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>>(this->metrics))));
}
