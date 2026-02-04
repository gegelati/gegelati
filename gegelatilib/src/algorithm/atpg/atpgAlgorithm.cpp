
#include "algorithm/atpg/atpgAlgorithm.h"


std::unique_ptr<Algorithm::Algorithm> Algorithm::ATPG::ATPGAlgorithm::copy() const
{
    return std::make_unique<ATPGAlgorithm>(
        this->params, 
        this->cGetSubAlgorithm(this->programAlgorithmName), 
        this->cGetSubAlgorithm(this->actionProgramAlgorithmName), 
        this->algorithmName + "_copy"
    );
}

void Algorithm::ATPG::ATPGAlgorithm::setActionProgramAlgorithm(const Algorithm& actionProgramAlgorithm)
{
    Algorithm::Algorithm::addSubAlgorithm(actionProgramAlgorithm);

    // Set action program algorithm name
    this->actionProgramAlgorithmName = this->subAlgorithms.back()->getAlgorithmName();
}

void Algorithm::ATPG::ATPGAlgorithm::addAggregatedActionProgramAlgorithm(const Algorithm& programAlgorithm)
{
    // Get existing action program algorithm
    Algorithm& actionProgramAlgo = this->getSubAlgorithm(this->actionProgramAlgorithmName);

    // Set action program algorithm name
    actionProgramAlgo.addAggregatedAlgorithm(programAlgorithm);
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
    Algorithm& actionProgramAlgo = this->getSubAlgorithm(this->actionProgramAlgorithmName);
    actionProgramAlgo.initAlgorithm(rng, outputs, dataSource, graph);

    // Add program manager and mutator to TPG manager and mutator
    this->manager->addSubManager(actionProgramAlgo.getManager());
    std::shared_ptr<ATPG::ATPGManager> tpgManager = std::dynamic_pointer_cast<ATPG::ATPGManager>(this->manager);
    tpgManager->setActionProgramAlgorithmName(this->actionProgramAlgorithmName);

    this->mutator->addSubMutator(actionProgramAlgo.getMutator());
    std::shared_ptr<ATPG::ATPGMutator> tpgMutator = std::dynamic_pointer_cast<ATPG::ATPGMutator>(this->mutator);
    tpgMutator->setActionProgramAlgorithmName(this->actionProgramAlgorithmName);
}
