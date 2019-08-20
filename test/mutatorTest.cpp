#include <gtest/gtest.h>

#include "mutator/rng.h"

TEST(MutatorTest, RNG){
	Mutator::RNG::setSeed(0);

	// With this seed, the current pseudo-random number generator returns 24 
	// on its first use
	ASSERT_EQ(Mutator::RNG::getInt(0, 100), 24) << "Returned pseudo-random value changed with a known seed.";
}