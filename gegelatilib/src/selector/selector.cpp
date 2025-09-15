

#include "selector/selector.h"

void Selector::Selector::doSelection(std::multimap<std::shared_ptr<Learn::EvaluationResult>,
    const TPG::TPGVertex*>& results, Mutator::RNG& rng)
{
    throw std::runtime_error("Selection with main selector is not allowed, subclasses like TruncationSelector should be used.");    
}


void Selector::Selector::keepBestPolicy()
{
    // Evaluate all roots
    if (this->graph->hasVertex(*this->bestRoot.first)) {
        auto bestRootVertex = this->bestRoot.first;

        // Remove all but the best root from the graph
        while (this->graph->getNbRootVertices() != 1) {
            auto roots = this->graph->getRootVertices();
            for (auto root : roots) {
                if (root != bestRootVertex) {
                    graph->removeVertex(*root);
                }
            }
        }
    }
}

void Selector::Selector::updateEvaluationRecords(const std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                        const TPG::TPGVertex*>& results)
{
    // Update resultsPerRoot
    this->updateResultsPerRoot(results);

    // Update bestRoot
    this->updateBestRoot(results);

}

void Selector::Selector::updateResultsPerRoot(const std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                        const TPG::TPGVertex*>& results)
{
    for (auto result : results) {
        auto mapIterator = this->resultsPerRoot.find(result.second);
        if (mapIterator == this->resultsPerRoot.end()) {
            // First time this root is evaluated
            this->resultsPerRoot.emplace(result.second, result.first);
        }
        else if (result.first != mapIterator->second) {
            // This root has already been evaluated.
            // If the received result pointer is different from the one
            // stored in the map, update the one in the map by replacing it
            // with the new one (which was combined with the pre-existing
            // one in evalRoot)
            mapIterator->second = result.first;
            // If the received result is associated to the current bestRoot,
            // update it.
            if (result.second == this->bestRoot.first) {
                this->bestRoot.second = result.first;
            }
        }
    }
}

void Selector::Selector::updateBestRoot(const std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                        const TPG::TPGVertex*>& results)
{
    auto iterator = --results.end();
    const std::shared_ptr<Learn::EvaluationResult> evaluation = iterator->first;
    const TPG::TPGVertex* candidate = iterator->second;
    // Test the three replacement cases
    // from the simpler to the most complex to test
    if (this->bestRoot.first == nullptr         // NULL case
        || *this->bestRoot.second < *evaluation // new high-score case
        || !this->graph->hasVertex(
            *this->bestRoot.first) // bestRoot disappearance
    ) {
        // Replace the best root
        this->bestRoot = {candidate, evaluation};
    }
}

const std::pair<const TPG::TPGVertex*, std::shared_ptr<Learn::EvaluationResult>>& Selector::Selector::getBestRoot() const
{
    return this->bestRoot;
}

void Selector::Selector::forgetPreviousResult()
{
    this->resultsPerRoot.clear();
    this->bestRoot.first = nullptr;
    this->bestRoot.second = nullptr;
}

const std::map<const TPG::TPGVertex*, std::shared_ptr<Learn::EvaluationResult>>& Selector::Selector::getResultsPerRoot() const
{
    return this->resultsPerRoot;
}