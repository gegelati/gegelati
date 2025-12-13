
#include "algorithm/lgp/lgpAlgorithm.h"


std::vector<double> Algorithm::LGP::LGPAlgorithm::executeAgent(std::shared_ptr<const Agent> agent) const
{
    return {};
}



 std::shared_ptr<const Environment> Algorithm::LGP::LGPAlgorithm::getEnvironment() const
{
    return this->env;
}

std::shared_ptr<const Archive> Algorithm::LGP::LGPAlgorithm::getArchive() const
{
    return this->archive;
}

std::unique_ptr<Algorithm::ExecutionEngine> Algorithm::LGP::LGPAlgorithm::createExecutionEngine() const
{
    return std::make_unique<LGPExecutionEngine>(*this->env, this->algorithmName);
}

void Algorithm::LGP::LGPAlgorithm::init(RNG::RNG& rng, Learn::LearningEnvironment& le, std::shared_ptr<EvoGraph::Graph> graph)
{
    this->archive = std::make_shared<Archive>(this->params.archiveSize, this->params.archivingProbability);
    this->env = std::make_shared<Environment>(iSet, params, le.getDataSources());

    this->mutator = std::make_shared<LGPMutator>();
    
    this->manager = std::make_shared<LGPManager>(this->archive, this->env, this->nbOutputs);

    this->selector = Selector::selectorFactory(this->manager, this->params);

    Algorithm::Algorithm::init(rng, le, graph);
}