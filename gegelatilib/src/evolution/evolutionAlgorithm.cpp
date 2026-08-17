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
    if(representation->isTangled()) {
        representation->setTangledPopulation(*population);
    }

    std::unique_ptr<const Node::GenotypeTemplate> genotypeTemplate(std::move(this->representation->getGenotypeTemplate()));

    size_t nbIndividuals = 100;
    for(size_t idx = 0; idx < nbIndividuals; idx++) {
        Individual& indiv = this->population->getMutableIndividual(this->population->createIndividual());
        this->mutation->initRandomGenotype(indiv.getMutableGenotype(), *genotypeTemplate, this->rng);
    }
}

std::vector<std::reference_wrapper<const Evolution::Individual>> Evolution::EvolutionAlgorithm::selectParents(size_t nbParents)
{
    std::vector<std::reference_wrapper<const Evolution::Individual>> currentIndividuals(this->population->getIndividuals());

    std::vector<std::reference_wrapper<const Evolution::Individual>> selectedParents;
    for(size_t idx = 0; idx < nbParents; idx ++) {
        // Random parent selection for now
        selectedParents.push_back(currentIndividuals.at(rng.getUnsignedInt64(0, currentIndividuals.size() - 1)));
    }
    return selectedParents;
}

std::vector<std::reference_wrapper<const Evolution::Individual>> Evolution::EvolutionAlgorithm::reproduceParents(
    std::vector<std::reference_wrapper<const Individual>> parents
)
{    // Reproduction process, only replication for now.
    std::vector<std::reference_wrapper<const Evolution::Individual>> offspring;
    for(size_t idx = 0; idx < parents.size(); idx++) {
        offspring.push_back(this->population->copyIndividual(parents.at(idx)));
    }
    return offspring;
}

void Evolution::EvolutionAlgorithm::mutateOffspring(std::vector<std::reference_wrapper<const Individual>> offspring)
{
    std::unique_ptr<const Node::GenotypeTemplate> genotypeTemplate(std::move(this->representation->getGenotypeTemplate()));
    for(const Individual& constIndiv: offspring) {
        Individual& indiv = this->population->getMutableIndividual(constIndiv);
        this->mutation->mutateGenotype(indiv.getMutableGenotype(), *genotypeTemplate, rng);
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


void Evolution::EvolutionAlgorithm::replacePopulation(std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                              std::reference_wrapper<const Individual>>& scores)
{
    // Verify that all scores correspond to existing individuals.
    for(const auto& score: scores) {
        if(!this->population->containsIndividual(score.second)){
            throw std::runtime_error("Evolution::EvolutionAlgorithm::replace: scores should all correspond to individual of the population");
        }
    }

    // Standard (mu+lambda) replacement
    size_t mu = 100;
    while(this->population->size() > mu) {
        auto it = scores.begin();
        this->population->deleteIndividual(it->second);
        scores.erase(it);
    }
}