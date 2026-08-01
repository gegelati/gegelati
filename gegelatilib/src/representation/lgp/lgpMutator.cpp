
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

void Representation::LGP::LGPMutator::initRandomPopulation(EvoGraph::Graph& graph, Population& population, const RepresentationParameters& params, RNG::RNG& rng)
{
    // Check configuration is valid
    this->isConfigurationValid(params, population.getOutputs());

    // Empty agent population
    population.clearAgents(graph);

    for (size_t idx = 0; idx < params.nbIndividuals; idx++) {
        this->initRandomAgent(graph, population, params, rng);
    }
}

void Representation::LGP::LGPMutator::initRandomSpecificAgent(const Individual& agent, EvoGraph::Graph& graph, Population& population, const RepresentationParameters& params, RNG::RNG& rng)
{
    // If first agent, check validity
    if(population.getAgents().size() == 1){
        this->isConfigurationValid(params, population.getOutputs());
    }

    population.emptyAgent(agent, graph);

    LGPPopulation& lgpPopulation = dynamic_cast<LGPPopulation&>(population);
    if(&lgpPopulation == nullptr){
        throw std::invalid_argument("LGPMutator::initRandomAgent: the given population is not a LGPPopulation.");
    }

    const LgpIndividual& lgpIndividual = dynamic_cast<const LgpIndividual&>(agent);
    if(&lgpIndividual == nullptr){
        throw std::invalid_argument("LGPMutator::initRandomAgent: the created agent is not a LgpIndividual.");
    }

    // insert random constants in the program
    Data::Constant c_value;
    for (int i = 0; i < params.lgp.nbProgramConstant; i++) {
        c_value = {rng.getDouble(params.lgp.minConstValue,
                                 params.lgp.maxConstValue)};
        lgpPopulation.setConstantAt(agent, i, c_value);
    }

    // Select the number of line randomly
    const uint64_t nbLine = rng.getUnsignedInt64(
        params.lgp.initMinProgramSize, params.lgp.initMaxProgramSize);
    // Insert them
    while (lgpIndividual.getNbLines() < nbLine) {
        this->insertRandomLine(lgpIndividual, lgpPopulation, params, rng);
    }

    // Identify Introns
    lgpPopulation.identifyIntrons(agent);
}

void Representation::LGP::LGPMutator::crossoverAgents(
    std::array<std::reference_wrapper<const Individual>, 2> agents, EvoGraph::Graph& graph, 
    Population& population, std::vector<std::reference_wrapper<const Individual>>& newSubAgents, 
    const RepresentationParameters& params, RNG::RNG& rng)
{
    // Casted agent 1 and 2
    const LgpIndividual& lgpIndividual1 = dynamic_cast<const LgpIndividual&>(agents[0].get());
    const LgpIndividual& lgpIndividual2 = dynamic_cast<const LgpIndividual&>(agents[1].get());
    auto lgpIndividuals = std::array<std::reference_wrapper<const LgpIndividual>, 2>{lgpIndividual1, lgpIndividual2};
    if(&lgpIndividual1 == nullptr || &lgpIndividual2 == nullptr){
        throw std::invalid_argument("LGPMutator::crossoverAgents: the given agents are not LgpIndividuals.");
    }
    LGPPopulation& lgpPopulation = dynamic_cast<LGPPopulation&>(population);
    if(&lgpPopulation == nullptr){
        throw std::invalid_argument("LGPMutator::initRandomAgent: the given population is not a LGPPopulation.");
    }

    if(lgpIndividual1.getNbLines() < 2 || lgpIndividual2.getNbLines() < 2){
        return; // Crossover cannot be done if a program contains less than two lines
    }

    
    std::array<uint64_t, 2> cutStart, cutEnd, sizeProgs;


    // if the sum of the program size is above the max size, the size of
    // the cross lines is the same for both program.
    bool specialCase =
        lgpIndividual1.getNbLines() + lgpIndividual2.getNbLines() >=
        params.lgp.maxProgramSize;

    // Select random index for the crossover, normal case
    for (int i = 0; i < 2; i++) {
        uint64_t nbLines = lgpIndividuals[i].get().getNbLines();
        if (specialCase) {
            nbLines = std::min(lgpIndividuals[0].get().getNbLines(),
                               lgpIndividuals[1].get().getNbLines());
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
        sizeProgs[i] = lgpIndividuals[i].get().getNbLines() -
                       (cutEnd[i] - cutStart[i]) +
                       (cutEnd[1 - i] - cutStart[1 - i]);
    }

    // Store the lines of parents in list, need to copy them because it is deleted after
    std::array<std::vector<std::reference_wrapper<const LGPLine>>, 2> lines;
    for(int i = 0; i < 2; i++){
        lines[i].reserve(lgpIndividuals[i].get().getNbLines());
        for (size_t j = 0; j < lgpIndividuals[i].get().getNbLines(); j++) {
            lines[i].push_back(lgpIndividuals[i].get().getLine(j));
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

                lgpPopulation.addNewLine(lgpIndividuals[childIdx], parent1[idx]);
            }
            else if (idx >= start1 + (end2 - start2)) {

                lgpPopulation.addNewLine(lgpIndividuals[childIdx], parent1[idx + (end1 - start1) - (end2 - start2)]);
            }
            else {
                lgpPopulation.addNewLine(lgpIndividuals[childIdx], parent2[idx - start1 + start2]);
            }
        }

    }

    // Remove the old lines and identify introns
    for(int i = 0; i < 2; i++){
        for (size_t j = 0; j < lines[i].size(); j++) {
            lgpPopulation.removeLine(lgpIndividuals[i].get(), 0);
        }
        lgpPopulation.identifyIntrons(lgpIndividuals[i]);
    }
}

