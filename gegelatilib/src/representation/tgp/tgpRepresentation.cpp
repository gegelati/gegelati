
#include "representation/tgp/tgpRepresentation.h"



std::unique_ptr<Representation::Representation> Representation::TGP::TGPRepresentation::copy() const
{
    return std::make_unique<TGPRepresentation>(this->iSet, std::make_unique<RepresentationParameters>(*this->params), this->representationName, this->representationColor);
}
void Representation::TGP::TGPRepresentation::initMutator()
{
    this->mutator = std::make_unique<TGP::TGPMutator>(*this->selector, this->representationID);
}


void Representation::TGP::TGPRepresentation::initRepresentation(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{
    size_t maxNbOperands = iSet.getMaxNbOperands();
    if(maxNbOperands > params->tgp.maxNbEdgePerNode) {
        throw std::runtime_error("TGPRepresentation::initRepresentation: Only support instruction with a reasonable number of operands (or one a sub graph might then be ignored)");
    } else if (maxNbOperands < params->tgp.maxNbEdgePerNode) {
        // If lower, set the param to the value for efficiency issue
        params->tgp.maxNbEdgePerNode = maxNbOperands;
    } 
    
    params->lgp.nbRegisters = std::pow(params->tgp.maxNbEdgePerNode, params->tgp.maxDepth) - 1;


    this->env = std::make_unique<LGP::LGPEnvironment>(iSet, params->lgp.nbRegisters, params->lgp.nbProgramConstant, dataSource);
    Representation::Representation::initRepresentation(rng, outputs, dataSource, graph);
}


