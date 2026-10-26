

#include "selection/tournamentSelector.h"

std::vector<std::shared_ptr<const Individual>> Selection::TournamentSelector::select(
                const std::map<std::shared_ptr<const Individual>, double, SharedLess<Individual>>& fitnessMap,
                size_t nbSelected, RNG::RNG& rng) const
{
    std::vector<std::pair<std::shared_ptr<const Individual>, double>> individualFitnesses = getOrderedFitness(fitnessMap, false);

    if(this->sizeTournament < 2) {
        throw std::runtime_error("Selection::TournamentSelector::select: Tournament size should be higher than 1");
    }

    // Selected individuals
    std::vector<std::shared_ptr<const Individual>> selected;
    
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

        std::shared_ptr<const Individual> bestIndiv = individualFitnesses.at(bestIndex).first;
        /// Select best
        selected.push_back(bestIndiv);

        if(!this->replacement) {
            individualFitnesses.erase(individualFitnesses.begin() + bestIndex);
        }
    }

    return selected;
}