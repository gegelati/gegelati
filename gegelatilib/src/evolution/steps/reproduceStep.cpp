#include "evolution/steps/reproduceStep.h"


std::set<std::string> Evolution::ReproduceStep::requiredData() const
{
    return {this->parents.name()};
}

std::set<std::string> Evolution::ReproduceStep::providedData() const
{
    return {this->offspring.name()};
}

std::string Evolution::ReproduceStep::details() const
{
    return std::to_string(this->nbOffspring) + " from " + this->parents.name() + " to " + this->offspring.name();
}

void Evolution::ReproduceStep::apply(EvolutionData& state)
{
    state.set(this->offspring, std::make_shared<std::vector<std::shared_ptr<Individual>>>(
        this->reproducer->reproduce(state.getIndividuals(this->parents), this->nbOffspring, state.rng)));
}