void Representation::LGP::LGPMutator::mutateAgent(
    const Individual& agent, EvoGraph::Graph& graph, Population& population, std::vector<std::reference_wrapper<const Individual>>& newSubAgents, const RepresentationParameters& params, RNG::RNG& rng)
{
    LGPPopulation& lgpPopulation = dynamic_cast<LGPPopulation&>(population);
    if(&lgpPopulation == nullptr){
        throw std::invalid_argument("LGPMutator::initRandomAgent: the given population is not a LGPPopulation.");
    }

    const LgpIndividual& lgpIndividual = dynamic_cast<const LgpIndividual&>(agent);
    if(&lgpIndividual == nullptr){
        throw std::invalid_argument("LGPMutator::initRandomAgent: the created agent is not a LgpIndividual.");
    }

    if (params.lgp.forceProgramBehaviorChangeOnMutation) {
        // Copy the program to check that its behavior is changed before
        // verifying its unicity against the archive
        const Individual& newProgCopy = lgpPopulation.copyAgent(agent, graph);
        while (!this->mutateLgpIndividual(lgpIndividual, lgpPopulation, params, rng) &&
                !lgpPopulation.hasIdenticalBehavior(agent, newProgCopy));
        lgpPopulation.deleteAgent(newProgCopy, graph);
    } else {
        // Mutate until a mutation happen
        while (!this->mutateLgpIndividual(lgpIndividual, lgpPopulation, params, rng));
    }
}

bool Representation::LGP::LGPMutator::mutateLgpIndividual(const LgpIndividual& agent, LGPPopulation& population, const RepresentationParameters& params, RNG::RNG& rng)
{
    bool anyMutation = false;
    if (agent.getNbLines() > 1 && rng.getDouble(0.0, 1.0) < params.lgp.pDelete) {
        anyMutation = true;
        deleteRandomLine(agent, population, rng);
    }

    if (agent.getNbLines() < params.lgp.maxProgramSize &&
        rng.getDouble(0.0, 1.0) < params.lgp.pAdd) {
        anyMutation = true;
        insertRandomLine(agent, population, params, rng);
    }

    if (rng.getDouble(0.0, 1.0) < params.lgp.pMutate) {
        anyMutation = true;
        alterRandomLine(agent, population, params, rng);
    }

    if (rng.getDouble(0.0, 1.0) < params.lgp.pSwap) {
        anyMutation = true;
        swapRandomLines(agent, population, rng);
    }

    // mutate the programs constants if they exists
    if (params.lgp.nbProgramConstant > 0 &&
        rng.getDouble(0.0, 1.0) < params.lgp.pConstantMutation) {
        anyMutation = true;
        alterRandomConstant(agent, population, params, rng);
    }

    for(size_t idx = 0; idx < agent.getUsedNbOutputs(population.getOutputs()); idx++) {
        if(rng.getDouble(0.0, 1.0) < params.lgp.pMutateOutput) {
            alterRandomOutputs(agent, population, idx, params, rng);
        }
    }

    // Identify introns
    if (anyMutation) {
        population.identifyIntrons(agent);
    }
    return anyMutation;
}

bool Representation::LGP::LGPMutator::deleteRandomLine(const LgpIndividual& agent, LGPPopulation& population, 
                                               RNG::RNG& rng)
{
    // Line cannot be removed from a program with a single line.
    if (agent.getNbLines() <= 1) {
        return false;
    }

    uint64_t lineIndex = rng.getUnsignedInt64(0, agent.getNbLines() - 1);
    population.removeLine(agent, lineIndex);
    return true;
}

void Representation::LGP::LGPMutator::insertRandomLine(const LgpIndividual& agent, LGPPopulation& population, const RepresentationParameters& params, RNG::RNG& rng)
{
    uint64_t lineIndex = rng.getUnsignedInt64(0, agent.getNbLines());
    population.addNewLine(agent, lineIndex);
    this->lineMutator.initRandomCorrectLine(population.getLineForMutation(agent, lineIndex), rng);
}


bool Representation::LGP::LGPMutator::swapRandomLines(const LgpIndividual& agent, LGPPopulation& population, 
                                              RNG::RNG& rng)
{
    if (agent.getNbLines() < 2) {
        return false;
    }
    // Select two distinct random index.
    const uint64_t lineIndex0 = rng.getUnsignedInt64(0, agent.getNbLines() - 1);
    uint64_t lineIndex1 = rng.getUnsignedInt64(0, agent.getNbLines() - 2);
    lineIndex1 += (lineIndex1 >= lineIndex0) ? 1 : 0;

    population.swapLines(agent, lineIndex0, lineIndex1);

    return true;
}

bool Representation::LGP::LGPMutator::alterRandomLine(const LgpIndividual& agent, LGPPopulation& population, 
                                              const RepresentationParameters& params, RNG::RNG& rng)
{
    if (agent.getNbLines() < 1) {
        return false;
    }
    // Select a random index.
    const uint64_t lineIndex = rng.getUnsignedInt64(0, agent.getNbLines() - 1);
    this->lineMutator.alterCorrectLine(population.getLineForMutation(agent, lineIndex), rng);
    return true;
}

bool Representation::LGP::LGPMutator::alterRandomConstant(
    const LgpIndividual& agent, LGPPopulation& population, const RepresentationParameters& params, RNG::RNG& rng)
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
    population.setConstantAt(agent, constant_idx, newConstant);

    return true;
}

bool Representation::LGP::LGPMutator::alterRandomOutputs(const LgpIndividual& agent, LGPPopulation& population, size_t location, 
                                             const RepresentationParameters& params, RNG::RNG& rng)
{
    population.setOutputIndex(agent, rng.getUnsignedInt64(0, params.lgp.nbRegisters - 1), location);
    return true;
}