/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2025) :
 *
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

#include "selector/classificationSelector.h"
#include "selector/classificationSelectionMetrics.h"

std::shared_ptr<Selector::SelectionMetrics> Selector::ClassificationSelector::
    createSelectionMetrics()
{
    return std::make_shared<ClassificationSelectionMetrics>();
}

void Selector::ClassificationSelector::doSelection(
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const TPG::TPGVertex*>& results,
    Mutator::RNG& rng)
{
    // Check that results are ClassificationSelectionMetrics is used.
    // (also throws on empty results)
    const Learn::EvaluationResult* result = results.begin()->first.get();
    if (typeid(ClassificationSelectionMetrics) !=
        typeid(*result->getSelectionMetrics().get())) {
        throw std::runtime_error("Can not decimate worst roots for "
                                 "results whose metrics type is not "
                                 "ClassificationSelectionMetrics.");
    }

    // Compute the number of root to keep/delete base on each criterion
    uint64_t totalNbRoot = this->graph->getNbRootVertices();
    uint64_t nbRootsToDelete = (uint64_t)floor(
        this->params.selection.truncation.ratioDeletedRoots * totalNbRoot);
    uint64_t nbRootsToKeep = (totalNbRoot - nbRootsToDelete);

    // Keep ~half+ of the roots based on their general score on
    // all class.
    // and ~half- of the roots on a per class score (none if nbRoots to keep
    // < 2*nb class)
    uint64_t nbRootsKeptPerClass = (nbRootsToKeep / this->nbActions) / 2;
    uint64_t nbRootsKeptGeneralScore =
        nbRootsToKeep - this->nbActions * nbRootsKeptPerClass;

    // Build a list of roots to keep
    std::vector<const TPG::TPGVertex*> rootsToKeep;

    // Insert roots to keep per class
    for (uint64_t classIdx = 0; classIdx < this->nbActions; classIdx++) {
        // Fill a map with the roots and the score of the specific class as
        // ID.
        std::multimap<double, const TPG::TPGVertex*> sortedRoot;
        std::for_each(
            results.begin(), results.end(),
            [&sortedRoot, &classIdx](
                const std::pair<std::shared_ptr<Learn::EvaluationResult>,
                                const TPG::TPGVertex*>& res) {
                sortedRoot.emplace(((ClassificationSelectionMetrics*)res.first
                                        ->getSelectionMetrics()
                                        .get())
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
    while (rootsToKeep.size() < nbRootsToKeep && iterator != results.rend()) {
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
