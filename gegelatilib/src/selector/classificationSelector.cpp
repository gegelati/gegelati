

#include "selector/classificationSelector.h"
#include "learn/classificationEvaluationResult.h"

void Selector::ClassificationSelector::doSelection(std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                const TPG::TPGVertex*>& results, Mutator::RNG& rng)
{
    // Check that results are ClassificationEvaluationResults.
    // (also throws on empty results)
    const Learn::EvaluationResult* result = results.begin()->first.get();
    if (typeid(Learn::ClassificationEvaluationResult) != typeid(*result)) {
        throw std::runtime_error(
            "ClassificationLearningAgent can not decimate worst roots for "
            "results whose type is not ClassificationEvaluationResult.");
    }

    // Compute the number of root to keep/delete base on each criterion
    uint64_t totalNbRoot = this->graph->getNbRootVertices();
    uint64_t nbRootsToDelete =
        (uint64_t)floor(this->params.selection.truncation.ratioDeletedRoots * totalNbRoot);
    uint64_t nbRootsToKeep = (totalNbRoot - nbRootsToDelete);

    // Keep ~half+ of the roots based on their general score on
    // all class.
    // and ~half- of the roots on a per class score (none if nbRoots to keep
    // < 2*nb class)
    uint64_t nbRootsKeptPerClass =
        (nbRootsToKeep / this->nbActions) / 2;
    uint64_t nbRootsKeptGeneralScore =
        nbRootsToKeep -
        this->nbActions * nbRootsKeptPerClass;

    // Build a list of roots to keep
    std::vector<const TPG::TPGVertex*> rootsToKeep;

    // Insert roots to keep per class
    for (uint64_t classIdx = 0;
            classIdx < this->nbActions; classIdx++) {
        // Fill a map with the roots and the score of the specific class as
        // ID.
        std::multimap<double, const TPG::TPGVertex*> sortedRoot;
        std::for_each(
            results.begin(), results.end(),
            [&sortedRoot,
                &classIdx](const std::pair<std::shared_ptr<Learn::EvaluationResult>,
                                        const TPG::TPGVertex*>& res) {
                sortedRoot.emplace(
                    ((Learn::ClassificationEvaluationResult*)res.first.get())
                        ->getScorePerClass()
                        .at(classIdx),
                    res.second);
            });

        // Keep the best nbRootsKeptPerClass (or less for reasons explained
        // in the loop)
        auto iterator = sortedRoot.rbegin();
        for (auto i = 0; i < nbRootsKeptPerClass; i++) {
            // If the root is not already marked to be kept
            if (std::find(rootsToKeep.begin(), rootsToKeep.end(),
                            iterator->second) == rootsToKeep.end()) {
                rootsToKeep.push_back(iterator->second);
            }
            // Advance the iterator no matter what.
            // This means that if a root scores well for several classes
            // it is kept only once anyway, but additional roots will not
            // be kept for any of the concerned class.
            iterator++;
        }
    }

    // Insert remaining roots to keep
    auto iterator = results.rbegin();
    while (rootsToKeep.size() < nbRootsToKeep &&
            iterator != results.rend()) {
        // If the root is not already marked to be kept
        if (std::find(rootsToKeep.begin(), rootsToKeep.end(),
                        iterator->second) == rootsToKeep.end()) {
            rootsToKeep.push_back(iterator->second);
        }
        // Advance the iterator no matter what.
        iterator++;
    }

    // Do the removal.
    // Because of potential root actions, the preserved number of roots
    // may be higher than the given ratio.
    auto allRoots = this->graph->getRootVertices();
    auto& tpgRef = this->graph;
    auto& resultsPerRootRef = this->resultsPerRoot;
    std::for_each(
        allRoots.begin(), allRoots.end(),
        [&rootsToKeep, &tpgRef, &resultsPerRootRef,
            &results](const TPG::TPGVertex* vert) {
            // Do not remove actions
            if (dynamic_cast<const TPG::TPGAction*>(vert) == nullptr &&
                std::find(rootsToKeep.begin(), rootsToKeep.end(), vert) ==
                    rootsToKeep.end()) {
                tpgRef->removeVertex(*vert);

                // Keep only results of non-decimated roots.
                resultsPerRootRef.erase(vert);

                // Update results also
                std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                                const TPG::TPGVertex*>::iterator iter =
                    results.begin();
                while (iter != results.end()) {
                    if (iter->second == vert) {
                        results.erase(iter);
                        break;
                    }
                    iter++;
                }
            }
        });
}