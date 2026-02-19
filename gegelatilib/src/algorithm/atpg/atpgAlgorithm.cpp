
#include "algorithm/atpg/atpgAlgorithm.h"


std::unique_ptr<Algorithm::Algorithm> Algorithm::ATPG::ATPGAlgorithm::copy() const
{
    return std::make_unique<ATPGAlgorithm>(
        this->params, 
        this->cGetSubAlgorithm(this->programAlgorithmID), 
        this->cGetSubAlgorithm(this->actionProgramAlgorithmID), 
        this->algorithmName
    );
}

void Algorithm::ATPG::ATPGAlgorithm::setActionProgramAlgorithm(const Algorithm& actionProgramAlgorithm)
{
    Algorithm::Algorithm::addSubAlgorithm(actionProgramAlgorithm);

    // Set action program algorithm name
    this->actionProgramAlgorithmID = this->subAlgorithms.back()->getAlgorithmID();
}

void Algorithm::ATPG::ATPGAlgorithm::addAggregatedActionProgramAlgorithm(const Algorithm& programAlgorithm)
{
    // Get existing action program algorithm
    Algorithm& actionProgramAlgo = this->getSubAlgorithm(this->actionProgramAlgorithmID);

    // Set action program algorithm name
    actionProgramAlgo.addAggregatedAlgorithm(programAlgorithm);
}


void Algorithm::ATPG::ATPGAlgorithm::initManager()
{
    this->manager = std::make_unique<ATPG::ATPGManager>(*this->outputs, this->algorithmID);
}

void Algorithm::ATPG::ATPGAlgorithm::initMutator()
{
    this->mutator = std::make_unique<ATPG::ATPGMutator>(*this->selector, this->algorithmID, *this->archive);
}

std::map<uint64_t, std::set<std::reference_wrapper<const Algorithm::Agent>>> Algorithm::ATPG::ATPGAlgorithm::getUsedSubAgents() const
{
    std::map<uint64_t, std::set<std::reference_wrapper<const Agent>>> usedSubAgents = TPGAlgorithm::getUsedSubAgents();
    usedSubAgents[this->actionProgramAlgorithmID] = std::set<std::reference_wrapper<const Agent>>();

    for(const EvoGraph::Vertex& vertex: this->graph->getVertices()){
        if(vertex.hasProgram() && vertex.getProgram().getAlgorithmID() == this->actionProgramAlgorithmID){
            usedSubAgents[this->actionProgramAlgorithmID].insert(vertex.getProgram());
        }
    }

    return usedSubAgents;
}



void Algorithm::ATPG::ATPGAlgorithm::initSubAlgorithms(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{
    // Init context program algo with TPG method.
    TPG::TPGAlgorithm::initSubAlgorithms(rng, outputs, dataSource, graph);

    // Initialize action program algorithm.
    Algorithm& actionProgramAlgo = this->getSubAlgorithm(this->actionProgramAlgorithmID);
    actionProgramAlgo.initAlgorithm(rng, outputs, dataSource, graph);

    // Add program manager and mutator to TPG manager and mutator
    this->manager->addSubManager(actionProgramAlgo.getManager());
    ATPG::ATPGManager* atpgManager = dynamic_cast<ATPGManager*>(this->manager.get());
    atpgManager->setActionProgramAlgorithmID(this->actionProgramAlgorithmID);

    this->mutator->addSubMutator(actionProgramAlgo.getMutator());
    ATPG::ATPGMutator* atpgMutator = dynamic_cast<ATPGMutator*>(this->mutator.get());
    atpgMutator->setActionProgramAlgorithmID(this->actionProgramAlgorithmID);
}


std::shared_ptr<Algorithm::PolicyStats> Algorithm::ATPG::ATPGAlgorithm::createPolicyStats() const
{
    std::map<uint64_t, std::shared_ptr<PolicyStats>> subPolicyStatsMap;
    subPolicyStatsMap[this->programAlgorithmID] = this->cGetSubAlgorithm(this->programAlgorithmID).createPolicyStats();
    subPolicyStatsMap[this->actionProgramAlgorithmID] = this->cGetSubAlgorithm(this->actionProgramAlgorithmID).createPolicyStats();
    return std::make_shared<ATPGPolicyStats>(this->algorithmName, this->algorithmID, subPolicyStatsMap);
}