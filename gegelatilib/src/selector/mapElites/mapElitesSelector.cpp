
#include "selector/mapElites/mapElitesSelector.h"


std::shared_ptr<const Selector::MapElites::MapElitesArchive> Selector::MapElites::MapElitesSelector::getMapElitesArchiveAt(std::shared_ptr<const MapElitesDescriptor> descriptor)
{
    if(this->mapEliteArchives.find(descriptor) == this->mapEliteArchives.end()){
        throw std::runtime_error("No archive found for the given descriptor");
    }
    return this->mapEliteArchives.at(descriptor);
}

const std::map<std::shared_ptr<const Selector::MapElites::MapElitesDescriptor>, std::shared_ptr<Selector::MapElites::MapElitesArchive>>& Selector::MapElites::MapElitesSelector::getMapElitesArchives()
{
    return this->mapEliteArchives;
}



std::shared_ptr<Selector::SelectionMetrics> Selector::MapElites::MapElitesSelector::createSelectionMetrics()
{
    if(this->mapEliteArchives.size() == 0){
        throw std::runtime_error("No Descriptors have been added to mapElites!");
    }

    std::vector<std::shared_ptr<const MapElitesDescriptor>> descriptors;

    for(auto& pair: this->mapEliteArchives){
        if(!pair.first->isInit()){
            throw std::runtime_error("A descriptor is not initialized");
        }
        descriptors.push_back(pair.first);
    }

    return std::make_shared<MapElitesSelectionMetrics>(descriptors);
}

void Selector::MapElites::MapElitesSelector::addArchiveFromDescriptor(size_t nbBins, std::shared_ptr<const MapElitesDescriptor> descriptor, Learn::LearningEnvironment& le)
{
    std::pair<size_t, size_t> minAndMaxRange = descriptor->getMinAndMaxRange();
    mapEliteArchives.insert({descriptor, std::make_shared<MapElitesArchive>(nbBins, descriptor->getNbDescriptors(), minAndMaxRange.first, minAndMaxRange.second)});
}

void Selector::MapElites::MapElitesSelector::addCvtArchiveFromDescriptor(size_t nbCentroids, std::shared_ptr<const MapElitesDescriptor> descriptor, Learn::LearningEnvironment& le, Mutator::RNG& rng, size_t nbIterationInit, size_t nbDotsInit, double a1, double b1, double a2, double b2)
{
    std::pair<size_t, size_t> minAndMaxRange = descriptor->getMinAndMaxRange();
    mapEliteArchives.insert({descriptor, std::make_shared<CvtMapElitesArchive>(rng, nbCentroids, descriptor->getNbDescriptors(), minAndMaxRange.first, minAndMaxRange.second, nbIterationInit, nbDotsInit, a1, b1, a2, b2)});
}

void Selector::MapElites::MapElitesSelector::doSelection(
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                    const TPG::TPGVertex*>& results,
    Mutator::RNG& rng)
{

    // Clear values reevaluated
    for(auto& pair: this->mapEliteArchives){
        std::shared_ptr<MapElitesArchive> mapEliteArchive = pair.second;
        for(auto it = results.begin(); it != results.end(); it++){
            // The root is already in the archive
            if(mapEliteArchive->containsRoot(it->second)){
                // The root has been reevaluated, delete it from the archive if it has not been evaluated enough times
                mapEliteArchive->removeRootFromArchiveIfNotComplete(it->second, params.maxNbEvaluationPerPolicy);
            }
        }
    }

    for(auto& pair: this->mapEliteArchives){
        std::shared_ptr<const MapElitesDescriptor> descriptor = pair.first;
        std::shared_ptr<MapElitesArchive> mapEliteArchive = pair.second;

        std::vector<const TPG::TPGVertex*> verticesToDelete;

        size_t numberNewValues = 0;

        for (auto it = results.rbegin(); it != results.rend(); ++it){


            // Get the selectionMetrics (casted) and root
            auto metrics = std::dynamic_pointer_cast<MapElitesSelectionMetrics>(it->first->getSelectionMetrics());
            if (metrics == nullptr) {
                throw std::runtime_error("SelectionMetrics should be castable to MapElitesSelectionMetrics");
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
    Selector::Selector::updateContext();

    // Get all the vertices in the different archives
    std::set<const TPG::TPGVertex*> verticesInAllArchives;
    for(auto& pair: this->mapEliteArchives){
        std::set<const TPG::TPGVertex*> verticesInArchive = pair.second->getVerticesInArchive();
        verticesInAllArchives.insert(verticesInArchive.begin(), verticesInArchive.end());
    }

    // Get the number of team agent and action agent.
    size_t nbTeamsInArchives = 0;
    size_t nbActionsInArchives = 0;
    for(auto& vertex: verticesInAllArchives){
        if(dynamic_cast<const TPG::TPGTeam*>(vertex) != nullptr){
            nbTeamsInArchives++;
        } else {
            nbActionsInArchives++;
        }
    }

    // Update the number of team and archive to create.
    this->context.nbTeamsToCreate += nbTeamsInArchives;
    this->context.nbActionsToCreate += nbActionsInArchives;

    return this->context;
}