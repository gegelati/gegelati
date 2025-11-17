
#include "algorithm/algorithm.h"

void Algorithm::Algorithm::init(Mutator::RNG& rng)
{
    this->mutator->initPopulation(rng);
}