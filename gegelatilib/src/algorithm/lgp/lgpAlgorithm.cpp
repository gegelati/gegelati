
#include "algorithm/lgp/lgpAlgorithm.h"




 std::shared_ptr<const Environment> Algorithm::LGP::LGPAlgorithm::getEnvironment() const
{
    return this->env;
}

void Algorithm::LGP::LGPAlgorithm::initManager(std::shared_ptr<const Output::OutputHandler> outputs)
{
    this->manager = std::make_shared<LGP::LGPManager>(this->env, *outputs);
    this->manager->setAlgorithmName(algorithmName);
}

void Algorithm::LGP::LGPAlgorithm::initMutator()
{
    this->mutator = std::make_shared<LGP::LGPMutator>(*this->selector);
    this->mutator->setAlgorithmName(algorithmName);
}


void Algorithm::LGP::LGPAlgorithm::initAlgorithm(RNG::RNG& rng, std::shared_ptr<const Output::OutputHandler> outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{
    this->env = std::make_shared<Environment>(iSet, params, dataSource);
    Algorithm::Algorithm::initAlgorithm(rng, outputs, dataSource, graph);
}