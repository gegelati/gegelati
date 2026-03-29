
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
                  std::reference_wrapper<const Algorithm::Agent>>& results,
    RNG::RNG& rng)
{

    // Clear values reevaluated
    for (auto& pair : this->mapEliteArchives) {
        std::shared_ptr<MapElitesArchive> mapEliteArchive = pair.second;
        for (auto it = results.begin(); it != results.end(); it++) {
            // The agent is already in the archive
            if (mapEliteArchive->containsAgent(it->second)) {
                // The agent has been reevaluated, delete it from the archive if
                // it has not been evaluated enough times
                mapEliteArchive->removeAgentFromArchiveIfNotComplete(
                    it->second, params.maxNbEvaluationPerPolicy);
            }
        }
    }

    for (auto& pair : this->mapEliteArchives) {
        std::shared_ptr<const MapElitesDescriptor> descriptor = pair.first;
        std::shared_ptr<MapElitesArchive> mapEliteArchive = pair.second;

        std::vector<std::reference_wrapper<const Algorithm::Agent>> verticesToDelete;

        size_t numberNewValues = 0;

        for (auto it = results.rbegin(); it != results.rend(); ++it) {

            // Get the selectionMetrics (casted) and agent
            auto metrics = std::dynamic_pointer_cast<MapElitesSelectionMetrics>(
                it->first->getSelectionMetrics());
            if (metrics == nullptr) {
                throw std::runtime_error("SelectionMetrics should be castable "
                                         "to MapElitesSelectionMetrics");
            }
            const Algorithm::Agent& agent = it->second;
      
            std::vector<double> descriptorUsed(
                metrics->getMapDescriptors().at(descriptor));  

            // Get the saved evaluation and agent
            const auto& pairSaved =
                mapEliteArchive->getArchiveFromDescriptors(descriptorUsed);

            // The value saved in the archive is better than the current agent
            // There is also a verification that the agent is not the same
            if (!pairSaved.second || (*pairSaved.second != agent &&
                pairSaved.first->getSelectionMetrics()->getScore() <
                     metrics->getScore())) {
                // Saving
                mapEliteArchive->setArchiveFromDescriptors(agent, it->first,
                                                           descriptorUsed);

            }
        }
    }

    for (auto it = results.begin(); it != results.end();) {
        bool containAgent = false;
        for (auto& pairArchive : this->mapEliteArchives) {
            if (pairArchive.second->containsAgent(it->second)) {
                containAgent = true;
                break;
            }
        }

        if (!containAgent) {
            this->removeFromSavedResults(it->second);
            this->getManager().deleteAgent(it->second, graph);
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
    std::set<std::reference_wrapper<const Algorithm::Agent>> agentsInAllArchives;
    for (auto& pair : this->mapEliteArchives) {
        std::set<std::reference_wrapper<const Algorithm::Agent>> agentsInArchive =
            pair.second->getVerticesInArchive();
        agentsInAllArchives.insert(agentsInArchive.begin(),
                                     agentsInArchive.end());
    }

    if(agentsInAllArchives.size() != 0){
        context->nbAgentsToCreate = (uint64_t)(params.mutation.tpg.nbRoots) + agentsInAllArchives.size();
    }

    return context;
}