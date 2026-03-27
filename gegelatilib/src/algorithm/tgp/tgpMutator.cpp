
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
    this->insertRandomSubTree(lgpAgent, 0, this->maxInitDepth, lgpManager, rng);

    // Identify Introns
    lgpManager.identifyIntrons(agent);
}

void Algorithm::TGP::TGPMutator::insertRandomSubTree(const LGP::LGPAgent& agent, size_t destinationIndexLine, size_t maxDepthTree, LGP::LGPManager& manager,  RNG::RNG& rng)
{
    std::vector<size_t> lineDestinationIndexToInsert;
    lineDestinationIndexToInsert.push_back(destinationIndexLine);

    while(lineDestinationIndexToInsert.size() > 0) {
        destinationIndexLine = lineDestinationIndexToInsert.front();
        lineDestinationIndexToInsert.erase(lineDestinationIndexToInsert.begin());
        // Always add a line at index 0, because lines are inserted in the opposite order as the distribution
        size_t lineIndex = 0;        
        const LGP::LGPLine& line = manager.addNewLine(agent, 0);
        
        // If current index is at max depth, don't allows the line to select registers.
        
        bool maxDepthReached = (this->getNodeDepth(destinationIndexLine) == maxDepthTree - 1);

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

    // Casted agent 1 and 2
    const LGP::LGPAgent& lgpAgent1 = dynamic_cast<const LGP::LGPAgent&>(agents[0].get());
    const LGP::LGPAgent& lgpAgent2 = dynamic_cast<const LGP::LGPAgent&>(agents[1].get());
    auto lgpAgents = std::array<std::reference_wrapper<const LGP::LGPAgent>, 2>{lgpAgent1, lgpAgent2};
    if(&lgpAgent1 == nullptr || &lgpAgent2 == nullptr){
        throw std::invalid_argument("LGPMutator::crossoverAgents: the given agents are not LGPAgents.");
    }
    LGP::LGPManager& lgpManager = dynamic_cast<LGP::LGPManager&>(manager);
    if(&lgpManager == nullptr){
        throw std::invalid_argument("LGPMutator::initRandomAgent: the given manager is not a LGPManager.");
    }

    if(lgpAgent1.getNbLines() < 2 || lgpAgent2.getNbLines() < 2){
        return; // Crossover cannot be done if a program contains less than two lines
    }
    
    /// get node with tree size below or equal too maxDepth / 2
    std::array<size_t, 2> chosenIndexNodes;

    size_t maxDepthCross = this->maxDepth;

    std::array<std::map<size_t, std::reference_wrapper<const LGP::LGPLine>>, 2> linesToCross;
    std::array<std::set<size_t>, 2> removedIdxLines;
    for(size_t idxAgent = 0; idxAgent < 2; idxAgent++) {
        const LGP::LGPAgent& curAgent = lgpAgents.at(idxAgent);
        std::vector<size_t> possibleNodes;
        size_t nbLines = curAgent.getNbLines();
        for(size_t idx = 0; idx < nbLines; idx++) {
            size_t destIndex = curAgent.getLine(idx).getDestinationIndex();
            if(this->getRealNodeDepth(curAgent, destIndex) < maxDepthCross && this->getNodeDepth(destIndex) + maxDepthCross <= maxDepth && destIndex != 0) {
                possibleNodes.push_back(destIndex);
            }
        }
        
        if(possibleNodes.size() == 0) {
            return;
        }

        // Ranomly choose the node to crossover.
        size_t destIndexChosen = possibleNodes.at(rng.getUnsignedInt64(0, possibleNodes.size() - 1));

        // Recursively get the lines to cross
        std::vector<size_t> destinationIndexToInclude;
        destinationIndexToInclude.push_back(destIndexChosen);
        while(destinationIndexToInclude.size() > 0) {
            size_t destIndex = destinationIndexToInclude.front();
            destinationIndexToInclude.erase(destinationIndexToInclude.begin());

            size_t idxLine = this->getIndexLineFromDest(curAgent, destIndex);
            const LGP::LGPLine& lineCrossed = curAgent.getLine(idxLine);
            linesToCross.at(idxAgent).insert({destIndex, lineCrossed});
            removedIdxLines.at(idxAgent).insert(idxLine);

            std::array<bool, 2> subTree = this->hasSubTree(curAgent, idxLine);
            for(size_t idxOp = 0; idxOp < 2; idxOp ++ ) {
                if(subTree.at(idxOp)) {
                    destinationIndexToInclude.push_back(lineCrossed.getOperand(idxOp).second);
                }
            }
        }
    }

    for(size_t idxAgent = 0; idxAgent < 2; idxAgent++) {
        // Get agent and lines of the other agent
        const LGP::LGPAgent& curAgent = lgpAgents.at(idxAgent);
        const LGP::LGPAgent& otherAgent = lgpAgents.at(1 - idxAgent);
        const auto& mapLines = linesToCross.at(1 - idxAgent);

        // Add lines in opposite order.
        auto itLines = mapLines.rbegin();
        while(itLines != mapLines.rend()) {
            lgpManager.addNewLine(curAgent, itLines->second, 0);
            itLines++;
        }
        this->changeNodeIndex(curAgent, lgpManager, 0, linesToCross.at(idxAgent).begin()->first);
    }
    for(size_t idxAgent = 0; idxAgent < 2; idxAgent++) {

        auto itRemovedIdx = removedIdxLines.at(idxAgent).rbegin();
        while(itRemovedIdx != removedIdxLines.at(idxAgent).rend()) {
            // Remove the old lines
            lgpManager.removeLine(lgpAgents.at(idxAgent), *itRemovedIdx + linesToCross.at(1 - idxAgent).size());
            itRemovedIdx++;
        }

        lgpManager.identifyIntrons(lgpAgents.at(idxAgent));
    }
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

    if(manager.getOutputs().size() > 1) {
        for(size_t idx = 0; idx < manager.getOutputs().size(); idx++) {
    
            size_t nbZeroRegUsed = std::count_if(
                agent.getOutputIndices().begin(), agent.getOutputIndices().end(), []
                (size_t outputIdx) {return outputIdx == 0;});
    
            if(nbZeroRegUsed == 0) {
                throw std::runtime_error("TGPMutator::mutateLGPAgent: zero output index should always be used at least once.");
            }
    
            // One of the output must always be zero, so mutation is allowed if zero is used multiple time, or if current idx is not using zero (meaning it is used elsewhere)
            if((nbZeroRegUsed > 1 || agent.getOutputIndices().at(idx) != 0) && rng.getDouble(0.0, 1.0) < 1) {//params.mutation.prog.pMutateOutputs) {
                alterRandomOutputs(agent, manager, idx, rng);
            }
        }

    }

    // Identify introns
    if (anyMutation) {
        manager.identifyIntrons(agent);
    }
    return anyMutation;
}


bool Algorithm::TGP::TGPMutator::alterRandomOutputs(const LGP::LGPAgent& agent, LGP::LGPManager& manager, size_t location,
                                              RNG::RNG& rng)
{

    std::set<size_t> availableRegisters;
    for(size_t idx = 0; idx < agent.getNbLines(); idx++) {
        availableRegisters.insert(agent.getLine(idx).getDestinationIndex());
    }

    auto it = availableRegisters.begin();
    std::advance(it, rng.getUnsignedInt64(0, availableRegisters.size() - 1));
    manager.setOutputIndex(agent, *it, location);
    return true;
}

std::array<bool, 2> Algorithm::TGP::TGPMutator::hasSubTree(const LGP::LGPAgent& agent, size_t idx) {
    std::array<bool, 2> result = {false, false};

    const LGP::LGPLine& line = agent.getLine(idx);
    for(size_t idxOp = 0; idxOp < 2; idxOp++) {
        // Operand is a register
        result.at(idxOp) = (line.getOperand(idxOp).first == 0);
    }
    return result;
}

size_t Algorithm::TGP::TGPMutator::getIndexLineFromDest(const LGP::LGPAgent& agent, size_t destIdx) {
    size_t idxLine = 0;
    while(agent.getLine(idxLine).getDestinationIndex() != destIdx) {
        idxLine++;
        if(idxLine == agent.getNbLines()) {
            throw std::runtime_error("TGPMutator::getIndexLineFromDest: line not found");
        }
    }
    return idxLine;
}

size_t Algorithm::TGP::TGPMutator::getNodeDepth(size_t destIndex) {
    return std::floor(std::log2(destIndex + 1));
}

size_t Algorithm::TGP::TGPMutator::getRealNodeDepth(const LGP::LGPAgent& agent, size_t destIndex) {
    // Recursively get the lines to cross
    size_t highestDepth = 0;
    std::vector<size_t> destinationIndexToInclude;
    destinationIndexToInclude.push_back(destIndex);
    while(destinationIndexToInclude.size() > 0) {
        size_t destIndex = destinationIndexToInclude.front();
        destinationIndexToInclude.erase(destinationIndexToInclude.begin());

        size_t nodeDepth = this->getNodeDepth(destIndex);
        if(nodeDepth > highestDepth) {
            highestDepth = nodeDepth;
        }

        const LGP::LGPLine& line = agent.getLine(this->getIndexLineFromDest(agent, destIndex));

        std::array<bool, 2> subTree = this->hasSubTree(agent, this->getIndexLineFromDest(agent, destIndex));
        for(size_t idxOp = 0; idxOp < 2; idxOp ++ ) {
            if(subTree.at(idxOp)) {
                destinationIndexToInclude.push_back(line.getOperand(idxOp).second);
            }
        }
    }

    return highestDepth - this->getNodeDepth(destIndex);
}

void Algorithm::TGP::TGPMutator::destroySubTree(const LGP::LGPAgent& agent, size_t idxSubTree, LGP::LGPManager& manager) 
{
    // Recursively iterate to erase the lines in the hierarchy
    std::vector<size_t> destinationIdxToDestroy;
    destinationIdxToDestroy.push_back(idxSubTree);
    while(destinationIdxToDestroy.size() > 0) {
        size_t destinationIdx = destinationIdxToDestroy.front();
        destinationIdxToDestroy.erase(destinationIdxToDestroy.begin());

        size_t idxLine = this->getIndexLineFromDest(agent, destinationIdx);

        const LGP::LGPLine& destroyedLine = agent.getLine(idxLine);
        std::array<bool, 2> result = this->hasSubTree(agent, idxLine);
        for(size_t idx = 0; idx < 2; idx++) {
            // Operand is a register
            if(result.at(idx)) {
                destinationIdxToDestroy.push_back(destroyedLine.getOperand(idx).second);
            }
        }
        manager.removeLine(agent, idxLine);
    }
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
    
    std::array<bool, 2> oldSubTree = this->hasSubTree(agent, lineIndex);

    // If current index is at max depth, don't allows the line to select registers.
    bool maxDepthReached = (this->getNodeDepth(line.getDestinationIndex()) == this->maxDepth - 1);
    this->tgpLineMutator.alterCorrectLine(manager.getLineForMutation(agent, lineIndex), maxDepthReached, rng); // specified accessible registers

    if(!maxDepthReached) {
        std::array<bool, 2> newSubTree = this->hasSubTree(agent, lineIndex);
        for(size_t idx = 0; idx < 2; idx++) {
            size_t idxSubTree = 2 * line.getDestinationIndex() + 1 + idx;
            if(oldSubTree[idx] && !newSubTree[idx]) {
                // Destroy old sub tree
                this->destroySubTree(agent, idxSubTree, manager);
            } else if(!oldSubTree[idx] && newSubTree[idx]) {
                // Create new sub tree
                this->insertRandomSubTree(agent, idxSubTree, this->maxDepth, manager, rng);
            }
        }
    }
    return true;
} 

void Algorithm::TGP::TGPMutator::changeNodeIndex(const LGP::LGPAgent& agent, LGP::LGPManager& manager, size_t lineIndex, size_t destIndex)
{
    const LGP::LGPLine& line = agent.getLine(lineIndex);
    if(line.getDestinationIndex() != destIndex) {

        std::array<bool, 2> subTree = this->hasSubTree(agent, lineIndex);
        for(size_t idx = 0; idx < 2; idx++) {
            if(subTree.at(idx)) {
                this->changeNodeIndex(agent, manager, this->getIndexLineFromDest(agent, agent.getLine(lineIndex).getOperand(idx).second), 2 * destIndex + 1 + idx);
                bool success = manager.getLineForMutation(agent, lineIndex).setOperand(idx, 0, 2 * destIndex + 1 + idx);
                if(!success) {
                    throw std::runtime_error("TGPMutator::changeNodeIndex: operand of sub tree modification did not success.");
                }
            }
        }
        manager.getLineForMutation(agent, lineIndex).setDestinationIndex(destIndex);
    }
}
