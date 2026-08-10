#include "evolution/evolutionAlgorithm.h"

void Evolution::EvolutionAlgorithm::initializedPopulation()
{
    //structure = this->representation->getNodesStructure;

    size_t nbIndividuals = 100;
    for(size_t idx = 0; idx < nbIndividuals; idx++) {
        Individual& indiv = this->population->getMutableIndividual(this->population->createIndividual());
        // mutator.initializeIndividual(indiv, structure, this->rng);
    }
}

