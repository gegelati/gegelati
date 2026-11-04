#ifndef UTIL_INDIVIDUAL_UTILS_H
#define UTIL_INDIVIDUAL_UTILS_H

#include <cstdint>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

#include "individual.h"

namespace Util {

    /**
     * \brief Return the best individual of a list according to a fitness.
     *
     * The best individual has the highest fitness, then the lowest ID. Individuals without fitness are ignored.
     *
     * \param[in] individuals the individuals ranked.
     * \param[in] fitness the fitness of the individuals, by ID.
     * \return the best individual, or nullptr if no individual has a fitness.
     */
    std::shared_ptr<const Individual> bestIndividual(const std::vector<std::shared_ptr<const Individual>>& individuals, const std::map<uint64_t, double>& fitness);

    /**
     * \brief Create an index of individuals by their ID.
     *
     * \param[in] individuals the indexed individuals.
     */
    std::unordered_map<uint64_t, std::shared_ptr<const Individual>> makeIndex(const std::vector<std::shared_ptr<const Individual>>& individuals);

}; // namespace Util

#endif // UTIL_INDIVIDUAL_UTILS_H
