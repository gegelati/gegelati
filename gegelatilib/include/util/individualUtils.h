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

    /**
     * \brief Create an index of individuals by their ID, ordered by ID.
     *
     * \param[in] individuals the indexed individuals, an individual appearing several times being indexed once.
     */
    std::map<uint64_t, std::shared_ptr<const Individual>> makeOrderedIndex(const std::vector<std::shared_ptr<const Individual>>& individuals);

    /**
     * \brief Return the individuals referenced by the values of the genotype of an individual (e.g. the members and the destinations of a TPG vertex).
     *
     * \param[in] individual the individual.
     * \return the referenced individuals, ordered by ID and without duplicates.
     */
    std::vector<std::shared_ptr<const Individual>> getMemberIndividuals(const Individual& individual);

    /**
     * \brief Return the individuals of a representation referenced by the values of the genotype of an individual.
     *
     * \param[in] individual the individual.
     * \param[in] representation the representation of the returned individuals (e.g. the TPG, for the destinations of a TPG vertex).
     * \return the referenced individuals of the representation, ordered by ID and without duplicates.
     */
    std::vector<std::shared_ptr<const Individual>> getMemberIndividuals(const Individual& individual, const Representations::Representation& representation);

    /**
     * \brief Return the individuals referenced by some individuals, directly or through other referenced individuals (e.g. the members of
     * the teams of a population, and the members of these members).
     *
     * \param[in] individuals the individuals whose members are searched.
     * \return the referenced individuals, ordered by ID and without duplicates. An individual of the list is returned only if another one references it.
     */
    std::vector<std::shared_ptr<const Individual>> getAllMemberIndividuals(const std::vector<std::shared_ptr<const Individual>>& individuals);

    /**
     * \brief Add the individuals referenced by the values of the nodes of a node group.
     *
     * \param[in] group the node group.
     * \param[in,out] members the referenced individuals, by ID.
     */
    void addMemberIndividuals(const GraphBased::NodeGroup& group, std::map<uint64_t, std::shared_ptr<const Individual>>& members);

    /**
     * \brief Add the individuals referenced by the values of a node.
     *
     * \param[in] node the node.
     * \param[in,out] members the referenced individuals, by ID.
     */
    void addMemberIndividuals(const GraphBased::GPNode& node, std::map<uint64_t, std::shared_ptr<const Individual>>& members);

}; // namespace Util

#endif // UTIL_INDIVIDUAL_UTILS_H
