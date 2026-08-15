#include "evolution/evolutionAlgorithm.h"


const Evolution::Population& Evolution::EvolutionAlgorithm::getPopulation()
{
    return *this->population;
}

const Evolution::Representation& Evolution::EvolutionAlgorithm::getRepresentation()
{
    return *this->representation;
}


const Evolution::Mutation& Evolution::EvolutionAlgorithm::getMutation()
{
    return *this->mutation;
}

const Learn::EvaluationAgent& Evolution::EvolutionAlgorithm::getEvaluation()
{
    return *this->evaluation;
}

const Selector::Selector& Evolution::EvolutionAlgorithm::getSelector()
{
    return *this->selection;
}

void Evolution::EvolutionAlgorithm::initializePopulation()
{
    std::unique_ptr<const Node::GenotypeTemplate> genotypeTemplate(std::move(this->representation->getGenotypeTemplate()));

    size_t nbIndividuals = 100;
    for(size_t idx = 0; idx < nbIndividuals; idx++) {
        Individual& indiv = this->population->getMutableIndividual(this->population->createIndividual());
        this->mutation->initRandomIndividual(indiv, *genotypeTemplate, this->rng);
    }
}

std::multimap<std::shared_ptr<Learn::EvaluationResult>,
    std::reference_wrapper<const Evolution::Individual>> Evolution::EvolutionAlgorithm::evaluatePopulation(
        size_t generationNumber, Learn::LearningMode mode
    )
{
    return this->evaluation->evaluateIndividuals(
        this->population->getIndividuals(),*this->representation, 
        *this->selection, generationNumber, mode);
}