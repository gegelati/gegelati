
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

    // Empty individual population
    population.clearIndividuals(graph);

    for (size_t idx = 0; idx < params.nbIndividuals; idx++) {
        this->initRandomIndividual(graph, population, params, rng);
    }
}

void Representation::LGP::LGPMutator::initRandomSpecificIndividual(const Individual& individual, EvoGraph::Graph& graph, Population& population, const RepresentationParameters& params, RNG::RNG& rng)
{
    // If first individual, check validity
    if(population.getIndividuals().size() == 1){
        this->isConfigurationValid(params, population.getOutputs());
    }

    population.emptyIndividual(individual, graph);

    LGPPopulation& lgpPopulation = dynamic_cast<LGPPopulation&>(population);
    if(&lgpPopulation == nullptr){
        throw std::invalid_argument("LGPMutator::initRandomIndividual: the given population is not a LGPPopulation.");
    }

    const LgpIndividual& lgpIndividual = dynamic_cast<const LgpIndividual&>(individual);
    if(&lgpIndividual == nullptr){
        throw std::invalid_argument("LGPMutator::initRandomIndividual: the created individual is not a LgpIndividual.");
    }

    // insert random constants in the program
    Data::Constant c_value;
    for (int i = 0; i < params.lgp.nbProgramConstant; i++) {
        c_value = {rng.getDouble(params.lgp.minConstValue,
                                 params.lgp.maxConstValue)};
        lgpPopulation.setConstantAt(individual, i, c_value);
    }

    // Select the number of line randomly
    const uint64_t nbLine = rng.getUnsignedInt64(
        params.lgp.initMinProgramSize, params.lgp.initMaxProgramSize);
    // Insert them
    while (lgpIndividual.getNbLines() < nbLine) {
        this->insertRandomLine(lgpIndividual, lgpPopulation, params, rng);
    }

    // Identify Introns
    lgpPopulation.identifyIntrons(individual);
}

