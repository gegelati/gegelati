
#include "algorithm/maple/mapleAlgorithm.h"


void Algorithm::Maple::MapleAlgorithm::initAlgorithm(RNG::RNG& rng, std::shared_ptr<const Output::OutputHandler> outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{

    this->outputs = outputs;
    if(programAlgorithmName.empty()){
        throw std::runtime_error("MapleAlgorithm::init: No program algorithm associated with the Maple agents.");
    }

    this->mutator = std::make_shared<Maple::MapleMutator>(this->archive);
    std::shared_ptr<Maple::MapleMutator> tpgMutator = std::dynamic_pointer_cast<Maple::MapleMutator>(this->mutator);
    tpgMutator->setProgramAlgorithmName(this->programAlgorithmName);

    this->manager = std::make_shared<Maple::MapleManager>(*outputs, *this->archive);
    std::shared_ptr<Maple::MapleManager> tpgManager = std::dynamic_pointer_cast<Maple::MapleManager>(this->manager);
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