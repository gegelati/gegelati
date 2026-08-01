

#include "selector/classificationSelector.h"
#include "selector/classificationSelectionMetrics.h"

std::shared_ptr<Selector::SelectionMetrics> Selector::ClassificationSelector::
    createSelectionMetrics() const
{
    return std::make_shared<ClassificationSelectionMetrics>();
}

void Selector::ClassificationSelector::doSelection(
    EvoGraph::Graph& graph,
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  std::reference_wrapper<const Representation::Individual>>& results,
    RNG::RNG& rng)
{
    // Check that results are ClassificationSelectionMetrics is used.
    // (also throws on empty results)
    const Learn::EvaluationResult* result = results.begin()->first.get();
    if (typeid(ClassificationSelectionMetrics) !=
        typeid(*result->getSelectionMetrics().get())) {
        throw std::runtime_error("Can not decimate worst individuals for "
                                 "results whose metrics type is not "
                                 "ClassificationSelectionMetrics.");
    }

    Representation::Population& population = this->getPopulation();

    // Compute the number of individual to keep/delete base on each criterion
    uint64_t totalNbIndividual = population.getIndividuals().size();
    uint64_t nbIndividualsToDelete = (uint64_t)floor(
        this->params->truncation.ratioDeletedRoots * totalNbIndividual);
    uint64_t nbIndividualsToKeep = (totalNbIndividual - nbIndividualsToDelete);

    // Keep ~half+ of the individuals based on their general score on
    // all class.
    // and ~half- of the individuals on a per class score (none if nbIndividuals to keep
    // < 2*nb class)
    uint64_t nbIndividualsKeptPerClass = (nbIndividualsToKeep / this->nbActions) / 2;
    uint64_t nbIndividualsKeptGeneralScore =
        nbIndividualsToKeep - this->nbActions * nbIndividualsKeptPerClass;

    // Build a list of individuals to keep
    std::vector<std::reference_wrapper<const Representation::Individual>> individualsToKeep;

    // Insert individuals to keep per class
    for (uint64_t classIdx = 0; classIdx < this->nbActions; classIdx++) {
        // Fill a map with the individuals and the score of the specific class as
        // ID.
        std::multimap<double, std::reference_wrapper<const Representation::Individual>> sortedIndividual;
        std::for_each(
            results.begin(), results.end(),
            [&sortedIndividual, &classIdx](
                const std::pair<std::shared_ptr<Learn::EvaluationResult>,
                                std::reference_wrapper<const Representation::Individual>>& res) {
                sortedIndividual.emplace(((ClassificationSelectionMetrics*)res.first
                                        ->getSelectionMetrics()
                                        .get())
                                       ->getScorePerClass()
                                       .at(classIdx),
                                   res.second);
            });

        // Keep the best nbIndividualsKeptPerClass (or less for reasons explained
        // in the loop)
        auto iterator = sortedIndividual.rbegin();
        for (auto i = 0; i < nbIndividualsKeptPerClass; i++) {
            // If the individual is not already marked to be kept
            const Representation::Individual& individual = iterator->second;
            if (std::find_if(individualsToKeep.begin(), individualsToKeep.end(), 
                [&individual](const std::reference_wrapper<const Representation::Individual>& individualToKeep) {
                    return individual == individualToKeep.get();
                }) == individualsToKeep.end()) {
                individualsToKeep.push_back(iterator->second);
            }
            // Advance the iterator no matter what.
            iterator++;
        }

    // Insert remaining individuals to keep
    auto iterator2 = results.rbegin();
    while (individualsToKeep.size() < nbIndividualsToKeep && iterator2 != results.rend()) {
        // If the individual is not already marked to be kept
        const Representation::Individual& lockedIndividual = iterator2->second;
        if (std::find_if(individualsToKeep.begin(), individualsToKeep.end(), 
            [&lockedIndividual](const std::reference_wrapper<const Representation::Individual>& individual) {
                return individual.get() == lockedIndividual;
            }) == individualsToKeep.end()) {
            individualsToKeep.push_back(iterator2->second);
        }
        // Advance the iterator no matter what.
        iterator2++;
    }

    // Do the removal.
    // Because of potential individual actions, the preserved number of individuals
    // may be higher than the given ratio.
    auto allIndividuals = population.getIndividuals();
    auto& graphRef = graph;
    std::for_each(
        allIndividuals.begin(), allIndividuals.end(),
        [&individualsToKeep, &graphRef, this, &population,
         &results](std::reference_wrapper<const Representation::Individual> currindividual) {

            if (std::find_if(individualsToKeep.begin(), individualsToKeep.end(), 
                [&currindividual](const std::reference_wrapper<const Representation::Individual>& individual) {
                    return individual.get() == currindividual.get();
                }) == individualsToKeep.end()) {
                population.deleteIndividual(currindividual, graphRef);

                // Keep only results of non-decimated individuals.
                this->removeFromSavedResults(currindividual);

                // Update results also
                std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                              std::reference_wrapper<const Representation::Individual>>::iterator iter =
                    results.begin();
                while (iter != results.end()) {
                    if (iter->second == currindividual) {
                        results.erase(iter);
                        break;
                    }
                    iter++;
                }
            }
        });
    }
}