void Representation::LGP::LGPMutator::crossoverIndividuals(
    std::array<std::reference_wrapper<const Individual>, 2> individuals, EvoGraph::Graph& graph, 
    Population& population, std::vector<std::reference_wrapper<const Individual>>& newSubIndividuals, 
    const RepresentationParameters& params, RNG::RNG& rng)
{
    // Casted individual 1 and 2
    const LgpIndividual& lgpIndividual1 = dynamic_cast<const LgpIndividual&>(individuals[0].get());
    const LgpIndividual& lgpIndividual2 = dynamic_cast<const LgpIndividual&>(individuals[1].get());
    auto lgpIndividuals = std::array<std::reference_wrapper<const LgpIndividual>, 2>{lgpIndividual1, lgpIndividual2};
    if(&lgpIndividual1 == nullptr || &lgpIndividual2 == nullptr){
        throw std::invalid_argument("LGPMutator::crossoverIndividuals: the given individuals are not LgpIndividuals.");
    }
    LGPPopulation& lgpPopulation = dynamic_cast<LGPPopulation&>(population);
    if(&lgpPopulation == nullptr){
        throw std::invalid_argument("LGPMutator::initRandomIndividual: the given population is not a LGPPopulation.");
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

void Representation::LGP::LGPMutator::mutateIndividual(
    const Individual& individual, EvoGraph::Graph& graph, Population& population, std::vector<std::reference_wrapper<const Individual>>& newSubIndividuals, const RepresentationParameters& params, RNG::RNG& rng)
{
    LGPPopulation& lgpPopulation = dynamic_cast<LGPPopulation&>(population);
    if(&lgpPopulation == nullptr){
        throw std::invalid_argument("LGPMutator::initRandomIndividual: the given population is not a LGPPopulation.");
    }

    const LgpIndividual& lgpIndividual = dynamic_cast<const LgpIndividual&>(individual);
    if(&lgpIndividual == nullptr){
        throw std::invalid_argument("LGPMutator::initRandomIndividual: the created individual is not a LgpIndividual.");
    }

    if (params.lgp.forceProgramBehaviorChangeOnMutation) {
        // Copy the program to check that its behavior is changed before
        // verifying its unicity against the archive
        const Individual& newProgCopy = lgpPopulation.copyIndividual(individual, graph);
        while (!this->mutateLgpIndividual(lgpIndividual, lgpPopulation, params, rng) &&
                !lgpPopulation.hasIdenticalBehavior(individual, newProgCopy));
        lgpPopulation.deleteIndividual(newProgCopy, graph);
    } else {
        // Mutate until a mutation happen
        while (!this->mutateLgpIndividual(lgpIndividual, lgpPopulation, params, rng));
    }
}

bool Representation::LGP::LGPMutator::mutateLgpIndividual(const LgpIndividual& individual, LGPPopulation& population, const RepresentationParameters& params, RNG::RNG& rng)
{
    bool anyMutation = false;
    if (individual.getNbLines() > 1 && rng.getDouble(0.0, 1.0) < params.lgp.pDelete) {
        anyMutation = true;
        deleteRandomLine(individual, population, rng);
    }

    if (individual.getNbLines() < params.lgp.maxProgramSize &&
        rng.getDouble(0.0, 1.0) < params.lgp.pAdd) {
        anyMutation = true;
        insertRandomLine(individual, population, params, rng);
    }

    if (rng.getDouble(0.0, 1.0) < params.lgp.pMutate) {
        anyMutation = true;
        alterRandomLine(individual, population, params, rng);
    }

    if (rng.getDouble(0.0, 1.0) < params.lgp.pSwap) {
        anyMutation = true;
        swapRandomLines(individual, population, rng);
    }

    // mutate the programs constants if they exists
    if (params.lgp.nbProgramConstant > 0 &&
        rng.getDouble(0.0, 1.0) < params.lgp.pConstantMutation) {
        anyMutation = true;
        alterRandomConstant(individual, population, params, rng);
    }

    for(size_t idx = 0; idx < individual.getUsedNbOutputs(population.getOutputs()); idx++) {
        if(rng.getDouble(0.0, 1.0) < params.lgp.pMutateOutput) {
            alterRandomOutputs(individual, population, idx, params, rng);
        }
    }

    // Identify introns
    if (anyMutation) {
        population.identifyIntrons(individual);
    }
    return anyMutation;
}

bool Representation::LGP::LGPMutator::deleteRandomLine(const LgpIndividual& individual, LGPPopulation& population, 
                                               RNG::RNG& rng)
{
    // Line cannot be removed from a program with a single line.
    if (individual.getNbLines() <= 1) {
        return false;
    }

    uint64_t lineIndex = rng.getUnsignedInt64(0, individual.getNbLines() - 1);
    population.removeLine(individual, lineIndex);
    return true;
}

void Representation::LGP::LGPMutator::insertRandomLine(const LgpIndividual& individual, LGPPopulation& population, const RepresentationParameters& params, RNG::RNG& rng)
{
    uint64_t lineIndex = rng.getUnsignedInt64(0, individual.getNbLines());
    population.addNewLine(individual, lineIndex);
    this->lineMutator.initRandomCorrectLine(population.getLineForMutation(individual, lineIndex), rng);
}


bool Representation::LGP::LGPMutator::swapRandomLines(const LgpIndividual& individual, LGPPopulation& population, 
                                              RNG::RNG& rng)
{
    if (individual.getNbLines() < 2) {
        return false;
    }
    // Select two distinct random index.
    const uint64_t lineIndex0 = rng.getUnsignedInt64(0, individual.getNbLines() - 1);
    uint64_t lineIndex1 = rng.getUnsignedInt64(0, individual.getNbLines() - 2);
    lineIndex1 += (lineIndex1 >= lineIndex0) ? 1 : 0;

    population.swapLines(individual, lineIndex0, lineIndex1);

    return true;
}

bool Representation::LGP::LGPMutator::alterRandomLine(const LgpIndividual& individual, LGPPopulation& population, 
                                              const RepresentationParameters& params, RNG::RNG& rng)
{
    if (individual.getNbLines() < 1) {
        return false;
    }
    // Select a random index.
    const uint64_t lineIndex = rng.getUnsignedInt64(0, individual.getNbLines() - 1);
    this->lineMutator.alterCorrectLine(population.getLineForMutation(individual, lineIndex), rng);
    return true;
}

bool Representation::LGP::LGPMutator::alterRandomConstant(
    const LgpIndividual& individual, LGPPopulation& population, const RepresentationParameters& params, RNG::RNG& rng)
{
    const uint64_t constant_idx = rng.getUnsignedInt64(
        0, params.lgp.nbProgramConstant - 1);

    // Sample the new value
    double delta = rng.getDouble(0.5, 1.5);
    if (delta > 1)
        delta = delta * 2 - 1;

    double currentConstantValue = individual.getConstantAt(constant_idx);

    double newConstantValue = currentConstantValue * delta;

    if (0.1 > rng.getDouble(0, 1)) {
        newConstantValue = -newConstantValue;
    }

    Data::Constant newConstant = {newConstantValue};
    population.setConstantAt(individual, constant_idx, newConstant);

    return true;
}

bool Representation::LGP::LGPMutator::alterRandomOutputs(const LgpIndividual& individual, LGPPopulation& population, size_t location, 
                                             const RepresentationParameters& params, RNG::RNG& rng)
{
    population.setOutputIndex(individual, rng.getUnsignedInt64(0, params.lgp.nbRegisters - 1), location);
    return true;
}