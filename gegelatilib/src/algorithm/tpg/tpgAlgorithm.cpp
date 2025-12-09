
#include "algorithm/tpg/tpgAlgorithm.h"


std::vector<double> Algorithm::TPG::TPGAlgorithm::executeAgent(std::shared_ptr<const Agent> agent) const
{
    
}

void Algorithm::TPG::TPGAlgorithm::addLGPAlgorithm(std::shared_ptr<EvoGraph::Graph> graph, const Learn::LearningParameters& params, size_t nbOutputs, const Instructions::Set& iSet, std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources)
{
    std::shared_ptr<LGP::LGPAlgorithm> lgpAlgorithm =
        std::make_shared<LGP::LGPAlgorithm>(graph, params, nbOutputs, iSet, dataSources);

    Algorithm::Algorithm::addSubAlgorithm(lgpAlgorithm);

    // Set program algorithm name
    this->programAlgorithmName = lgpAlgorithm->getAlgorithmName();
    std::shared_ptr<TPG::TPGMutator> tpgMutator = std::dynamic_pointer_cast<TPG::TPGMutator>(this->mutator);
    tpgMutator->setProgramAlgorithmName(lgpAlgorithm->getAlgorithmName());

}


std::shared_ptr<Algorithm::ExecutionEngine> Algorithm::TPG::TPGAlgorithm::createExecutionEngine()
{
    auto engine = std::make_shared<TPG::TPGExecutionEngine>(this->algorithmName);

    std::dynamic_pointer_cast<TPG::TPGExecutionEngine>(engine)->setProgramExecutionEngine(
        this->getSubAlgorithm(this->programAlgorithmName)->createExecutionEngine()
    );

    return engine;
}