#include "evolutionAlgorithm.h"


Population& EvolutionAlgorithm::getPopulation()
{
    return *this->population;
}

const Representations::Representation& EvolutionAlgorithm::getRepresentation() const
{
    return this->representation;
}


Mutation::Mutator& EvolutionAlgorithm::getMutation()
{
    return *this->mutation;
}

Evaluation::EvaluationAgent& EvolutionAlgorithm::getEvaluation()
{
    return this->evaluation;
}

Selection::Selector& EvolutionAlgorithm::getSelector()
{
    return *this->survivingSelection;
}

RNG::RNG& EvolutionAlgorithm::getRNG()
{
    return this->rng;
}

void EvolutionAlgorithm::initializePopulation()
{
    std::vector<std::unique_ptr<Evaluation::EvaluationMetric>> selectionMetrics = this->survivingSelection->getSelectionMetrics();
    for(const std::unique_ptr<Evaluation::EvaluationMetric>& metric: selectionMetrics) {
        this->evaluation.addRequestedMetric(*metric);
    }


}

std::vector<std::reference_wrapper<const Individual>> EvolutionAlgorithm::selectParents(size_t nbParents)
{
    std::set<std::reference_wrapper<const Individual>> currentIndividuals(this->population->getIndividuals());

    std::vector<std::reference_wrapper<const Individual>> selectedParents;
    for(size_t idx = 0; idx < nbParents; idx ++) {
        // Random parent selection for now
        auto it = currentIndividuals.begin();
        std::advance(it, rng.getUnsignedInt64(0, currentIndividuals.size() - 1));
        selectedParents.push_back(*it);
    }
    return selectedParents;
}

std::set<std::unique_ptr<Individual>, UniqueLess<Individual>> EvolutionAlgorithm::reproduceParents(
    std::vector<std::reference_wrapper<const Individual>> parents
)
{    // Reproduction process, only replication for now.
    std::set<std::unique_ptr<Individual>, UniqueLess<Individual>> offspring;
    for(size_t idx = 0; idx < parents.size(); idx++) {
        offspring.insert(std::move(parents.at(idx).get().cloneUniquePtr()));
    }
    return offspring;
}

void EvolutionAlgorithm::mutateOffspring(const std::set<std::unique_ptr<Individual>, UniqueLess<Individual>>& offspring)
{
    for(const std::unique_ptr<Individual>& indiv: offspring) {
        //this->mutation->mutateGenotype(indiv->getMutableGenotype(), std::move(this->representation.getGenotypeTemplate()), rng);
    }
}

void EvolutionAlgorithm::evaluatePopulation(
    const std::set<std::unique_ptr<Individual>, UniqueLess<Individual>>& offspring, 
    size_t generationNumber, Evaluation::LearningMode mode
    )
{
    std::set<std::reference_wrapper<const Individual>> evaluatedIndividuals = this->population->getNotProtectedIndividuals();
    for (const std::unique_ptr<Individual>& os: offspring) {
        evaluatedIndividuals.insert(*os);
    }

    //this->evaluation.evaluateIndividuals(  evaluatedIndividuals, generationNumber, mode);
}

void EvolutionAlgorithm::selectSurvivors(
    std::set<std::unique_ptr <Individual>, UniqueLess<Individual>>& offspring)
{

    /*std::set<std::reference_wrapper<const Individual>> evaluatedIndividuals = this->population->getNotProtectedIndividuals();
    for (const std::unique_ptr<Individual>& os: offspring) {
        evaluatedIndividuals.insert(*os);
    }
    std::map<std::reference_wrapper<const Individual>, bool> selectionResult = this->survivingSelection->select(evaluatedIndividuals, 0, this->rng);


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
    }*/
}