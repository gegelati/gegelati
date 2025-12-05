
#include "algorithm/lgp/lgpMutator.h"


void Algorithm::LGP::LGPMutator::initRandomPopulation(std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Learn::LearningParameters& params, RNG::RNG& rng, size_t nbActions)
{

}

std::shared_ptr<const Algorithm::Agent> Algorithm::LGP::LGPMutator::initRandomAgent(std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Learn::LearningParameters& params, RNG::RNG& rng, size_t nbActions)
{
    auto agent = manager->createAgent(graph);
    auto lgpAgent = std::dynamic_pointer_cast<const LGPAgent>(agent);
    if(lgpAgent == nullptr){
        throw std::invalid_argument("LGPMutator::initRandomAgent: the created agent is not a LGPAgent.");
    }

    auto lgpManager = std::dynamic_pointer_cast<LGPManager>(manager);
    if(lgpManager == nullptr){
        throw std::invalid_argument("LGPMutator::initRandomAgent: the given manager is not a LGPManager.");
    }

    // insert random constants in the program
    Data::Constant c_value;
    for (int i = 0; i < lgpAgent->getEnvironment()->getParams().nbProgramConstant; i++) {
        c_value = {rng.getDouble(params.mutation.prog.minConstValue,
                                 params.mutation.prog.maxConstValue)};
        lgpManager->setConstantAt(agent, i, c_value);
        //lgpAgent->getConstantHandler().setDataAt(typeid(Data::Constant), i, c_value);xo
    }

    // Select the number of line randomly
    const uint64_t nbLine = rng.getUnsignedInt64(
        params.mutation.prog.initMinProgramSize, params.mutation.prog.initMaxProgramSize);
    // Insert them
    while (lgpAgent->getNbLines() < nbLine) {
        this->insertRandomLine(lgpAgent, lgpManager, rng);
    }

    // Identify Introns
    lgpManager->identifyIntrons(agent);
    return agent;
}

void Algorithm::LGP::LGPMutator::crossoverAgents(
    std::vector<std::shared_ptr<const Agent>> agents, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Selector::SelectionContext& context, std::vector<std::shared_ptr<const Agent>> newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    /**NO CROSSOVER FOR NOW */
}

void Algorithm::LGP::LGPMutator::mutateAgent(
    std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Selector::SelectionContext& context, std::vector<std::shared_ptr<const Agent>> newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    auto lgpManager = std::dynamic_pointer_cast<LGPManager>(manager);
    if(lgpManager == nullptr){
        throw std::invalid_argument("LGPMutator::initRandomAgent: the given manager is not a LGPManager.");
    }

    auto lgpAgent = std::dynamic_pointer_cast<const LGPAgent>(agent);
    if(lgpAgent == nullptr){
        throw std::invalid_argument("LGPMutator::initRandomAgent: the created agent is not a LGPAgent.");
    }

    // If the lgp behavior should be new after mutation:
    std::shared_ptr<const Agent> agentCopy(nullptr);
    if (params.mutation.tpg.forceProgramBehaviorChangeOnMutation) {
        // Copy the program to check that its behavior is changed before
        // verifying its unicity against the archive
        agentCopy = manager->copyAgent(agent, graph);
    }

    bool allUnique;
    // Mutate behavior until it changes (against the archive).
    do {

        // If a new program is created
        if (rng.getDouble(0.0, 1.0) < params.mutation.prog.pNewProgram) {
            
        }
        else {
            // Mutate until something is mutated (i.e. the function returns
            // true) And until the program behavior is changed
            while (!(
                this->mutateLGPAgent(lgpAgent, lgpManager, params, rng) &&
                !(agentCopy != nullptr &&
                  lgpManager->hasIdenticalBehavior(agent, agentCopy))));
        }
        // Check for uniqueness in archive
        auto archivedDataHandlers = lgpManager->getArchive()->getDataHandlers();
        std::map<size_t, double> hashesAndResults;
        /*Program::ProgramExecutionEngine pee(*agent);
        for (std::pair<
                 size_t,
                 std::vector<std::reference_wrapper<const Data::DataHandler>>>
                 archiveDatahandler : archivedDataHandlers) {
            // Execute the mutated program on the archive data handlers
            pee.setDataSources(archiveDatahandler.second);
            double result = pee.executeProgram();
            hashesAndResults.insert({archiveDatahandler.first, result});
        }*/

        // If the result is not unique, do another mutation.
        allUnique = lgpManager->getArchive()->areProgramResultsUnique(hashesAndResults);

        // Do not use Archive right now if the environment is continuous
        // TODO Update that
    } while (!allUnique &&
             (lgpManager->getNbOutputs() > 1 ||
              params.mutation.tpg.useMultiActionProgram));
}

