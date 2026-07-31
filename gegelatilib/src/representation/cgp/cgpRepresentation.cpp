
#include "representation/cgp/cgpRepresentation.h"



std::unique_ptr<Representation::Representation> Representation::CGP::CGPRepresentation::copy() const
{
    return std::make_unique<CGPRepresentation>(this->iSet, 
        std::make_unique<RepresentationParameters>(*this->params), this->representationName, this->representationColor);
}
void Representation::CGP::CGPRepresentation::initMutator()
{
    this->mutator = std::make_unique<CGP::CGPMutator>(*this->selector, this->representationID);
}


void Representation::CGP::CGPRepresentation::initRepresentation(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{
    params->lgp.nbRegisters = params->cgp.nbLayers * params->cgp.nbNodesPerLayer;
    this->env = std::make_unique<LGP::LGPEnvironment>(iSet, params->lgp.nbRegisters, params->lgp.nbProgramConstant, dataSource);
    Representation::Representation::initRepresentation(rng, outputs, dataSource, graph);
}


