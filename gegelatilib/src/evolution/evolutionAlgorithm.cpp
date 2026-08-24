#include "evolution/evolutionAlgorithm.h"


Evolution::Population& Evolution::EvolutionAlgorithm::getPopulation()
{
    return *this->population;
}

Evolution::Representation& Evolution::EvolutionAlgorithm::getRepresentation()
{
    return *this->representation;
}


Evolution::Mutation& Evolution::EvolutionAlgorithm::getMutation()
{
    return *this->mutation;
}

Evaluation::EvaluationAgent& Evolution::EvolutionAlgorithm::getEvaluation()
{
    return this->evaluation;
}

Evolution::SurvivingSelection& Evolution::EvolutionAlgorithm::getSelector()
{
    return *this->survivingSelection;
}

RNG::RNG& Evolution::EvolutionAlgorithm::getRNG()
{
    return this->rng;
}

void Evolution::EvolutionAlgorithm::initializePopulation()
{
    if(representation->isTangled()) {
        representation->setTangledPopulation(*population);
    }

    std::vector<std::unique_ptr<Evaluation::EvaluationMetric>> selectionMetrics = this->survivingSelection->getSelectionMetrics();
    for(const std::unique_ptr<Evaluation::EvaluationMetric>& metric: selectionMetrics) {
        this->evaluation.addRequestedMetric(*metric);
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
    std::set<std::reference_wrapper<const Evolution::Individual>> currentIndividuals(this->population->getIndividuals());

    std::vector<std::reference_wrapper<const Evolution::Individual>> selectedParents;
    for(size_t idx = 0; idx < nbParents; idx ++) {
        // Random parent selection for now
        auto it = currentIndividuals.begin();
        std::advance(it, rng.getUnsignedInt64(0, currentIndividuals.size() - 1));
        selectedParents.push_back(*it);
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

void Evolution::EvolutionAlgorithm::evaluatePopulation(
    const std::set<std::unique_ptr<Individual>, UniqueLess<Individual>>& offspring, 
    size_t generationNumber, Learn::LearningMode mode
    )
{
    std::set<std::reference_wrapper<const Evolution::Individual>> evaluatedIndividuals = this->population->getNotProtectedIndividuals();
    for (const std::unique_ptr<Individual>& os: offspring) {
        evaluatedIndividuals.insert(*os);
    }

    this->evaluation.evaluateIndividuals(
        evaluatedIndividuals, *this->representation, generationNumber, mode);
}

void Evolution::EvolutionAlgorithm::selectSurvivors(
    std::set<std::unique_ptr <Individual>, UniqueLess<Individual>>& offspring)
{

    std::set<std::reference_wrapper<const Evolution::Individual>> evaluatedIndividuals = this->population->getNotProtectedIndividuals();
    for (const std::unique_ptr<Individual>& os: offspring) {
        evaluatedIndividuals.insert(*os);
    }
    std::map<std::reference_wrapper<const Evolution::Individual>, bool> selectionResult = this->survivingSelection->select(evaluatedIndividuals);


    for (auto it = selectionResult.begin(); it != selectionResult.end();) {
        const Individual& indiv = it->first.get();

        if (it->second) {
            // Survivor: if offspring, add to the population, else do nothing
            auto offIt = offspring.find(&indiv);
            if (offIt != offspring.end()) {
                this->population->addIndividual(std::move(offspring.extract(offIt).value()));
            }

        } else {
            // Loser: if offspring, delete from offspring list, else delete from population
            if (this->population->containsIndividual(indiv)) {
                this->population->deleteIndividual(indiv);
            } else {
                offspring.erase(offspring.find(&indiv));
            }
        }

        it = selectionResult.erase(it);   // clean the whole map as we go
    }
}