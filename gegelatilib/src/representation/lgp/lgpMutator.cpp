
#include <array>
#include "representation/lgp/lgpMutator.h"

bool Representation::LGP::LGPMutator::isConfigurationValid(const RepresentationParameters& params, const Output::OutputHandler& outputs) const
{
    if(outputs.sizeContinuous() != 0 && outputs.sizeDiscrete() != 0){
        throw std::runtime_error("LGPMutator::initRandomPopulation: LGP does not support mixed discrete and continuous outputs.");
    } else if (outputs.sizeContinuous() == 0 && outputs.sizeDiscrete() == 0) {
        throw std::runtime_error("LGPMutator::initRandomPopulation: No outputs defined.");
    }
    return true;
}

void Representation::LGP::LGPMutator::initRandomPopulation(EvoGraph::Graph& graph, AgentManager& manager, const RepresentationParameters& params, RNG::RNG& rng)
{
    // Check configuration is valid
    this->isConfigurationValid(params, manager.getOutputs());

    // Empty agent manager
    manager.clearAgents(graph);

    for (size_t idx = 0; idx < params.nbAgents; idx++) {
        this->initRandomAgent(graph, manager, params, rng);
    }
}

void Representation::LGP::LGPMutator::initRandomSpecificAgent(const Agent& agent, EvoGraph::Graph& graph, AgentManager& manager, const RepresentationParameters& params, RNG::RNG& rng)
{
    // If first agent, check validity
    if(manager.getAgents().size() == 1){
        this->isConfigurationValid(params, manager.getOutputs());
    }

    manager.emptyAgent(agent, graph);

    LGPManager& lgpManager = dynamic_cast<LGPManager&>(manager);
    if(&lgpManager == nullptr){
        throw std::invalid_argument("LGPMutator::initRandomAgent: the given manager is not a LGPManager.");
    }

    const LGPAgent& lgpAgent = dynamic_cast<const LGPAgent&>(agent);
    if(&lgpAgent == nullptr){
        throw std::invalid_argument("LGPMutator::initRandomAgent: the created agent is not a LGPAgent.");
    }

    // insert random constants in the program
    Data::Constant c_value;
    for (int i = 0; i < params.lgp.nbProgramConstant; i++) {
        c_value = {rng.getDouble(params.lgp.minConstValue,
                                 params.lgp.maxConstValue)};
        lgpManager.setConstantAt(agent, i, c_value);
    }

    // Select the number of line randomly
    const uint64_t nbLine = rng.getUnsignedInt64(
        params.lgp.initMinProgramSize, params.lgp.initMaxProgramSize);
    // Insert them
    while (lgpAgent.getNbLines() < nbLine) {
        this->insertRandomLine(lgpAgent, lgpManager, params, rng);
    }

    // Identify Introns
    lgpManager.identifyIntrons(agent);
}

