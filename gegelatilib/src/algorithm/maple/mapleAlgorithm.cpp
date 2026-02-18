
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