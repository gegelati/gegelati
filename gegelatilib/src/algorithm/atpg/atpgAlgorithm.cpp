
#include "algorithm/atpg/atpgAlgorithm.h"


void Algorithm::ATPG::ATPGAlgorithm::setActionProgramAlgorithm(std::shared_ptr<Algorithm> actionProgramAlgorithm)
{
    Algorithm::Algorithm::addSubAlgorithm(actionProgramAlgorithm);

    // Set action program algorithm name
    this->actionProgramAlgorithmName = actionProgramAlgorithm->getAlgorithmName();
}


void Algorithm::ATPG::ATPGAlgorithm::initManager(std::shared_ptr<const Output::OutputHandler> outputs)
{
    this->manager = std::make_shared<ATPG::ATPGManager>(*outputs);
    this->manager->setAlgorithmName(algorithmName);
}

void Algorithm::ATPG::ATPGAlgorithm::initMutator()
{
    this->mutator = std::make_shared<ATPG::ATPGMutator>(*this->selector, this->archive);
    this->mutator->setAlgorithmName(algorithmName);
}

void Algorithm::ATPG::ATPGAlgorithm::initSubAlgorithms(RNG::RNG& rng, std::shared_ptr<const Output::OutputHandler> outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{
    // Init context program algo with TPG method.
    TPG::TPGAlgorithm::initSubAlgorithms(rng, outputs, dataSource, graph);



    // Initialize action program algorithm.
    std::shared_ptr<Algorithm> actionProgramAlgo = this->getSubAlgorithm(this->actionProgramAlgorithmName);
    actionProgramAlgo->initAlgorithm(rng, outputs, dataSource, graph);

    // Add program manager and mutator to TPG manager and mutator
    this->manager->addSubManager(actionProgramAlgo->getManager());
    std::shared_ptr<ATPG::ATPGManager> tpgManager = std::dynamic_pointer_cast<ATPG::ATPGManager>(this->manager);
    tpgManager->setActionProgramAlgorithmName(this->actionProgramAlgorithmName);

    this->mutator->addSubMutator(actionProgramAlgo->getMutator());
    std::shared_ptr<ATPG::ATPGMutator> tpgMutator = std::dynamic_pointer_cast<ATPG::ATPGMutator>(this->mutator);
    tpgMutator->setActionProgramAlgorithmName(this->actionProgramAlgorithmName);
}
