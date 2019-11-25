#include <gtest/gtest.h>

#include "deterministicRandom.h"

#include <iostream>
#include <random>

int main(int argc, char **argv) {

	std::mt19937_64 engine(0);

	Mutator::uniform_int_distribution<uint64_t> distribution(0, 100);

	for (auto i = 0; i < 100; i++) {
		std::cout << distribution(engine) << " " << std::endl;
	}

	Mutator::uniform_int_distribution<uint64_t> distribution2(0, UINT64_MAX);
	for (auto i = 0; i < 100; i++) {
		std::cout << distribution2(engine) << " " << std::endl;
	}
	

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
