#ifndef EVOLUTION_TEST_HELPERS_H
#define EVOLUTION_TEST_HELPERS_H

#include <memory>
#include <vector>

#include "individual.h"

/// Return the individuals of a list as individuals that cannot be modified (what the algorithms of the library take).
inline std::vector<std::shared_ptr<const Individual>> constIndividuals(const std::vector<std::shared_ptr<Individual>>& individuals)
{
    return std::vector<std::shared_ptr<const Individual>>(individuals.begin(), individuals.end());
}

#endif // EVOLUTION_TEST_HELPERS_H
