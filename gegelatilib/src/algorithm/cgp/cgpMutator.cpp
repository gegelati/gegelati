
#include <array>
#include "algorithm/cgp/cgpMutator.h"

bool Algorithm::CGP::CGPMutator::isConfigurationValid(const Learn::LearningParameters& params, const Output::OutputHandler& outputs) const
{
    if(outputs.sizeContinuous() != 0 && outputs.sizeDiscrete() != 0){
        throw std::runtime_error("CGPMutator::initRandomPopulation: CGP does not support mixed discrete and continuous outputs.");
    } else if (outputs.sizeContinuous() != 0){

        
    } else if (outputs.sizeDiscrete() != 0){
        if(outputs.size() > params.nbRegisters){
            throw std::runtime_error("CGPMutator::initRandomPopulation: Number of discrete outputs exceeds the number of registers.");
        }
    } else {
        throw std::runtime_error("CGPMutator::initRandomPopulation: No outputs defined.");
    }
    return true;
}

void Algorithm::CGP::CGPMutator::initRandomSpecificAgent(const Agent& agent, EvoGraph::Graph& graph, AgentManager& manager, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    // If first agent, check validity
    if(manager.getAgents().size() == 1){
        this->isConfigurationValid(params, manager.getOutputs());
    }

    manager.emptyAgent(agent, graph);

    CGPManager& cgpManager = dynamic_cast<CGPManager&>(manager);
    if(&cgpManager == nullptr){
        throw std::invalid_argument("CGPMutator::initRandomAgent: the given manager is not a CGPManager.");
    }

    const CGPAgent& cgpAgent = dynamic_cast<const CGPAgent&>(agent);
    if(&cgpAgent == nullptr){
        throw std::invalid_argument("CGPMutator::initRandomAgent: the created agent is not a CGPAgent.");
    }

    // insert random constants in the program
    Data::Constant c_value;
    for (int i = 0; i < cgpAgent.getEnvironment().getParams().nbProgramConstant; i++) {
        c_value = {rng.getDouble(params.mutation.prog.minConstValue,
                                 params.mutation.prog.maxConstValue)};
        cgpManager.setConstantAt(agent, i, c_value);
    }

    // Select the number of line randomly
    const uint64_t nbLine = this->sizeLayer * this->nbLayer;
    // Insert them
    while (cgpAgent.getNbLines() < nbLine) {
        this->insertRandomLine(cgpAgent, cgpManager, rng);
    }

    // Identify Introns
    cgpManager.identifyIntrons(agent);
}


void Algorithm::CGP::CGPMutator::insertRandomLine(const LGP::LGPAgent& agent, LGP::LGPManager& manager,  RNG::RNG& rng)
{
    uint64_t lineIndex = agent.getNbLines();
    manager.addNewLine(agent, lineIndex);
    
    size_t maxIndex = sizeLayer * (lineIndex / sizeLayer);
    this->cgpLineMutator.initRandomCorrectLine(manager.getLineForMutation(agent, lineIndex), lineIndex, maxIndex, rng);
}

void Algorithm::CGP::CGPMutator::crossoverAgents(
    std::array<std::reference_wrapper<const Agent>, 2> agents, EvoGraph::Graph& graph, 
    AgentManager& manager, std::vector<std::reference_wrapper<const Agent>>& newSubAgents, 
    const Learn::LearningParameters& params, RNG::RNG& rng)
{ 
    /// No crossover with CGP
}

void Algorithm::CGP::CGPMutator::mutateAgent(
    const Agent& agent, EvoGraph::Graph& graph, AgentManager& manager, std::vector<std::reference_wrapper<const Agent>>& newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    CGPManager& cgpManager = dynamic_cast<CGPManager&>(manager);
    if(&cgpManager == nullptr){
        throw std::invalid_argument("CGPMutator::initRandomAgent: the given manager is not a CGPManager.");
    }

    const CGPAgent& cgpAgent = dynamic_cast<const CGPAgent&>(agent);
    if(&cgpAgent == nullptr){
        throw std::invalid_argument("CGPMutator::initRandomAgent: the created agent is not a CGPAgent.");
    }

    // Mutate until a mutation happen
    while (!this->mutateCGPAgent(cgpAgent, cgpManager, params, rng));
}

bool Algorithm::CGP::CGPMutator::mutateCGPAgent(const CGPAgent& agent, CGPManager& manager, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    bool anyMutation = false;
    if (rng.getDouble(0.0, 1.0) < params.mutation.prog.pMutate) {
        anyMutation = true;
        alterRandomLine(agent, manager, rng);
    }

    // mutate the programs constants if they exists
    if (agent.getEnvironment().getParams().nbProgramConstant > 0 &&
        rng.getDouble(0.0, 1.0) < params.mutation.prog.pConstantMutation) {
        anyMutation = true;
        alterRandomConstant(agent, manager, params, rng);
    }

    for(size_t idx = 0; idx < manager.getOutputs().size(); idx++) {
        if(rng.getDouble(0.0, 1.0) < 1) {//params.mutation.prog.pMutateOutputs) {
            alterRandomOutputs(agent, manager, idx, rng);
        }
    }

    // Identify introns
    if (anyMutation) {
        manager.identifyIntrons(agent);
    }
    return anyMutation;
}

bool Algorithm::CGP::CGPMutator::alterRandomLine(const CGPAgent& agent, CGPManager& manager, 
                                              RNG::RNG& rng)
{
    if (agent.getNbLines() < 1) {
        return false;
    }
    // Select a random index.
    const uint64_t lineIndex = rng.getUnsignedInt64(0, agent.getNbLines() - 1);
    
    size_t maxIndex = sizeLayer * (lineIndex / sizeLayer);
    this->cgpLineMutator.alterCorrectLine(manager.getLineForMutation(agent, lineIndex), maxIndex, rng); // specified accessible registers
    return true;
}