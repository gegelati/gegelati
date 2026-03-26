
#include "algorithm/cgp/cgpManager.h"

Algorithm::CGP::CGPAgent& Algorithm::CGP::CGPManager::getCGPAgentFromCst(const Agent& agent)
{
    return dynamic_cast<CGPAgent&>(**this->getAgentFromCst(agent));
}

const Algorithm::Agent& Algorithm::CGP::CGPManager::createAgent(EvoGraph::Graph& graph)
{
    size_t nbLayers = 3;
    size_t nbNodesPerLayer = 5;
    this->agents.insert(std::make_unique<CGPAgent>(this->env, nbLayers, nbNodesPerLayer, this->outputs, this->getAlgorithmID()));
    return **this->agents.rbegin();
}

const Algorithm::Agent& Algorithm::CGP::CGPManager::copyAgent(const Agent& agent, EvoGraph::Graph& graph)
{
    const CGPAgent* castedAgent = dynamic_cast<const CGPAgent*>(&agent);
    if(castedAgent == nullptr){
        throw std::runtime_error("Algorithm::CGP::CGPManager::copyAgent: trying to copy an agent that is not a CGPAgent.");
    }
    CGPAgent& newAgent = this->getCGPAgentFromCst(this->createAgent(graph));

    for(size_t idx = 0; idx < castedAgent->getNbLines(); idx++){
        newAgent.addNewLine(castedAgent->getLine(idx));
    }

    for(size_t idx = 0; idx < castedAgent->getEnvironment().getParams().nbProgramConstant; idx++){
        this->setConstantAt(newAgent, idx, castedAgent->getConstantAt(idx));
    }

    this->identifyIntrons(newAgent);
    return **this->agents.rbegin();
}

void Algorithm::CGP::CGPManager::emptyAgent(const Agent& agent, EvoGraph::Graph& graph)
{
    Algorithm::CGP::CGPAgent& cgpAgent = this->getCGPAgentFromCst(agent);
    while (cgpAgent.getNbLines() > 0) {
        cgpAgent.removeLine(0);
    }
}