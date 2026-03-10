#include "algorithm/species/speciesAlgorithm.h"


void Algorithm::Species::SpeciesAlgorithm::clearAlgorithm()
{   
    std::set<std::reference_wrapper<const EvoGraph::Team>> teams(dynamic_cast<SpeciesManager&>(*this->manager).getTeams());

    Algorithm::clearAlgorithm();
    
    for(const EvoGraph::Vertex& vertex: teams) {
        this->graph->removeVertex(vertex);
    }
}
std::unique_ptr<Algorithm::Algorithm> Algorithm::Species::SpeciesAlgorithm::initNewSpecies(RNG::RNG& rng)
{
    std::unique_ptr<Algorithm> copyAlgo = this->copy();
    SpeciesAlgorithm& copySpeciesAlgo = dynamic_cast<SpeciesAlgorithm&>(*copyAlgo);
    copySpeciesAlgo.setParentID(this->algorithmID);

    std::map<std::reference_wrapper<const EvoGraph::Edge>, std::reference_wrapper<const EvoGraph::Edge>> edgeMap;

    SpeciesMutator& speciesMutator = dynamic_cast<SpeciesMutator&>(*this->mutator);
    const EvoGraph::Vertex& newRootVertex = speciesMutator.mutateSpeciesGraph(*this->graph, *this->manager, this->params, rng, edgeMap);

    copySpeciesAlgo.setRootVertex(newRootVertex);
    copyAlgo->initAlgorithm(rng, *this->outputs, this->dataSources, this->graph);

    double prop = 1.0;
    std::vector<std::reference_wrapper<const Agent>> originAgents(this->getAgents());
    std::vector<std::reference_wrapper<const Agent>> exchangedAgents;

    size_t nbAgentsExchanged = (size_t)(prop * originAgents.size());
    for(size_t idx = 0; idx < nbAgentsExchanged; idx++) {
        auto it = originAgents.begin();
        std::advance(it, rng.getUnsignedInt64(0, originAgents.size() - 1));
        const Agent& selectedAgent = *it;

        originAgents.erase(it);

        // Add the agent to the new algorithm.
        dynamic_cast<SpeciesMutator&>(copySpeciesAlgo.getMutator()).initAgentFromSpecies(selectedAgent, *this->graph, copyAlgo->getManager(), this->params, rng, edgeMap);

    }
    
    return copyAlgo;
}




bool Algorithm::Species::SpeciesAlgorithm::hasRootVertex() const
{
    return this->rootVertex != std::nullopt;
}

const EvoGraph::Vertex& Algorithm::Species::SpeciesAlgorithm::getRootVertex() const
{
    return *this->rootVertex;
}

void Algorithm::Species::SpeciesAlgorithm::setRootVertex(const EvoGraph::Vertex& newRootVertex)
{
    this->rootVertex = newRootVertex;
    if(this->init){
        dynamic_cast<SpeciesManager*>(this->manager.get())->setRootVertex(newRootVertex);
    }
}

double Algorithm::Species::SpeciesAlgorithm::calculateProportion(int position, int totalAlgos) {
    if (totalAlgos == 1) {
        return 1.0;
    }
    if (position == 1) {
        return static_cast<double>(2) / (totalAlgos + 1);
    }
    double remaining = 1.0 - calculateProportion(1, totalAlgos);
    return calculateProportion(position - 1, totalAlgos - 1) * remaining;
}




std::unique_ptr<Algorithm::Algorithm> Algorithm::Species::SpeciesAlgorithm::copy() const
{
    return std::make_unique<SpeciesAlgorithm>(this->params, this->cGetSubAlgorithm(this->programAlgorithmID), this->algorithmName);
}

void Algorithm::Species::SpeciesAlgorithm::setProgramAlgorithm(const Algorithm& programAlgorithm)
{
    Algorithm::Algorithm::addSubAlgorithm(programAlgorithm);

    // Set program algorithm name
    this->programAlgorithmID = this->subAlgorithms.back()->getAlgorithmID();
}


const Archive& Algorithm::Species::SpeciesAlgorithm::getArchive() const
{
    return *this->archive;
}

void Algorithm::Species::SpeciesAlgorithm::initManager()
{
    this->manager = std::make_unique<Species::SpeciesManager>(*this->outputs, this->algorithmID);
}

void Algorithm::Species::SpeciesAlgorithm::initMutator()
{
    this->mutator = std::make_unique<Species::SpeciesMutator>(*this->selector, this->algorithmID, *this->archive);
}

