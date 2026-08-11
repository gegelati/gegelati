#include "evolution/evolutionAlgorithm.h"

void Evolution::EvolutionAlgorithm::initializePopulation()
{
    //structure = this->representation->getNodesStructure;

    size_t nbIndividuals = 100;
    for(size_t idx = 0; idx < nbIndividuals; idx++) {
        Individual& indiv = this->population->getMutableIndividual(this->population->createIndividual());
        this->mutation->initRandomIndividual(indiv, this->rng);
    }
}


const Evolution::Population& Evolution::EvolutionAlgorithm::getPopulation()
{
    return *this->population;
}

const Evolution::Representation& Evolution::EvolutionAlgorithm::getRepresentation()
{
    return *this->representation;
}