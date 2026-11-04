#include "evolution/steps/lambdaStep.h"

void Evolution::LambdaStep::apply(EvolutionData& state)
{
    this->function(state);
}
