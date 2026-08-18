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
        Individual& indiv = this->population->getMutableIndividual(this->population->addIndividual());
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

std::set<std::unique_ptr<Evolution::Individual>, UniqueLess<Evolution::Individual>> Evolution::EvolutionAlgorithm::reproduceParents(
    std::vector<std::reference_wrapper<const Individual>> parents
)
{    // Reproduction process, only replication for now.
    std::set<std::unique_ptr<Individual>, UniqueLess<Individual>> offspring;
    for(size_t idx = 0; idx < parents.size(); idx++) {
        offspring.insert(std::move(parents.at(idx).get().cloneUniquePtr()));
    }
    return offspring;
}

void Evolution::EvolutionAlgorithm::mutateOffspring(const std::set<std::unique_ptr<Individual>, UniqueLess<Individual>>& offspring)
{
    std::unique_ptr<const Node::GenotypeTemplate> genotypeTemplate(std::move(this->representation->getGenotypeTemplate()));
    for(const std::unique_ptr<Individual>& indiv: offspring) {
        this->mutation->mutateGenotype(indiv->getMutableGenotype(), *genotypeTemplate, rng);
    }
}

std::multimap<std::shared_ptr<Learn::EvaluationResult>,
    std::reference_wrapper<const Evolution::Individual>> Evolution::EvolutionAlgorithm::evaluatePopulation(
        const std::set<std::unique_ptr<Individual>, UniqueLess<Individual>>& offspring, size_t generationNumber, Learn::LearningMode mode
    )
{
    std::vector<std::reference_wrapper<const Evolution::Individual>> evaluatedIndividuals = this->population->getNotProtectedIndividuals();
    for (const std::unique_ptr<Individual>& os: offspring) {
        evaluatedIndividuals.push_back(*os);
    }

    return this->evaluation->evaluateIndividuals(
        evaluatedIndividuals,*this->representation, 
        *this->selection, generationNumber, mode);
}

std::vector<std::reference_wrapper<const Evolution::Individual>> Evolution::EvolutionAlgorithm::selectSurvivors(std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                        std::reference_wrapper<const Individual>>& scores)
{
    std::vector<std::reference_wrapper<const Evolution::Individual>> loosers;

    // Standard (mu+lambda) replacement
    size_t mu = 100;
    while(scores.size() > mu) {
        auto it = scores.begin();
        loosers.push_back(it->second);
        scores.erase(it);
    }
    return loosers;
}

void Evolution::EvolutionAlgorithm::replacePopulation(
    std::set<std::unique_ptr<Individual>, UniqueLess<Individual>>& offspring,
    std::vector<std::reference_wrapper<const Evolution::Individual>> loosers)
{
    // Discard loosers! Bouuuuh
    while(loosers.size() > 0) {
        auto it = loosers.begin();
        const Individual& loser = *it;

        if(this->population->containsIndividual(loser)) {
            bool deleted = this->population->deleteIndividual(loser);
            if(!deleted) {
                throw std::runtime_error("Evolution::EvolutionAlgorithm::replacePopulation: deleting indivudal failed, in current version it means a protected individual has been evaluated, which should not happend.");
            }
        } else {
            offspring.erase(offspring.find(&loser));
        }
        loosers.erase(it);
    }

    // Add surviving offspring to the population
    while (!offspring.empty()) {
        this->population->addIndividual(std::move(offspring.extract(offspring.begin()).value()));
    }
}