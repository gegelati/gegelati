#include "evolution/steps/reproduceStep.h"


std::string Evolution::ReproduceStep::details() const
{
    return std::to_string(this->nbOffspring) + " from " + this->parents.name() + " to " + this->offspring.name();
}

void Evolution::ReproduceStep::apply(EvolutionData& state)
{
    const std::vector<std::shared_ptr<Individual>>& parents = *state.get(this->parents);
    state.set(this->offspring, std::make_shared<std::vector<std::shared_ptr<Individual>>>(
        this->reproducer->reproduce(std::vector<std::shared_ptr<const Individual>>(parents.begin(), parents.end()), this->nbOffspring, state.rng)));
}
