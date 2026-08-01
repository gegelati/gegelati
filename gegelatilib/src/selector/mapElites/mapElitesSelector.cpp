
#include "selector/mapElites/mapElitesSelector.h"
#include "selector/mapElites/cvtMapElitesArchive.h"

std::shared_ptr<const Selector::MapElites::MapElitesArchive> Selector::
    MapElites::MapElitesSelector::getMapElitesArchiveAt(
        std::shared_ptr<const MapElitesDescriptor> descriptor)
{
    if (this->mapEliteArchives.find(descriptor) ==
        this->mapEliteArchives.end()) {
        throw std::runtime_error("No archive found for the given descriptor");
    }
    return this->mapEliteArchives.at(descriptor);
}

const std::map<std::shared_ptr<const Selector::MapElites::MapElitesDescriptor>,
               std::shared_ptr<Selector::MapElites::MapElitesArchive>>&
Selector::MapElites::MapElitesSelector::getMapElitesArchives()
{
    return this->mapEliteArchives;
}

std::shared_ptr<Selector::SelectionMetrics> Selector::MapElites::
    MapElitesSelector::createSelectionMetrics() const
{
    if (this->mapEliteArchives.size() == 0) {
        throw std::runtime_error(
            "No Descriptors have been added to mapElites!");
    }

    std::vector<std::shared_ptr<const MapElitesDescriptor>> descriptors;

    for (auto& pair : this->mapEliteArchives) {
        if (!pair.first->isInit()) {
            throw std::runtime_error("A descriptor is not initialized");
        }
        descriptors.push_back(pair.first);
    }

    return std::make_shared<MapElitesSelectionMetrics>(descriptors);
}

std::shared_ptr<const Selector::MapElites::MapElitesArchive> Selector::
    MapElites::MapElitesSelector::addArchiveFromDescriptor(
        size_t nbBins, std::shared_ptr<const MapElitesDescriptor> descriptor,
        Learn::LearningEnvironment& le)
{
    std::pair<double, double> minAndMaxRange = descriptor->getMinAndMaxRange();
    std::shared_ptr<MapElitesArchive> archive =
        std::make_shared<MapElitesArchive>(
            nbBins, descriptor->getNbDescriptors(), minAndMaxRange.first,
            minAndMaxRange.second);

    mapEliteArchives.insert({descriptor, archive});

    return archive;
}

std::shared_ptr<const Selector::MapElites::MapElitesArchive> Selector::
    MapElites::MapElitesSelector::addCvtArchiveFromDescriptor(
        size_t nbCentroids,
        std::shared_ptr<const MapElitesDescriptor> descriptor,
        Learn::LearningEnvironment& le, RNG::RNG& rng,
        size_t nbIterationInit, size_t nbDotsInit, double a1, double b1,
        double a2, double b2)
{
    std::pair<double, double> minAndMaxRange = descriptor->getMinAndMaxRange();
    std::shared_ptr<MapElitesArchive> archive =
        std::make_shared<CvtMapElitesArchive>(
            rng, nbCentroids, descriptor->getNbDescriptors(),
            minAndMaxRange.first, minAndMaxRange.second, nbIterationInit,
            nbDotsInit, a1, b1, a2, b2);

    mapEliteArchives.insert({descriptor, archive});

    return archive;
}

void Selector::MapElites::MapElitesSelector::doSelection(
    EvoGraph::Graph& graph,
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  std::reference_wrapper<const Representation::Individual>>& results,
    RNG::RNG& rng)
{

    // Clear values reevaluated
    for (auto& pair : this->mapEliteArchives) {
        std::shared_ptr<MapElitesArchive> mapEliteArchive = pair.second;
        for (auto it = results.begin(); it != results.end(); it++) {
            // The individual is already in the archive
            if (mapEliteArchive->containsIndividual(it->second)) {
                // The individual has been reevaluated, delete it from the archive if
                // it has not been evaluated enough times
                mapEliteArchive->removeIndividualFromArchive(it->second, this->maxNbEvaluation);
            }
        }
    }

    for (auto& pair : this->mapEliteArchives) {
        std::shared_ptr<const MapElitesDescriptor> descriptor = pair.first;
        std::shared_ptr<MapElitesArchive> mapEliteArchive = pair.second;

        std::vector<std::reference_wrapper<const Representation::Individual>> verticesToDelete;

        size_t numberNewValues = 0;

        for (auto it = results.rbegin(); it != results.rend(); ++it) {

            // Get the selectionMetrics (casted) and individual
            auto metrics = std::dynamic_pointer_cast<MapElitesSelectionMetrics>(
                it->first->getSelectionMetrics());
            if (metrics == nullptr) {
                throw std::runtime_error("SelectionMetrics should be castable "
                                         "to MapElitesSelectionMetrics");
            }
            const Representation::Individual& individual = it->second;
      
            std::vector<double> descriptorUsed(
                metrics->getMapDescriptors().at(descriptor));  

            // Get the saved evaluation and individual
            const auto& pairSaved =
                mapEliteArchive->getArchiveFromDescriptors(descriptorUsed);

            // The value saved in the archive is better than the current individual
            // There is also a verification that the individual is not the same
            if (!pairSaved.second || (*pairSaved.second != individual &&
                pairSaved.first->getSelectionMetrics()->getScore() <
                     metrics->getScore())) {
                // Saving
                mapEliteArchive->setArchiveFromDescriptors(individual, it->first,
                                                           descriptorUsed);

            }
        }
    }

    for (auto it = results.begin(); it != results.end();) {
        bool containIndividual = false;
        for (auto& pairArchive : this->mapEliteArchives) {
            if (pairArchive.second->containsIndividual(it->second)) {
                containIndividual = true;
                break;
            }
        }

        if (!containIndividual) {
            this->removeFromSavedResults(it->second);
            this->getPopulation().deleteIndividual(it->second, graph);
            it = results.erase(it); // erase returns next iterator
        }
        else {
            ++it;
        }
    }
}

std::unique_ptr<Selector::SelectionContext> Selector::MapElites::MapElitesSelector::
    updateContext() const
{
    std::unique_ptr<SelectionContext> context = std::move(Selector::Selector::updateContext());

    // Get all the vertices in the different archives
    std::set<std::reference_wrapper<const Representation::Individual>> individualsInAllArchives;
    for (auto& pair : this->mapEliteArchives) {
        std::set<std::reference_wrapper<const Representation::Individual>> individualsInArchive =
            pair.second->getVerticesInArchive();
        individualsInAllArchives.insert(individualsInArchive.begin(),
                                     individualsInArchive.end());
    }

    if(individualsInAllArchives.size() != 0){
        context->nbIndividualsToCreate = this->nbIndividuals + individualsInAllArchives.size();
    }

    return context;
}