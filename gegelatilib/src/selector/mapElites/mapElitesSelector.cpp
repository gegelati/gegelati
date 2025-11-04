

#include "selector/mapElites/mapElitesSelector.h"


void Selector::MapElites::MapElitesSelector::doSelection(
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                    const TPG::TPGVertex*>& results,
    Mutator::RNG& rng)
{

    // Clear values reevaluated
    for(auto& pair: this->mapEliteArchives){
        MapElitesArchive* mapEliteArchive = pair.second;
        for(auto it = results.begin(); it != results.end(); it++){
            // The root is already in the archive
            if(mapEliteArchive->containsRoot(it->second)){
                // The root has been reevaluated, delete it from the archive if it has not been evaluated enough times
                mapEliteArchive->removeRootFromArchiveIfNotComplete(it->second, params.maxNbEvaluationPerPolicy);
            }
        }
    }

    for(auto& pair: this->mapEliteArchives){
        const MapElitesDescriptor descriptor = pair.first;
        MapElitesArchive* mapEliteArchive = pair.second;

        std::vector<const TPG::TPGVertex*> verticesToDelete;

        size_t numberNewValues = 0;

        for (auto it = results.rbegin(); it != results.rend(); ++it){


            // Get the selectionMetrics (casted) and root
            auto metrics = std::dynamic_pointer_cast<MapElitesSelectionMetrics>(it->first->getSelectionMetrics());
            if (metrics == nullptr) {
                throw std::runtime_error("Evalresult should be castable to MapElitesSelectionMetrics");
            }
            const TPG::TPGVertex* root = it->second;

            std::vector<double> descriptorUsed(metrics->getMapDescriptors().at(descriptor));

            // Get the saved evaluation and root
            std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*> pairSaved = mapEliteArchive->getArchiveFromDescriptors(descriptorUsed);

            // The value saved in the archive is better than the current root
            // There is also a verification that the root is not the same
            if(pairSaved.second != nullptr && pairSaved.second != root && pairSaved.first->getSelectionMetrics()->getScore() >= metrics->getScore()){
                // Nothing happened

            // The current root is better than the values saved
            } else if (pairSaved.second != root) {
                numberNewValues++;

                // Saving
                mapEliteArchive->setArchiveFromDescriptors(root, it->first, descriptorUsed);
            }
        }

        std::cout<<"  nv "<<numberNewValues<<"  ";
    }

    for (auto it = results.begin(); it != results.end(); ) {
        bool containRoot = false;
        for(auto& pairArchive: this->mapEliteArchives){
            if(pairArchive.second->containsRoot(it->second)){
                containRoot = true;
                break;
            }
        }

        if (!containRoot) {
            this->resultsPerRoot.erase(it->second);
            graph->removeVertex(*it->second);
            it = results.erase(it); // erase returns next iterator
        } else {
            ++it;
        }
    }
}

const Selector::SelectionContext& Selector::MapElites::MapElitesSelector::updateContext()
{

}