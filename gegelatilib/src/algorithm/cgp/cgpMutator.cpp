
#include <array>
#include "algorithm/cgp/cgpMutator.h"

bool Algorithm::CGP::CGPMutator::isConfigurationValid(const AlgorithmParameters& params, const Output::OutputHandler& outputs) const
{
    if(outputs.sizeContinuous() != 0 && outputs.sizeDiscrete() != 0){
        throw std::runtime_error("CGPMutator::initRandomPopulation: CGP does not support mixed discrete and continuous outputs.");
    } else if (outputs.sizeContinuous() == 0 && outputs.sizeDiscrete() == 0) {
        throw std::runtime_error("CGPMutator::initRandomPopulation: No outputs defined.");
    }
    return true;
}

void Algorithm::CGP::CGPMutator::initRandomSpecificAgent(const Agent& agent, EvoGraph::Graph& graph, AgentManager& manager, const AlgorithmParameters& params, RNG::RNG& rng)
{
    // If first agent, check validity
    if(manager.getAgents().size() == 1){
        this->isConfigurationValid(params, manager.getOutputs());
    }

    manager.emptyAgent(agent, graph);

    LGP::LGPManager& lgpManager = dynamic_cast<LGP::LGPManager&>(manager);
    if(&lgpManager == nullptr){
        throw std::invalid_argument("CGPMutator::initRandomAgent: the given manager is not a LGPManager.");
    }

    const LGP::LGPAgent& lgpAgent = dynamic_cast<const LGP::LGPAgent&>(agent);
    if(&lgpAgent == nullptr){
        throw std::invalid_argument("CGPMutator::initRandomAgent: the created agent is not a CGPAgent.");
    }

    // insert random constants in the program
    Data::Constant c_value;
    for (int i = 0; i < params.lgp.nbProgramConstant; i++) {
        c_value = {rng.getDouble(params.lgp.minConstValue,
                                 params.lgp.maxConstValue)};
        lgpManager.setConstantAt(agent, i, c_value);
    }

    // Compute the number of nodes
    const uint64_t nbLine = params.cgp.nbNodesPerLayer * params.cgp.nbLayers;
    // Insert them
    while (lgpAgent.getNbLines() < nbLine) {
        this->insertRandomLine(lgpAgent, lgpManager, params, rng);
    }

    for(size_t idx = 0; idx < lgpAgent.getOutputIndices().size(); idx++) {
        lgpManager.setOutputIndex(lgpAgent, nbLine - 1 - idx, idx);
    }

    // Identify Introns
    lgpManager.identifyIntrons(agent);
}

void Algorithm::CGP::CGPMutator::insertRandomLine(const LGP::LGPAgent& agent, LGP::LGPManager& manager, const AlgorithmParameters& params, RNG::RNG& rng)
{
    uint64_t lineIndex = agent.getNbLines();
    manager.addNewLine(agent, lineIndex);
    
    size_t maxIndex = params.cgp.nbNodesPerLayer * (lineIndex / params.cgp.nbNodesPerLayer);
    this->cgpLineMutator.initRandomCorrectLine(manager.getLineForMutation(agent, lineIndex), lineIndex, maxIndex, rng);
}

void Algorithm::CGP::CGPMutator::crossoverAgents(
    std::array<std::reference_wrapper<const Agent>, 2> agents, EvoGraph::Graph& graph, 
    AgentManager& manager, std::vector<std::reference_wrapper<const Agent>>& newSubAgents, 
    const AlgorithmParameters& params, RNG::RNG& rng)
{ 
    /// No crossover with CGP
}

bool Algorithm::CGP::CGPMutator::mutateLGPAgent(const LGP::LGPAgent& agent, LGP::LGPManager& manager, const AlgorithmParameters& params, RNG::RNG& rng)
{
    bool anyMutation = false;
    for(size_t idx = 0; idx < params.cgp.nbLayers * params.cgp.nbNodesPerLayer; idx++) {
        if (rng.getDouble(0.0, 1.0) < params.cgp.pMutateNode) {
            anyMutation = true;
            alterRandomlyLine(agent, idx, manager, params, rng);
        }
    }

    // mutate the programs constants if they exists
    if (params.lgp.nbProgramConstant > 0 &&
        rng.getDouble(0.0, 1.0) < params.lgp.pConstantMutation) {
        anyMutation = true;
        alterRandomConstant(agent, manager, params, rng);
    }

    for(size_t idx = 0; idx < agent.getUsedNbOutputs(manager.getOutputs()); idx++) {
        if(rng.getDouble(0.0, 1.0) < params.lgp.pMutateOutput) {
            anyMutation = true;
            alterRandomOutputs(agent, manager, idx, params, rng);
        }
    }

    // Identify introns
    if (anyMutation) {
        manager.identifyIntrons(agent);
    }
    return anyMutation;
}

bool Algorithm::CGP::CGPMutator::alterRandomlyLine(
    const LGP::LGPAgent& agent, size_t lineIndex, LGP::LGPManager& manager, const AlgorithmParameters& params, RNG::RNG& rng)
{
    
    size_t maxIndex = params.cgp.nbNodesPerLayer * (lineIndex / params.cgp.nbNodesPerLayer);
    this->cgpLineMutator.alterCorrectLine(manager.getLineForMutation(agent, lineIndex), maxIndex, rng); // specified accessible registers
    return true;
}