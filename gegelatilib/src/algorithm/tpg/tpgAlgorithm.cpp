
#include "algorithm/tpg/tpgAlgorithm.h"


std::vector<double> Algorithm::TPG::TPGAlgorithm::executeAgent(std::shared_ptr<const Agent> agent) const
{
    return {};
}

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

    for(auto continuousOutput: this->outputs->getContinuousOutputs()){
        programOutput->addOutput(continuousOutput);
    }

    programAlgo->initAlgorithm(rng, programOutput, dataSource, graph);
    this->manager->addSubManager(programAlgo->getManager());
    this->mutator->addSubMutator(programAlgo->getMutator());

    // Clear the best agent in the selector
    this->selector->forgetPreviousResults();
}


std::shared_ptr<Algorithm::Job> Algorithm::TPG::TPGAlgorithm::createJob(std::shared_ptr<const Agent> agent, Learn::LearningMode mode,  RNG::RNG& rng, int idx) const
{
    if(agent == nullptr || !this->containsAgent(agent)){
        throw std::runtime_error("LearningAgent::makeJob: Cannot create a job with a null agent or an agent not belonging to this algorithm.");
    }

    // Before each agent evaluation, set a new seed for the archive in
    // TRAINING Mode Else, archiving should be deactivate anyway
    uint64_t archiveSeed = 0;
    if (mode ==Learn::LearningMode::TRAINING) {
        archiveSeed = rng.getUnsignedInt64(0, UINT64_MAX);
    }

    return std::make_shared<TPGJob>(agent, this->manager, this->selector, archiveSeed, idx);
}

void Algorithm::TPG::TPGAlgorithm::activeJob(Job& job)
{
    if(dynamic_cast<TPGJob*>(&job) != nullptr){
        this->archive->setRandomSeed(dynamic_cast<TPGJob*>(&job)->getArchiveSeed());
    } else {
        throw std::runtime_error("TPGAlgorithm::activeJob Job should be a TPGJob");
    }
}