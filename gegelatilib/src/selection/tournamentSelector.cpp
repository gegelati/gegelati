

#include "selection/tournamentSelector.h"

std::vector<uint64_t> Selection::TournamentSelector::select(
                const std::map<uint64_t, double>& fitnessMap,
                size_t nbSelected, RNG::RNG& rng) const
{
    std::vector<std::pair<uint64_t, double>> individualFitnesses = getOrderedFitness(fitnessMap, false);

    if(this->sizeTournament < 2) {
        throw std::runtime_error("Selection::TournamentSelector::select: Tournament size should be higher than 1");
    }

    // Selected individuals
    std::vector<uint64_t> selected;
    
    for(size_t idx = 0; idx < nbSelected; idx++) {

        if(this->sizeTournament > individualFitnesses.size()) {
            throw std::runtime_error("Selection::TournamentSelector::select: size of tournament: " + std::to_string(this->sizeTournament) + " is too large for the current set of individual: " + std::to_string(individualFitnesses.size()));
        }

        // Create tournament
        std::set<size_t> indices;
        while(indices.size() < this->sizeTournament) {
            indices.insert(rng.uniformSample<size_t>(0, individualFitnesses.size() - 1));
        }

        // Find best individual in the tournament
        // In practice it is the first one thanks to the getOrderedFitness
        size_t bestIndex = *indices.begin();

        uint64_t bestIndiv = individualFitnesses.at(bestIndex).first;
        /// Select best
        selected.push_back(bestIndiv);

        if(!this->replacement) {
            individualFitnesses.erase(individualFitnesses.begin() + bestIndex);
        }
    }

    return selected;
}