void Representation::LGP::LGPMutator::crossoverAgents(
    std::array<std::reference_wrapper<const Agent>, 2> agents, EvoGraph::Graph& graph, 
    AgentManager& manager, std::vector<std::reference_wrapper<const Agent>>& newSubAgents, 
    const RepresentationParameters& params, RNG::RNG& rng)
{
    // Casted agent 1 and 2
    const LGPAgent& lgpAgent1 = dynamic_cast<const LGPAgent&>(agents[0].get());
    const LGPAgent& lgpAgent2 = dynamic_cast<const LGPAgent&>(agents[1].get());
    auto lgpAgents = std::array<std::reference_wrapper<const LGPAgent>, 2>{lgpAgent1, lgpAgent2};
    if(&lgpAgent1 == nullptr || &lgpAgent2 == nullptr){
        throw std::invalid_argument("LGPMutator::crossoverAgents: the given agents are not LGPAgents.");
    }
    LGPManager& lgpManager = dynamic_cast<LGPManager&>(manager);
    if(&lgpManager == nullptr){
        throw std::invalid_argument("LGPMutator::initRandomAgent: the given manager is not a LGPManager.");
    }

    if(lgpAgent1.getNbLines() < 2 || lgpAgent2.getNbLines() < 2){
        return; // Crossover cannot be done if a program contains less than two lines
    }

    
    std::array<uint64_t, 2> cutStart, cutEnd, sizeProgs;


    // if the sum of the program size is above the max size, the size of
    // the cross lines is the same for both program.
    bool specialCase =
        lgpAgent1.getNbLines() + lgpAgent2.getNbLines() >=
        params.lgp.maxProgramSize;

    // Select random index for the crossover, normal case
    for (int i = 0; i < 2; i++) {
        uint64_t nbLines = lgpAgents[i].get().getNbLines();
        if (specialCase) {
            nbLines = std::min(lgpAgents[0].get().getNbLines(),
                               lgpAgents[1].get().getNbLines());
        }

        cutStart[i] = rng.getUnsignedInt64(0, nbLines - 1);
        cutEnd[i] = rng.getUnsignedInt64(0, nbLines - 2);
        if (cutEnd[i] == cutStart[i]) {
            cutEnd[i]++;
        }
        else if (cutEnd[i] < cutStart[i]) {
            std::swap(cutStart[i], cutEnd[i]);
        }

        if (specialCase) {
            cutStart[1] = cutStart[0];
            cutEnd[1] = cutEnd[0];
            break;
        }
    }

    // Compute program size of the children
    for (int i = 0; i < 2; i++) {
        sizeProgs[i] = lgpAgents[i].get().getNbLines() -
                       (cutEnd[i] - cutStart[i]) +
                       (cutEnd[1 - i] - cutStart[1 - i]);
    }

    // Store the lines of parents in list, need to copy them because it is deleted after
    std::array<std::vector<std::reference_wrapper<const LGPLine>>, 2> lines;
    for(int i = 0; i < 2; i++){
        lines[i].reserve(lgpAgents[i].get().getNbLines());
        for (size_t j = 0; j < lgpAgents[i].get().getNbLines(); j++) {
            lines[i].push_back(lgpAgents[i].get().getLine(j));
        }
    }
    // Add the new lines
    for (int childIdx = 0; childIdx < 2; childIdx++) {
        auto& parent1 = lines[childIdx];
        auto& parent2 = lines[1 - childIdx];
        uint64_t start1 = cutStart[childIdx], end1 = cutEnd[childIdx];
        uint64_t start2 = cutStart[1 - childIdx], end2 = cutEnd[1 - childIdx];

        for (size_t idx = 0; idx < sizeProgs[childIdx]; idx++) {
            if (idx < start1) {

                lgpManager.addNewLine(lgpAgents[childIdx], parent1[idx]);
            }
            else if (idx >= start1 + (end2 - start2)) {

                lgpManager.addNewLine(lgpAgents[childIdx], parent1[idx + (end1 - start1) - (end2 - start2)]);
            }
            else {
                lgpManager.addNewLine(lgpAgents[childIdx], parent2[idx - start1 + start2]);
            }
        }

    }

    // Remove the old lines and identify introns
    for(int i = 0; i < 2; i++){
        for (size_t j = 0; j < lines[i].size(); j++) {
            lgpManager.removeLine(lgpAgents[i].get(), 0);
        }
        lgpManager.identifyIntrons(lgpAgents[i]);
    }
}

void Representation::LGP::LGPMutator::mutateAgent(
    const Agent& agent, EvoGraph::Graph& graph, AgentManager& manager, std::vector<std::reference_wrapper<const Agent>>& newSubAgents, const RepresentationParameters& params, RNG::RNG& rng)
{
    LGPManager& lgpManager = dynamic_cast<LGPManager&>(manager);
    if(&lgpManager == nullptr){
        throw std::invalid_argument("LGPMutator::initRandomAgent: the given manager is not a LGPManager.");
    }

    const LGPAgent& lgpAgent = dynamic_cast<const LGPAgent&>(agent);
    if(&lgpAgent == nullptr){
        throw std::invalid_argument("LGPMutator::initRandomAgent: the created agent is not a LGPAgent.");
    }

    if (params.lgp.forceProgramBehaviorChangeOnMutation) {
        // Copy the program to check that its behavior is changed before
        // verifying its unicity against the archive
        const Agent& newProgCopy = lgpManager.copyAgent(agent, graph);
        while (!this->mutateLGPAgent(lgpAgent, lgpManager, params, rng) &&
                !lgpManager.hasIdenticalBehavior(agent, newProgCopy));
        lgpManager.deleteAgent(newProgCopy, graph);
    } else {
        // Mutate until a mutation happen
        while (!this->mutateLGPAgent(lgpAgent, lgpManager, params, rng));
    }
}

