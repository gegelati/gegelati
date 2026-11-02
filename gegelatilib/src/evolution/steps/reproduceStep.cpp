#include "evolution/steps/reproduceStep.h"


std::set<std::string> Evolution::ReproduceStep::requiredData() const
{
    return {this->parents};
}

std::set<std::string> Evolution::ReproduceStep::providedData() const
{
    return {this->offspring};
}

std::string Evolution::ReproduceStep::details() const
{
    return std::to_string(this->nbOffspring) + " from " + this->parents + " to " + this->offspring;
}

void Evolution::ReproduceStep::apply(EvolutionState& state)
{
    state.set(this->offspring, std::make_shared<std::vector<std::shared_ptr<Individual>>>(
        this->reproducer->reproduce(*state.get<std::vector<std::shared_ptr<const Individual>>>(this->parents), this->nbOffspring, state.rng)));
}
