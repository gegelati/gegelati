
#include <array>
#include "algorithm/tgp/tgpMutator.h"

bool Algorithm::TGP::TGPMutator::isConfigurationValid(const Learn::LearningParameters& params, const Output::OutputHandler& outputs) const
{

    if(outputs.sizeContinuous() != 0 && outputs.sizeDiscrete() != 0){
        throw std::runtime_error("TGPMutator::initRandomPopulation: TGP does not support mixed discrete and continuous outputs.");
    } else if (outputs.sizeContinuous() != 0){

        
    } else if (outputs.sizeDiscrete() != 0){
        if(outputs.size() > params.nbRegisters){
            throw std::runtime_error("TGPMutator::initRandomPopulation: Number of discrete outputs exceeds the number of registers.");
        }
    } else {
        throw std::runtime_error("TGPMutator::initRandomPopulation: No outputs defined.");
    }
    return true;
}

void Algorithm::TGP::TGPMutator::initRandomSpecificAgent(const Agent& agent, EvoGraph::Graph& graph, AgentManager& manager, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    // If first agent, check validity
    if(manager.getAgents().size() == 1){
        this->isConfigurationValid(params, manager.getOutputs());
    }

    manager.emptyAgent(agent, graph);

    LGP::LGPManager& lgpManager = dynamic_cast<LGP::LGPManager&>(manager);
    if(&lgpManager == nullptr){
        throw std::invalid_argument("TGPMutator::initRandomAgent: the given manager is not a LGPManager.");
    }

    const LGP::LGPAgent& lgpAgent = dynamic_cast<const LGP::LGPAgent&>(agent);
    if(&lgpAgent == nullptr){
        throw std::invalid_argument("TGPMutator::initRandomAgent: the created agent is not a TGPAgent.");
    }

    // insert random constants in the program
    Data::Constant c_value;
    for (int i = 0; i < lgpAgent.getEnvironment().getParams().nbProgramConstant; i++) {
        c_value = {rng.getDouble(params.mutation.prog.minConstValue,
                                 params.mutation.prog.maxConstValue)};
        lgpManager.setConstantAt(agent, i, c_value);
    }

    // Insert line, TGP will recursively call this method to create a whole TGP graph.
    this->insertRandomSubTree(lgpAgent, 0, lgpManager, rng);

    // Identify Introns
    lgpManager.identifyIntrons(agent);
}

void Algorithm::TGP::TGPMutator::insertRandomSubTree(const LGP::LGPAgent& agent, size_t destinationIndexLine, LGP::LGPManager& manager,  RNG::RNG& rng)
{
    std::vector<size_t> lineDestinationIndexToInsert;
    lineDestinationIndexToInsert.push_back(destinationIndexLine);

    while(lineDestinationIndexToInsert.size() > 0) {
        destinationIndexLine = lineDestinationIndexToInsert.front();
        lineDestinationIndexToInsert.erase(lineDestinationIndexToInsert.begin());

        size_t lineIndex = agent.getNbLines();        
        const LGP::LGPLine& line = manager.addNewLine(agent);
        
        // If current index is at max depth, don't allows the line to select registers.
        bool maxDepthReached = (std::floor(std::log2(destinationIndexLine + 1)) == this->maxDepth - 1);

        this->tgpLineMutator.initRandomCorrectLine(manager.getLineForMutation(agent, lineIndex), destinationIndexLine, maxDepthReached, rng);

        for(size_t idx = 0; idx < 2; idx++) {
            // Operand is a register
            if(line.getOperand(idx).first == 0) {
                lineDestinationIndexToInsert.push_back(line.getOperand(idx).second);
            }
        }
    }
}

void Algorithm::TGP::TGPMutator::crossoverAgents(
    std::array<std::reference_wrapper<const Agent>, 2> agents, EvoGraph::Graph& graph, 
    AgentManager& manager, std::vector<std::reference_wrapper<const Agent>>& newSubAgents, 
    const Learn::LearningParameters& params, RNG::RNG& rng)
{ 
    /// No crossover with TGP
}

bool Algorithm::TGP::TGPMutator::mutateLGPAgent(const LGP::LGPAgent& agent, LGP::LGPManager& manager, const Learn::LearningParameters& params, RNG::RNG& rng)
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

    /*for(size_t idx = 0; idx < manager.getOutputs().size(); idx++) {
        if(rng.getDouble(0.0, 1.0) < 1) {//params.mutation.prog.pMutateOutputs) {
            alterRandomOutputs(agent, manager, idx, rng);
        }
    }*/

    // Identify introns
    if (anyMutation) {
        manager.identifyIntrons(agent);
    }
    return anyMutation;
}

bool Algorithm::TGP::TGPMutator::alterRandomLine(const LGP::LGPAgent& agent, LGP::LGPManager& manager, 
                                              RNG::RNG& rng)
{
    if (agent.getNbLines() < 1) {
        return false;
    }
    // Select a random index.
    const uint64_t lineIndex = rng.getUnsignedInt64(0, agent.getNbLines() - 1);
    const LGP::LGPLine& line = agent.getLine(lineIndex);
    
    std::array<bool, 2> oldSubTree = {line.getOperand(0).first == 0, line.getOperand(1).first == 0};

    // If current index is at max depth, don't allows the line to select registers.
    bool maxDepthReached = (std::floor(std::log2(line.getDestinationIndex() + 1)) == this->maxDepth - 1);
    this->tgpLineMutator.alterCorrectLine(manager.getLineForMutation(agent, lineIndex), maxDepthReached, rng); // specified accessible registers

    if(!maxDepthReached) {
        std::array<bool, 2> newSubTree = {line.getOperand(0).first == 0, line.getOperand(1).first == 0};
        for(size_t idx = 0; idx < 2; idx++) {
            size_t idxSubTree = 2 * line.getDestinationIndex() + 1 + idx;
            if(oldSubTree[idx] && !newSubTree[idx]) {
                std::vector<size_t> destinationIdxToDestroy;
                destinationIdxToDestroy.push_back(idxSubTree);
                while(destinationIdxToDestroy.size() > 0) {
                    size_t destinationIdx = destinationIdxToDestroy.front();
                    destinationIdxToDestroy.erase(destinationIdxToDestroy.begin());

                    size_t idxLine = 0;
                    for(size_t idxLines = 0; idxLines < agent.getNbLines(); idxLines++) {
                        if(agent.getLine(idxLines).getDestinationIndex() == destinationIdx) {
                            idxLine = idxLines;
                        }
                    }

                    for(size_t idx = 0; idx < 2; idx++) {
                        // Operand is a register
                        const LGP::LGPLine& destroyedLine = agent.getLine(idxLine);
                        if(destroyedLine.getOperand(idx).first == 0) {
                            destinationIdxToDestroy.push_back(destroyedLine.getOperand(idx).second);
                        }
                    }
                    manager.removeLine(agent, idxLine);
                }
                    


                // Destroy old sub tree
            } else if(!oldSubTree[idx] && newSubTree[idx]) {
                // Create new sub tree
                this->insertRandomSubTree(agent, idxSubTree, manager, rng);
            }
        }
    }
    return true;
}