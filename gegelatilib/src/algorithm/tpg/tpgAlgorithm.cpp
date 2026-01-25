
#include "algorithm/tpg/tpgAlgorithm.h"


void Algorithm::TPG::TPGAlgorithm::addLGPAlgorithm(const Learn::LearningParameters& params, const Instructions::Set& iSet)
{
    std::shared_ptr<LGP::LGPAlgorithm> lgpAlgorithm =
        std::make_shared<LGP::LGPAlgorithm>(params, iSet, this->algorithmName + "_LGP_Program");
    Algorithm::Algorithm::addSubAlgorithm(lgpAlgorithm);

    // Set program algorithm name
    this->programAlgorithmName = lgpAlgorithm->getAlgorithmName();
}


std::shared_ptr<const Archive> Algorithm::TPG::TPGAlgorithm::getArchive() const
{
    return this->archive;
}


void Algorithm::TPG::TPGAlgorithm::initAlgorithm(RNG::RNG& rng, std::shared_ptr<const Output::OutputHandler> outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{

    this->outputs = outputs;
    if(programAlgorithmName.empty()){
        throw std::runtime_error("TPGAlgorithm::init: No program algorithm associated with the TPG agents.");
    }

    this->mutator = std::make_shared<TPG::TPGMutator>(this->archive);
    std::shared_ptr<TPG::TPGMutator> tpgMutator = std::dynamic_pointer_cast<TPG::TPGMutator>(this->mutator);
    tpgMutator->setProgramAlgorithmName(this->programAlgorithmName);

    this->manager = std::make_shared<TPG::TPGManager>(*outputs, *this->archive);
    std::shared_ptr<TPG::TPGManager> tpgManager = std::dynamic_pointer_cast<TPG::TPGManager>(this->manager);
    tpgManager->setProgramAlgorithmName(this->programAlgorithmName);


    this->graph = graph;

    // Set the algorithm name to the components
    this->manager->setAlgorithmName(algorithmName);
    this->mutator->setAlgorithmName(algorithmName);
    this->selector = Selector::selectorFactory(this->manager, this->params);

    // Initialize program algorithm.
    auto& programAlgo = this->subAlgorithms.front();
    auto programOutput = std::make_shared<Output::OutputHandler>(Output::Output());

    for(size_t idx = 0; idx < this->outputs->sizeContinuous(); idx++){
        programOutput->addOutput(Output::Output());
    }

    programAlgo->initAlgorithm(rng, programOutput, dataSource, graph);
    this->manager->addSubManager(programAlgo->getManager());
    this->mutator->addSubMutator(programAlgo->getMutator());

    // Clear the best agent in the selector
    this->selector->forgetPreviousResults();
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