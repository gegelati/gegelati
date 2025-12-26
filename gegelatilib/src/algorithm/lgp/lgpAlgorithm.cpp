
#include "algorithm/lgp/lgpAlgorithm.h"


std::vector<double> Algorithm::LGP::LGPAlgorithm::executeAgent(std::shared_ptr<const Agent> agent) const
{
    return {};
}



 std::shared_ptr<const Environment> Algorithm::LGP::LGPAlgorithm::getEnvironment() const
{
    return this->env;
}


void Algorithm::LGP::LGPAlgorithm::initAlgorithm(RNG::RNG& rng, std::shared_ptr<const Output::OutputHandler> outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{
    this->env = std::make_shared<Environment>(iSet, params, dataSource);

    this->mutator = std::make_shared<LGPMutator>();
    
    this->manager = std::make_shared<LGPManager>(this->env, *outputs);

    this->selector = Selector::selectorFactory(this->manager, this->params);

    Algorithm::Algorithm::initAlgorithm(rng, outputs, dataSource, graph);

}