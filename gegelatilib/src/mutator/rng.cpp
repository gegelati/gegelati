#include "mutator/rng.h"
#include "mutator/deterministicRandom.h"

void Mutator::RNG::setSeed(uint64_t seed) {
	engine.seed(seed);
}

uint64_t Mutator::RNG::getUnsignedInt64(uint64_t min, uint64_t max)
{
	Mutator::uniform_int_distribution<uint64_t> distribution(min, max);
	return distribution(engine);
}

double Mutator::RNG::getDouble(double min, double max)
{
	std::uniform_real_distribution<double> distribution(min, max);
	return distribution(engine);
}