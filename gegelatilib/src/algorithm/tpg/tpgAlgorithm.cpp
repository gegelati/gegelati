
#include "algorithm/tpg/tpgAlgorithm.h"


void Algorithm::TPG::TPGAlgorithm::setProgramAlgorithm(std::shared_ptr<Algorithm> programAlgorithm)
{
    Algorithm::Algorithm::addSubAlgorithm(programAlgorithm);

    // Set program algorithm name
    this->programAlgorithmName = programAlgorithm->getAlgorithmName();
}


std::shared_ptr<const Archive> Algorithm::TPG::TPGAlgorithm::getArchive() const
{
    return this->archive;
}

void Algorithm::TPG::TPGAlgorithm::initManager(std::shared_ptr<const Output::OutputHandler> outputs)
{
    this->manager = std::make_shared<TPG::TPGManager>(*outputs);
    this->manager->setAlgorithmName(algorithmName);
}

void Algorithm::TPG::TPGAlgorithm::initMutator()
{
    this->mutator = std::make_shared<TPG::TPGMutator>(*this->selector, this->archive);
    this->mutator->setAlgorithmName(algorithmName);
}

void Algorithm::TPG::TPGAlgorithm::initSubAlgorithms(RNG::RNG& rng, std::shared_ptr<const Output::OutputHandler> outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{
    // Initialize program algorithm.
    std::shared_ptr<Algorithm> programAlgo = this->getSubAlgorithm(this->programAlgorithmName);

    // Program output is only size 1, except for continuous outputs where we create more outputs (one per continuous output of the TPG)
    auto programOutput = std::make_shared<Output::OutputHandler>(Output::Output());
    for(size_t idx = 0; idx < this->outputs->sizeContinuous(); idx++){
        programOutput->addOutput(Output::Output());
    }

    // Init program algorithm
    programAlgo->initAlgorithm(rng, programOutput, dataSource, graph);

    // Add program manager and mutator to TPG manager and mutator
    this->manager->addSubManager(programAlgo->getManager());
    std::shared_ptr<TPG::TPGManager> tpgManager = std::dynamic_pointer_cast<TPG::TPGManager>(this->manager);
    tpgManager->setProgramAlgorithmName(this->programAlgorithmName);

    this->mutator->addSubMutator(programAlgo->getMutator());
    std::shared_ptr<TPG::TPGMutator> tpgMutator = std::dynamic_pointer_cast<TPG::TPGMutator>(this->mutator);
    tpgMutator->setProgramAlgorithmName(this->programAlgorithmName);
}

std::shared_ptr<Algorithm::Job> Algorithm::TPG::TPGAlgorithm::createJob(std::shared_ptr<const Agent> agent, Learn::LearningMode mode, RNG::RNG& rng, int idx) const
{
    if(agent == nullptr || !this->containsAgent(agent)){
        throw std::runtime_error("LearningAgent::makeJob: Cannot create a job with a null agent or an agent not belonging to this algorithm.");
    }

    // Before each agent evaluation, set a new seed for the archive in
    // TRAINING Mode Else, archiving should be deactivate anyway
    Archive* jobArchive = nullptr;
    if (mode == Learn::LearningMode::TRAINING) {
        size_t archiveSeed = rng.getUnsignedInt64(0, UINT64_MAX);
        jobArchive = new Archive (this->params.archiveSize, this->params.archivingProbability, archiveSeed);
    }

    return std::make_shared<TPGJob>(agent, idx, jobArchive);
}

void Algorithm::TPG::TPGAlgorithm::updateAfterEvaluation(const std::vector<std::shared_ptr<Job>>& jobs, Learn::LearningMode mode)
{
    // Merge the archives
    if (mode == Learn::LearningMode::TRAINING) {
        // Build archive map
        std::map<uint64_t, Archive*> archiveMap;
        for (const auto& jobPtr : jobs) {
            std::shared_ptr<const TPGJob> tpgJob = std::dynamic_pointer_cast<const TPGJob>(jobPtr);
            if(tpgJob == nullptr){
                throw std::runtime_error("Algorithm::TPG::TPGAlgorithm::updateAfterEvaluation trying to update after evaluation with a job which is not a TPGJob");
            }
            archiveMap[jobPtr->getIdx()] = tpgJob->getArchive();
        }


        // Scan the archives backward, starting from the last to identify the
        // last params.archiveSize recordings to keep (or less).
        auto reverseIterator = archiveMap.rbegin();

        uint64_t nbRecordings = 0;
        while (nbRecordings < this->params.archiveSize &&
            reverseIterator != archiveMap.rend()) {
            nbRecordings += reverseIterator->second->getNbRecordings();
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
            while (recordingIdx < reverseIterator->second->getNbRecordings()) {
                // Access in reverse order
                const ArchiveRecording& recording =
                    reverseIterator->second->at(recordingIdx);
                // forced Insertion
                this->archive->addRecording(
                    recording.agent,
                    reverseIterator->second->getDataHandlers().at(
                        recording.dataHash),
                    recording.result, true);
                recordingIdx++;

            }
        }

        // delete all archives
        reverseIterator = archiveMap.rbegin();
        while (reverseIterator != archiveMap.rend()) {
            delete reverseIterator->second;
            reverseIterator++;
        }
    }
}