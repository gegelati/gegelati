
#include "algorithm/tgp/tgpAlgorithm.h"



std::unique_ptr<Algorithm::Algorithm> Algorithm::TGP::TGPAlgorithm::copy() const
{
    return std::make_unique<TGPAlgorithm>(this->iSet, std::make_unique<AlgorithmParameters>(*this->params), this->algorithmName, this->algorithmColor);
}
void Algorithm::TGP::TGPAlgorithm::initMutator()
{
    this->mutator = std::make_unique<TGP::TGPMutator>(*this->selector, this->algorithmID);
}


void Algorithm::TGP::TGPAlgorithm::initAlgorithm(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{
    size_t maxNbOperands = iSet.getMaxNbOperands();
    if(maxNbOperands > params->tgp.maxNbEdgePerNode) {
        throw std::runtime_error("TGPAlgorithm::initAlgorithm: Only support instruction with a reasonable number of operands (or one a sub graph might then be ignored)");
    } else if (maxNbOperands < params->tgp.maxNbEdgePerNode) {
        // If lower, set the param to the value for efficiency issue
        params->tgp.maxNbEdgePerNode = maxNbOperands;
    } 
    
    params->lgp.nbRegisters = std::pow(params->tgp.maxNbEdgePerNode, params->tgp.maxDepth) - 1;


    this->env = std::make_unique<LGP::LGPEnvironment>(iSet, params->lgp.nbRegisters, params->lgp.nbProgramConstant, dataSource);
    Algorithm::Algorithm::initAlgorithm(rng, outputs, dataSource, graph);
}


