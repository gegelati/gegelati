#include "mutator/rng.h"

/// Construct the engine
std::mt19937_64 Mutator::RNG::engine;

void Mutator::RNG::setSeed(uint64_t seed) {
	engine.seed(seed);
}

uint64_t Mutator::RNG::getInt(uint64_t min, uint64_t max)
{
	std::uniform_int_distribution<uint64_t> distribution(min, max);
	return distribution(engine);
}
