
#include "algorithm/maple/mapleAlgorithm.h"

std::unique_ptr<Algorithm::Algorithm> Algorithm::Maple::MapleAlgorithm::copy() const
{
    return std::make_unique<MapleAlgorithm>(this->params, this->cGetSubAlgorithm(this->programAlgorithmName), this->algorithmName + "_copy");
}

void Algorithm::Maple::MapleAlgorithm::initManager(std::shared_ptr<const Output::OutputHandler> outputs)
{
    this->manager = std::make_shared<Maple::MapleManager>(*outputs);
    this->manager->setAlgorithmName(algorithmName);
}

void Algorithm::Maple::MapleAlgorithm::initMutator()
{
    this->mutator = std::make_shared<Maple::MapleMutator>(*this->selector, this->archive);
    this->mutator->setAlgorithmName(algorithmName);
}