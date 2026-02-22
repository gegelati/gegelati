#include "algorithm/species/speciesAlgorithm.h"

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
    for(size_t idx = 0; idx < this->outputs->sizeContinuous(); idx++){
        programOutput->addOutput(Output::Output());
    }

    // Init program algorithm
    programAlgo.initAlgorithm(rng, *programOutput, dataSource, graph);

    // Add program manager and mutator to Species manager and mutator
    this->manager->addSubManager(programAlgo.getManager());
    SpeciesManager* speciesManager = dynamic_cast<SpeciesManager*>(this->manager.get());
    speciesManager->setProgramAlgorithmID(this->programAlgorithmID);

    this->mutator->addSubMutator(programAlgo.getMutator());
    SpeciesMutator* speciesMutator = dynamic_cast<SpeciesMutator*>(this->mutator.get());
    speciesMutator->setProgramAlgorithmID(this->programAlgorithmID);
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
    return std::make_shared<SpeciesPolicyStats>(this->algorithmName, this->algorithmID, subPolicyStatsMap);
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

    for(const EvoGraph::Edge& edge: this->graph->getEdges()){
        if(edge.getProgram().getAlgorithmID() == this->programAlgorithmID){
            usedSubAgents[this->programAlgorithmID].insert(edge.getProgram());
        }
    }
    return usedSubAgents;
}

void Algorithm::Species::SpeciesAlgorithm::printAgent(const Agent& agent, FILE* pFile, std::string offset, std::set<uint64_t>& printedAgentID, std::vector<std::reference_wrapper<const EvoGraph::Element>>& elementsToPrint) const
{
    if(printedAgentID.find(agent.getAgentID()) == printedAgentID.end() && this->containsAgent(agent)){
        printedAgentID.insert(agent.getAgentID());

        // Get vertex of the SpeciesAgent
        const EvoGraph::Vertex& vertex = dynamic_cast<const SpeciesAgent&>(agent).getVertex();
        elementsToPrint.push_back(vertex);
    
        fprintf(pFile,
                "%sP%" PRIu64 " [fillcolor=\"%s\" shape=diamond margin=0.03 "
                "width=0 height=0 label=\"%s.%" PRIu64 "\"]\n",
                offset.c_str(), agent.getAgentID(), this->algorithmColor.c_str(), this->algorithmName.c_str(), this->algorithmID);
    }   
}

const Algorithm::Agent& Algorithm::Species::SpeciesAlgorithm::readAgent(std::smatch& matches)
{
    return dynamic_cast<SpeciesManager&>(*this->manager).createEmptyAgent();
}

void Algorithm::Species::SpeciesAlgorithm::linkAgentVertex(const Agent& agent, const EvoGraph::Vertex& vertex)
{
    dynamic_cast<SpeciesManager&>(*this->manager).setVertex(agent, vertex);
}