
#include "algorithm/cgp/cgpAlgorithm.h"



std::unique_ptr<Algorithm::Algorithm> Algorithm::CGP::CGPAlgorithm::copy() const
{
    return std::make_unique<CGPAlgorithm>(this->params, this->iSet, this->algorithmName);
}
void Algorithm::CGP::CGPAlgorithm::initMutator()
{
    this->mutator = std::make_unique<CGP::CGPMutator>(*this->selector, this->algorithmID);
}


void Algorithm::CGP::CGPAlgorithm::initAlgorithm(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{
    params.nbRegisters = 3 * 5;
    this->env = std::make_unique<Environment>(iSet, params, dataSource);
    Algorithm::Algorithm::initAlgorithm(rng, outputs, dataSource, graph);
}