bool Algorithm::LGP::LGPMutator::mutateLGPAgent(std::shared_ptr<const LGPAgent> agent, std::shared_ptr<LGPManager> manager, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    bool anyMutation = false;
    if (agent->getNbLines() > 1 && rng.getDouble(0.0, 1.0) < params.mutation.prog.pDelete) {
        anyMutation = true;
        deleteRandomLine(agent, manager, rng);
    }

    if (agent->getNbLines() < params.mutation.prog.maxProgramSize &&
        rng.getDouble(0.0, 1.0) < params.mutation.prog.pAdd) {
        anyMutation = true;
        insertRandomLine(agent, manager, rng);
    }

    if (rng.getDouble(0.0, 1.0) < params.mutation.prog.pMutate) {
        anyMutation = true;
        alterRandomLine(agent, manager, rng);
    }

    if (rng.getDouble(0.0, 1.0) < params.mutation.prog.pSwap) {
        anyMutation = true;
        swapRandomLines(agent, manager, rng);
    }

    // mutate the programs constants if they exists
    if (agent->getEnvironment()->getParams().nbProgramConstant > 0 &&
        rng.getDouble(0.0, 1.0) < params.mutation.prog.pConstantMutation) {
        anyMutation = true;
        alterRandomConstant(agent, manager, params, rng);
    }

    // Identify introns
    if (anyMutation) {
        manager->identifyIntrons(agent);
    }
    return anyMutation;
}

bool Algorithm::LGP::LGPMutator::deleteRandomLine(std::shared_ptr<const LGPAgent> agent, std::shared_ptr<LGPManager> manager, 
                                               RNG::RNG& rng)
{
    // Line cannot be removed from a program with a single line.
    if (agent->getNbLines() <= 1) {
        return false;
    }

    uint64_t lineIndex = rng.getUnsignedInt64(0, agent->getNbLines() - 1);
    manager->removeLine(agent, lineIndex);
    return true;
}

void Algorithm::LGP::LGPMutator::insertRandomLine(std::shared_ptr<const LGPAgent> agent, std::shared_ptr<LGPManager> manager,  RNG::RNG& rng)
{
    uint64_t lineIndex = rng.getUnsignedInt64(0, agent->getNbLines());
    LGPLine& line = manager->addNewLine(agent, lineIndex);
    this->lineMutator.initRandomCorrectLine(line, rng);
}


bool Algorithm::LGP::LGPMutator::swapRandomLines(std::shared_ptr<const LGPAgent> agent, std::shared_ptr<LGPManager> manager, 
                                              RNG::RNG& rng)
{
    if (agent->getNbLines() < 2) {
        return false;
    }
    // Select two distinct random index.
    const uint64_t lineIndex0 = rng.getUnsignedInt64(0, agent->getNbLines() - 1);
    uint64_t lineIndex1 = rng.getUnsignedInt64(0, agent->getNbLines() - 2);
    lineIndex1 += (lineIndex1 >= lineIndex0) ? 1 : 0;

    manager->swapLines(agent, lineIndex0, lineIndex1);

    return true;
}

bool Algorithm::LGP::LGPMutator::alterRandomLine(std::shared_ptr<const LGPAgent> agent, std::shared_ptr<LGPManager> manager, 
                                              RNG::RNG& rng)
{
    if (agent->getNbLines() < 1) {
        return false;
    }
    // Select a random index.
    const uint64_t lineIndex = rng.getUnsignedInt64(0, agent->getNbLines() - 1);
    this->lineMutator.alterCorrectLine(manager->getLine(agent, lineIndex), rng);
    return true;
}

bool Algorithm::LGP::LGPMutator::alterRandomConstant(
    std::shared_ptr<const LGPAgent> agent, std::shared_ptr<LGPManager> manager, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    const uint64_t constant_idx = rng.getUnsignedInt64(
        0, params.nbProgramConstant - 1);

    // Sample the new value
    double delta = rng.getDouble(0.5, 1.5);
    if (delta > 1)
        delta = delta * 2 - 1;

    double currentConstantValue = agent->getConstantAt(constant_idx);

    double newConstantValue = currentConstantValue * delta;

    if (0.1 > rng.getDouble(0, 1)) {
        newConstantValue = -newConstantValue;
    }

    Data::Constant newConstant = {newConstantValue};
    manager->setConstantAt(agent, constant_idx, newConstant);

    return true;
}