bool Representation::LGP::LGPMutator::mutateLGPAgent(const LGPAgent& agent, LGPManager& manager, const RepresentationParameters& params, RNG::RNG& rng)
{
    bool anyMutation = false;
    if (agent.getNbLines() > 1 && rng.getDouble(0.0, 1.0) < params.lgp.pDelete) {
        anyMutation = true;
        deleteRandomLine(agent, manager, rng);
    }

    if (agent.getNbLines() < params.lgp.maxProgramSize &&
        rng.getDouble(0.0, 1.0) < params.lgp.pAdd) {
        anyMutation = true;
        insertRandomLine(agent, manager, params, rng);
    }

    if (rng.getDouble(0.0, 1.0) < params.lgp.pMutate) {
        anyMutation = true;
        alterRandomLine(agent, manager, params, rng);
    }

    if (rng.getDouble(0.0, 1.0) < params.lgp.pSwap) {
        anyMutation = true;
        swapRandomLines(agent, manager, rng);
    }

    // mutate the programs constants if they exists
    if (params.lgp.nbProgramConstant > 0 &&
        rng.getDouble(0.0, 1.0) < params.lgp.pConstantMutation) {
        anyMutation = true;
        alterRandomConstant(agent, manager, params, rng);
    }

    for(size_t idx = 0; idx < agent.getUsedNbOutputs(manager.getOutputs()); idx++) {
        if(rng.getDouble(0.0, 1.0) < params.lgp.pMutateOutput) {
            alterRandomOutputs(agent, manager, idx, params, rng);
        }
    }

    // Identify introns
    if (anyMutation) {
        manager.identifyIntrons(agent);
    }
    return anyMutation;
}

bool Representation::LGP::LGPMutator::deleteRandomLine(const LGPAgent& agent, LGPManager& manager, 
                                               RNG::RNG& rng)
{
    // Line cannot be removed from a program with a single line.
    if (agent.getNbLines() <= 1) {
        return false;
    }

    uint64_t lineIndex = rng.getUnsignedInt64(0, agent.getNbLines() - 1);
    manager.removeLine(agent, lineIndex);
    return true;
}

void Representation::LGP::LGPMutator::insertRandomLine(const LGPAgent& agent, LGPManager& manager, const RepresentationParameters& params, RNG::RNG& rng)
{
    uint64_t lineIndex = rng.getUnsignedInt64(0, agent.getNbLines());
    manager.addNewLine(agent, lineIndex);
    this->lineMutator.initRandomCorrectLine(manager.getLineForMutation(agent, lineIndex), rng);
}


bool Representation::LGP::LGPMutator::swapRandomLines(const LGPAgent& agent, LGPManager& manager, 
                                              RNG::RNG& rng)
{
    if (agent.getNbLines() < 2) {
        return false;
    }
    // Select two distinct random index.
    const uint64_t lineIndex0 = rng.getUnsignedInt64(0, agent.getNbLines() - 1);
    uint64_t lineIndex1 = rng.getUnsignedInt64(0, agent.getNbLines() - 2);
    lineIndex1 += (lineIndex1 >= lineIndex0) ? 1 : 0;

    manager.swapLines(agent, lineIndex0, lineIndex1);

    return true;
}

bool Representation::LGP::LGPMutator::alterRandomLine(const LGPAgent& agent, LGPManager& manager, 
                                              const RepresentationParameters& params, RNG::RNG& rng)
{
    if (agent.getNbLines() < 1) {
        return false;
    }
    // Select a random index.
    const uint64_t lineIndex = rng.getUnsignedInt64(0, agent.getNbLines() - 1);
    this->lineMutator.alterCorrectLine(manager.getLineForMutation(agent, lineIndex), rng);
    return true;
}

bool Representation::LGP::LGPMutator::alterRandomConstant(
    const LGPAgent& agent, LGPManager& manager, const RepresentationParameters& params, RNG::RNG& rng)
{
    const uint64_t constant_idx = rng.getUnsignedInt64(
        0, params.lgp.nbProgramConstant - 1);

    // Sample the new value
    double delta = rng.getDouble(0.5, 1.5);
    if (delta > 1)
        delta = delta * 2 - 1;

    double currentConstantValue = agent.getConstantAt(constant_idx);

    double newConstantValue = currentConstantValue * delta;

    if (0.1 > rng.getDouble(0, 1)) {
        newConstantValue = -newConstantValue;
    }

    Data::Constant newConstant = {newConstantValue};
    manager.setConstantAt(agent, constant_idx, newConstant);

    return true;
}

bool Representation::LGP::LGPMutator::alterRandomOutputs(const LGPAgent& agent, LGPManager& manager, size_t location, 
                                             const RepresentationParameters& params, RNG::RNG& rng)
{
    manager.setOutputIndex(agent, rng.getUnsignedInt64(0, params.lgp.nbRegisters - 1), location);
    return true;
}