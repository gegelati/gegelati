
#include "algorithm/atpg/atpgAlgorithm.h"


void Algorithm::ATPG::ATPGAlgorithm::setActionProgramAlgorithm(std::shared_ptr<Algorithm> actionProgramAlgorithm)
{
    Algorithm::Algorithm::addSubAlgorithm(actionProgramAlgorithm);

    // Set action program algorithm name
    this->actionProgramAlgorithmName = actionProgramAlgorithm->getAlgorithmName();
}

void Algorithm::ATPG::ATPGAlgorithm::initAlgorithm(RNG::RNG& rng, std::shared_ptr<const Output::OutputHandler> outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{

    this->outputs = outputs;
    std::shared_ptr<Algorithm> contextProgramAlgo = this->getSubAlgorithm(this->programAlgorithmName);
    std::shared_ptr<Algorithm> actionProgramAlgo = this->getSubAlgorithm(this->actionProgramAlgorithmName);

    /*this->mutator = std::make_shared<ATPG::ATPGMutator>(this->archive);
    std::shared_ptr<ATPG::ATPGMutator> tpgMutator = std::dynamic_pointer_cast<ATPG::ATPGMutator>(this->mutator);
    tpgMutator->setProgramAlgorithmName(this->programAlgorithmName);*/

    this->manager = std::make_shared<ATPG::ATPGManager>(*outputs, *this->archive);
    std::shared_ptr<ATPG::ATPGManager> tpgManager = std::dynamic_pointer_cast<ATPG::ATPGManager>(this->manager);
    tpgManager->setProgramAlgorithmName(this->programAlgorithmName);


    this->graph = graph;

    // Set the algorithm name to the components
    this->manager->setAlgorithmName(algorithmName);
    this->mutator->setAlgorithmName(algorithmName);
    this->selector = Selector::selectorFactory(this->manager, this->params);

    // Initialize context program algorithm.
    auto programOutput = std::make_shared<Output::OutputHandler>(Output::Output());
    contextProgramAlgo->initAlgorithm(rng, programOutput, dataSource, graph);
    this->manager->addSubManager(contextProgramAlgo->getManager());
    this->mutator->addSubMutator(contextProgramAlgo->getMutator());

    // Initialize action program algorithm
    actionProgramAlgo->initAlgorithm(rng, outputs, dataSource, graph);
    this->manager->addSubManager(actionProgramAlgo->getManager());
    this->mutator->addSubMutator(actionProgramAlgo->getMutator());

    // Clear the best agent in the selector
    this->selector->forgetPreviousResults();
}
