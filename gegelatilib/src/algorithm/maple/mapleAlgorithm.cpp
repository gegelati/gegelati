
#include "algorithm/maple/mapleAlgorithm.h"

std::unique_ptr<Algorithm::Algorithm> Algorithm::Maple::MapleAlgorithm::copy() const
{
    return std::make_unique<MapleAlgorithm>(this->params, this->cGetSubAlgorithm(this->programAlgorithmID), this->algorithmName);
}

void Algorithm::Maple::MapleAlgorithm::initManager()
{
    this->manager = std::make_unique<Maple::MapleManager>(*this->outputs, this->algorithmID);
}

void Algorithm::Maple::MapleAlgorithm::initMutator()
{
    this->mutator = std::make_unique<Maple::MapleMutator>(*this->selector, this->algorithmID, *this->archive);
}

void Algorithm::Maple::MapleAlgorithm::initSubAlgorithms(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{
    // Initialize program algorithm.
    Algorithm& programAlgo = this->getSubAlgorithm(this->programAlgorithmID);

    // Program output is only size 1, except for continuous outputs where we create more outputs (one per continuous output of the TPG)
    auto programOutput = std::make_shared<Output::OutputHandler>(Output::Output());
    // Init program algorithm
    programAlgo.initAlgorithm(rng, *programOutput, dataSource, graph);

    // Add program manager and mutator to TPG manager and mutator
    this->manager->addSubManager(programAlgo.getManager());
    TPG::TPGManager* tpgManager = dynamic_cast<TPG::TPGManager*>(this->manager.get());
    tpgManager->setProgramAlgorithmID(this->programAlgorithmID);

    this->mutator->addSubMutator(programAlgo.getMutator());
    TPG::TPGMutator* tpgMutator = dynamic_cast<TPG::TPGMutator*>(this->mutator.get());
    tpgMutator->setProgramAlgorithmID(this->programAlgorithmID);
}