void Algorithm::Species::SpeciesAlgorithm::initSubAlgorithms(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{
    // Initialize program algorithm.
    Algorithm& programAlgo = this->getSubAlgorithm(this->programAlgorithmID);

    // Program output is only size 1, except for continuous outputs where we create more outputs (one per continuous output of the Species)
    auto programOutput = std::make_shared<Output::OutputHandler>(Output::Output());
    // Init program algorithm
    programAlgo.initAlgorithm(rng, *programOutput, dataSource, graph);

    // Add program manager and mutator to Species manager and mutator
    this->manager->addSubManager(programAlgo.getManager());
    SpeciesManager& speciesManager = dynamic_cast<SpeciesManager&>(*this->manager);
    speciesManager.setProgramAlgorithmID(this->programAlgorithmID);

    this->mutator->addSubMutator(programAlgo.getMutator());
    SpeciesMutator& speciesMutator = dynamic_cast<SpeciesMutator&>(*this->mutator);
    speciesMutator.setProgramAlgorithmID(this->programAlgorithmID);
}

void Algorithm::Species::SpeciesAlgorithm::initAlgorithm(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{
    Algorithm::initAlgorithm(rng, outputs, dataSource, graph);

    // Init the species and set the root vertex.
    SpeciesMutator& speciesMutator = dynamic_cast<SpeciesMutator&>(*this->mutator);
    SpeciesManager& speciesManager = dynamic_cast<SpeciesManager&>(*this->manager);
    if(!this->hasRootVertex()){
        this->setRootVertex(speciesMutator.initSpeciesGraphStructure(*graph, speciesManager, params, rng));
    } else {
        speciesManager.setRootVertex(*this->rootVertex);
    }
    speciesManager.setSpeciesGraphStructure();
}

std::shared_ptr<Algorithm::Job> Algorithm::Species::SpeciesAlgorithm::createJob(const Agent& agent, Learn::LearningMode mode, RNG::RNG& rng, int idx) const
{
    if(!this->containsAgent(agent)){
        throw std::runtime_error("LearningAgent::makeJob: Cannot create a job with a null agent or an agent not belonging to this algorithm.");
    }

    // Before each agent evaluation, set a new seed for the archive in
    // TRAINING Mode Else, archiving should be deactivate anyway
    std::unique_ptr<Archive> jobArchive = nullptr;
    if (mode == Learn::LearningMode::TRAINING) {
        size_t archiveSeed = rng.getUnsignedInt64(0, UINT64_MAX);
        jobArchive = std::make_unique<Archive>(this->params.archiveSize, this->params.archivingProbability, archiveSeed);
    }

    return std::make_shared<SpeciesJob>(agent, idx, std::move(jobArchive));
}

std::shared_ptr<Algorithm::PolicyStats> Algorithm::Species::SpeciesAlgorithm::createPolicyStats() const
{
    std::map<uint64_t, std::shared_ptr<PolicyStats>> subPolicyStatsMap;
    subPolicyStatsMap[this->programAlgorithmID] = this->cGetSubAlgorithm(this->programAlgorithmID).createPolicyStats();
    return std::make_shared<SpeciesPolicyStats>(this->algorithmName, this->algorithmID, subPolicyStatsMap, *this->rootVertex);
}

void Algorithm::Species::SpeciesAlgorithm::updateAfterEvaluation(const std::vector<std::shared_ptr<Job>>& jobs, Learn::LearningMode mode)
{
    // Merge the archives
    if (mode == Learn::LearningMode::TRAINING) {
        // Build archive map
        std::map<uint64_t, std::reference_wrapper<Archive>> archiveMap;
        for (const auto& jobPtr : jobs) {
            std::shared_ptr<const SpeciesJob> speciesJob = std::dynamic_pointer_cast<const SpeciesJob>(jobPtr);
            if(speciesJob == nullptr){
                throw std::runtime_error("Algorithm::Species::SpeciesAlgorithm::updateAfterEvaluation trying to update after evaluation with a job which is not a SpeciesJob");
            }
            std::reference_wrapper<Archive> archiveRef = speciesJob->getArchive();
            archiveMap.insert({jobPtr->getIdx(), archiveRef});
        }


        // Scan the archives backward, starting from the last to identify the
        // last params.archiveSize recordings to keep (or less).
        auto reverseIterator = archiveMap.rbegin();

        uint64_t nbRecordings = 0;
        while (nbRecordings < this->params.archiveSize &&
            reverseIterator != archiveMap.rend()) {
            nbRecordings += reverseIterator->second.get().getNbRecordings();
            reverseIterator++;
        }

        // Insert identified recordings into this->archive
        while (reverseIterator != archiveMap.rbegin()) {
            reverseIterator--;

            auto i = reverseIterator->first;

            // Skip recordings in the first archive if needed
            uint64_t recordingIdx = 0;
            while (nbRecordings > this->params.archiveSize) {
                recordingIdx++;
                nbRecordings--;
            }

            // Insert remaining recordings
            while (recordingIdx < reverseIterator->second.get().getNbRecordings()) {
                // Access in reverse order
                const ArchiveRecording& recording =
                    reverseIterator->second.get().at(recordingIdx);
                // forced Insertion
                this->archive->addRecording(
                    *recording.agent,
                    reverseIterator->second.get().getDataHandlers().at(
                        recording.dataHash),
                    recording.result, true);
                recordingIdx++;

            }
        }
    }
}


std::map<uint64_t, std::set<std::reference_wrapper<const Algorithm::Agent>>> Algorithm::Species::SpeciesAlgorithm::getUsedSubAgents() const
{
    std::map<uint64_t, std::set<std::reference_wrapper<const Agent>>> usedSubAgents;
    usedSubAgents[this->programAlgorithmID] = std::set<std::reference_wrapper<const Agent>>();

    for(const Agent& agent: this->manager->getAgents()) {
        const SpeciesAgent& speciesAgent = dynamic_cast<const SpeciesAgent&>(agent);
        for(const auto& pair: speciesAgent.getActionPrograms()) {
            usedSubAgents[this->programAlgorithmID].insert(*pair.second);
        }
        for(const auto& pair: speciesAgent.getContextPrograms()) {
            usedSubAgents[this->programAlgorithmID].insert(*pair.second);
        }
    }
    return usedSubAgents;
}

void Algorithm::Species::SpeciesAlgorithm::printAlgorithm(FILE* pFile, std::string offset, std::vector<std::reference_wrapper<const EvoGraph::Element>>& elementsToPrint) const
{
    double finalScore = selector->getLastEMAScore();
    
    fprintf(pFile,
            "%sALGO%" PRIu64 " [fillcolor=\"%s\" shape=diamond margin=0.03 "
            "label=\"%s.%" PRIu64 "\" parentID=\"%" PRIu64 "\" age=\"%" PRIu64 "\" nbTimeReproduced=\"%" PRIu64 "\" finalScore=\"%f\", startingGeneration=\"%" PRIu64 "\"]\n",
            offset.c_str(), this->algorithmID, this->algorithmColor.c_str(), this->algorithmName.c_str(), this->algorithmID, this->parentID, this->age, this->nbTimesReproduced, finalScore, this->startingGeneration);



    elementsToPrint.push_back(dynamic_cast<SpeciesManager*>(this->manager.get())->getRootVertex());

    
    fprintf(pFile, "%sALGO%" PRIu64 " -> T%" PRIu64 " [style=dashed]\n",
            offset.c_str(), 
            this->algorithmID, 
            dynamic_cast<SpeciesManager*>(this->manager.get())->getRootVertex().getVertexID());
}

void Algorithm::Species::SpeciesAlgorithm::printAgent(const Agent& agent, FILE* pFile, std::string offset, std::set<uint64_t>& printedAgentID, std::vector<std::reference_wrapper<const EvoGraph::Element>>& elementsToPrint, std::vector<std::reference_wrapper<const Agent>>& agentsToPrint) const
{
    if(printedAgentID.find(agent.getAgentID()) == printedAgentID.end() && this->containsAgent(agent)){
        printedAgentID.insert(agent.getAgentID());

        std::string edgeInfo = "";

        const SpeciesAgent& speciesAgent = dynamic_cast<const SpeciesAgent&>(agent);
        for(const auto& pair: speciesAgent.getActionPrograms()) {
            agentsToPrint.push_back(*pair.second);
            
            fprintf(pFile, "%sP%" PRIu64 " -> P%" PRIu64 " [style=dashed]\n",
                    offset.c_str(), agent.getAgentID(), pair.second->get().getAgentID());

            edgeInfo += std::to_string(pair.second->get().getAgentID()) + ";" + std::to_string(pair.first.get().getEdgeID()) + "|";
        }
        for(const auto& pair: speciesAgent.getContextPrograms()) {
            agentsToPrint.push_back(*pair.second);
            
            fprintf(pFile, "%sP%" PRIu64 " -> P%" PRIu64 " [style=dashed]\n",
                    offset.c_str(), agent.getAgentID(), pair.second->get().getAgentID());

            edgeInfo += std::to_string(pair.second->get().getAgentID()) + ";" + std::to_string(pair.first.get().getEdgeID()) + "|";
        }

        std::string actionInfo = "";
        for(const auto& pair: speciesAgent.getActionLinks()) {
            actionInfo += std::to_string(pair.first) + "," + std::to_string(pair.second) + "|";
        }
    
        fprintf(pFile,
                "%sP%" PRIu64 " [fillcolor=\"%s\" shape=diamond margin=0.03 "
                "width=0 height=0 label=\"%s.%" PRIu64 "\" edgeInfo=\"%s\" actionInfo=\"%s\"]\n",
                offset.c_str(), agent.getAgentID(), this->algorithmColor.c_str(), this->algorithmName.c_str(), this->algorithmID, edgeInfo.c_str(), actionInfo.c_str());

                


        /// TODO
        /*for(const auto& pair: speciesAgent.getPrograms()) {
            this->graph->removeEdgeProgram(pair.first);
        }*/
    }  
}

const Algorithm::Agent& Algorithm::Species::SpeciesAlgorithm::readAgent(std::smatch& matches)
{
    return dynamic_cast<SpeciesManager&>(*this->manager).createAgent(*this->graph);
}