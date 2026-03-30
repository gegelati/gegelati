
#include "algorithm/tgp/tgpAlgorithm.h"



std::unique_ptr<Algorithm::Algorithm> Algorithm::TGP::TGPAlgorithm::copy() const
{
    return std::make_unique<TGPAlgorithm>(this->params, this->iSet, this->algorithmName);
}
void Algorithm::TGP::TGPAlgorithm::initMutator()
{
    this->mutator = std::make_unique<TGP::TGPMutator>(*this->selector, this->algorithmID);
}


void Algorithm::TGP::TGPAlgorithm::initAlgorithm(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{
    size_t maxDepth = 4;
    params.lgp.nbRegisters = std::pow(2, maxDepth) - 1;

    if(iSet.getMaxNbOperands() > 2) {
        throw std::runtime_error("TGPAlgorithm::initAlgorithm: Only support instruction with two operands (or one a sub graph might then be ignored)");
    }

    this->env = std::make_unique<LGP::LGPEnvironment>(iSet, params.lgp.nbRegisters, params.lgp.nbProgramConstant, dataSource);
    Algorithm::Algorithm::initAlgorithm(rng, outputs, dataSource, graph);
}


