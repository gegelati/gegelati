
#include "algorithm/tpg/tpgAlgorithm.h"


std::vector<double> Algorithm::TPG::TPGAlgorithm::executeAgent(std::shared_ptr<const Agent> agent) const
{
    return {};
}

void Algorithm::TPG::TPGAlgorithm::addLGPAlgorithm(const Learn::LearningParameters& params, size_t nbOutputs, const Instructions::Set& iSet)
{
    std::shared_ptr<LGP::LGPAlgorithm> lgpAlgorithm =
        std::make_shared<LGP::LGPAlgorithm>(params, nbOutputs, iSet, this->algorithmName + "_LGP_Program");
    Algorithm::Algorithm::addSubAlgorithm(lgpAlgorithm);

    // Set program algorithm name
    this->programAlgorithmName = lgpAlgorithm->getAlgorithmName();

}


std::shared_ptr<const Archive> Algorithm::TPG::TPGAlgorithm::getArchive() const
{
    return this->archive;
}


void Algorithm::TPG::TPGAlgorithm::init(RNG::RNG& rng, Learn::LearningEnvironment& le, std::shared_ptr<EvoGraph::Graph> graph)
{
    if(programAlgorithmName.empty()){
        throw std::runtime_error("TPGAlgorithm::init: No program algorithm associated with the TPG agents.");
    }

    this->mutator = std::make_shared<TPG::TPGMutator>(this->archive);
    std::shared_ptr<TPG::TPGMutator> tpgMutator = std::dynamic_pointer_cast<TPG::TPGMutator>(this->mutator);
    tpgMutator->setProgramAlgorithmName(this->programAlgorithmName);

    this->manager = std::make_shared<TPG::TPGManager>(this->nbOutputs);
    std::shared_ptr<TPG::TPGManager> tpgManager = std::dynamic_pointer_cast<TPG::TPGManager>(this->manager);
    tpgManager->setProgramAlgorithmName(this->programAlgorithmName);


    Algorithm::Algorithm::init(rng, le, graph);
}