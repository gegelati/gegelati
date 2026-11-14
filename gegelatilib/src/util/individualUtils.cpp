#include "util/individualUtils.h"

#include <cmath>
#include <queue>

std::shared_ptr<const Individual> Util::bestIndividual(const std::vector<std::shared_ptr<const Individual>>& individuals, const std::map<uint64_t, double>& fitness)
{
    std::shared_ptr<const Individual> best = nullptr;
    double bestFitness = 0.0;
    for(const std::shared_ptr<const Individual>& individual: individuals) {
        const auto found = fitness.find(individual->getIndividualID());
        if(found == fitness.end() || std::isnan(found->second)) {
            continue;
        }
        if(best == nullptr || found->second > bestFitness || (found->second == bestFitness && individual->getIndividualID() < best->getIndividualID())) {
            best = individual;
            bestFitness = found->second;
        }
    }
    return best;
}

std::unordered_map<uint64_t, std::shared_ptr<const Individual>> Util::makeIndex(const std::vector<std::shared_ptr<const Individual>>& individuals)
{
    std::unordered_map<uint64_t, std::shared_ptr<const Individual>> index;
    index.reserve(individuals.size());
    for(const std::shared_ptr<const Individual>& individual: individuals) {
        index.emplace(individual->getIndividualID(), individual);
    }
    return index;
}

std::map<uint64_t, std::shared_ptr<const Individual>> Util::makeOrderedIndex(const std::vector<std::shared_ptr<const Individual>>& individuals)
{
    std::map<uint64_t, std::shared_ptr<const Individual>> index;
    for(const std::shared_ptr<const Individual>& individual: individuals) {
        index.emplace(individual->getIndividualID(), individual);
    }
    return index;
}

std::vector<std::shared_ptr<const Individual>> Util::getMemberIndividuals(const Individual& individual)
{
    std::map<uint64_t, std::shared_ptr<const Individual>> members;
    const GraphBased::Genotype& genotype = individual.getGenotype();
    for(size_t idxGroup = 0; idxGroup < genotype.getSize(); idxGroup++) {
        addMemberIndividuals(genotype.getNodeGroup(idxGroup), members);
    }

    std::vector<std::shared_ptr<const Individual>> result;
    for(const std::pair<const uint64_t, std::shared_ptr<const Individual>>& member: members) {
        result.push_back(member.second);
    }
    return result;
}

std::vector<std::shared_ptr<const Individual>> Util::getMemberIndividuals(const Individual& individual, const Representations::Representation& representation)
{
    std::vector<std::shared_ptr<const Individual>> result;
    const std::vector<std::shared_ptr<const Individual>> members = getMemberIndividuals(individual);
    for(const std::shared_ptr<const Individual>& member: members) {
        if(&member->getRepresentation() == &representation) {
            result.push_back(member);
        }
    }
    return result;
}

void Util::addMemberIndividuals(const GraphBased::NodeGroup& group, std::map<uint64_t, std::shared_ptr<const Individual>>& members)
{
    for(size_t idxNode = 0; idxNode < group.getSize(); idxNode++) {
        addMemberIndividuals(group.getNode(idxNode), members);
    }
}

void Util::addMemberIndividuals(const GraphBased::GPNode& node, std::map<uint64_t, std::shared_ptr<const Individual>>& members)
{
    for(size_t idxValue = 0; idxValue < node.getSize(); idxValue++) {
        const Data::DataValue& value = node.getValue(idxValue);
        if(value.getRank() == 0 && value.getElementType() == typeid(std::shared_ptr<const Individual>)) {
            const std::shared_ptr<const Individual>& member = value.getScalar<std::shared_ptr<const Individual>>();
            members.emplace(member->getIndividualID(), member);
        }
    }
}

std::vector<std::shared_ptr<const Individual>> Util::getAllMemberIndividuals(const std::vector<std::shared_ptr<const Individual>>& individuals)
{
    // The individuals to search, then each member found the first time, until no new member is found.
    std::map<uint64_t, std::shared_ptr<const Individual>> members;
    std::queue<std::shared_ptr<const Individual>> toSearch;
    for(const std::shared_ptr<const Individual>& individual: individuals) {
        toSearch.push(individual);
    }
    while(!toSearch.empty()) {
        const std::vector<std::shared_ptr<const Individual>> currentMembers = getMemberIndividuals(*toSearch.front());
        toSearch.pop();
        for(const std::shared_ptr<const Individual>& member: currentMembers) {
            if(members.emplace(member->getIndividualID(), member).second) {
                toSearch.push(member);
            }
        }
    }

    std::vector<std::shared_ptr<const Individual>> result;
    for(const std::pair<const uint64_t, std::shared_ptr<const Individual>>& member: members) {
        result.push_back(member.second);
    }
    return result;
}
