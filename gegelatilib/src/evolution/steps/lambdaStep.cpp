#include "evolution/steps/lambdaStep.h"

std::set<std::string> Evolution::LambdaStep::requiredData() const
{
    return this->required;
}

std::set<std::string> Evolution::LambdaStep::providedData() const
{
    return this->provided;
}

void Evolution::LambdaStep::apply(EvolutionState& state)
{
    this->function(state);
}
