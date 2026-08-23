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

std::map<std::reference_wrapper<const Evolution::Individual>, std::shared_ptr<Evaluation::EvaluationResult>> Evolution::EvolutionAlgorithm::evaluatePopulation(
        const std::set<std::unique_ptr<Individual>, UniqueLess<Individual>>& offspring, size_t generationNumber, Learn::LearningMode mode
    )
{
    std::vector<std::reference_wrapper<const Evolution::Individual>> evaluatedIndividuals = this->population->getNotProtectedIndividuals();
    for (const std::unique_ptr<Individual>& os: offspring) {
        evaluatedIndividuals.push_back(*os);
    }

    return this->evaluation.evaluateIndividuals(
        evaluatedIndividuals,*this->representation, generationNumber, mode);
}

std::map<std::reference_wrapper<const Evolution::Individual>, bool> Evolution::EvolutionAlgorithm::selectSurvivors(std::map<std::reference_wrapper<const Individual>, std::shared_ptr<Evaluation::EvaluationResult>>& scores)
{
    std::map<std::reference_wrapper<const Evolution::Individual>, bool> selectionResult = this->survivingSelection->select(scores);

    for(auto it = scores.begin(); it != scores.end();) {
        // If individual is selected, keep it, else erase it from the scores.
        if (selectionResult.find(it->first)->second) {
            it++;
        } else {
            it = scores.erase(it);
        }
    }

    return selectionResult;
}

void Evolution::EvolutionAlgorithm::replacePopulation(
    std::set<std::unique_ptr<Individual>, UniqueLess<Individual>>& offspring,
    std::map<std::reference_wrapper<const Individual>, bool>& selectionResult)
{
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
                if (!this->population->deleteIndividual(indiv)) {
                    throw std::runtime_error(
                        "EvolutionAlgorithm::replacePopulation: "
                        "a protected individual was evaluated and lost selection");
                }
            } else {
                offspring.erase(offspring.find(&indiv));
            }
        }

        it = selectionResult.erase(it);   // clean the whole map as we go
    }
}