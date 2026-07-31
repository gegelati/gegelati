
#include <array>
#include "representation/cgp/cgpMutator.h"

bool Representation::CGP::CGPMutator::isConfigurationValid(const RepresentationParameters& params, const Output::OutputHandler& outputs) const
{
    if(outputs.sizeContinuous() != 0 && outputs.sizeDiscrete() != 0){
        throw std::runtime_error("CGPMutator::initRandomPopulation: CGP does not support mixed discrete and continuous outputs.");
    } else if (outputs.sizeContinuous() == 0 && outputs.sizeDiscrete() == 0) {
        throw std::runtime_error("CGPMutator::initRandomPopulation: No outputs defined.");
    }
    return true;
}

void Representation::CGP::CGPMutator::initRandomSpecificAgent(const Individual& agent, EvoGraph::Graph& graph, Population& population, const RepresentationParameters& params, RNG::RNG& rng)
{
    // If first agent, check validity
    if(population.getAgents().size() == 1){
        this->isConfigurationValid(params, population.getOutputs());
    }

    population.emptyAgent(agent, graph);

    LGP::LGPPopulation& lgpPopulation = dynamic_cast<LGP::LGPPopulation&>(population);
    if(&lgpPopulation == nullptr){
        throw std::invalid_argument("CGPMutator::initRandomAgent: the given population is not a LGPPopulation.");
    }

    const LGP::LgpIndividual& lgpIndividual = dynamic_cast<const LGP::LgpIndividual&>(agent);
    if(&lgpIndividual == nullptr){
        throw std::invalid_argument("CGPMutator::initRandomAgent: the created agent is not a CGPAgent.");
    }

    // insert random constants in the program
    Data::Constant c_value;
    for (int i = 0; i < params.lgp.nbProgramConstant; i++) {
        c_value = {rng.getDouble(params.lgp.minConstValue,
                                 params.lgp.maxConstValue)};
        lgpPopulation.setConstantAt(agent, i, c_value);
    }

    // Compute the number of nodes
    const uint64_t nbLine = params.cgp.nbNodesPerLayer * params.cgp.nbLayers;
    // Insert them
    while (lgpIndividual.getNbLines() < nbLine) {
        this->insertRandomLine(lgpIndividual, lgpPopulation, params, rng);
    }

    for(size_t idx = 0; idx < lgpIndividual.getOutputIndices().size(); idx++) {
        lgpPopulation.setOutputIndex(lgpIndividual, nbLine - 1 - idx, idx);
    }

    // Identify Introns
    lgpPopulation.identifyIntrons(agent);
}

void Representation::CGP::CGPMutator::insertRandomLine(const LGP::LgpIndividual& agent, LGP::LGPPopulation& population, const RepresentationParameters& params, RNG::RNG& rng)
{
    uint64_t lineIndex = agent.getNbLines();
    population.addNewLine(agent, lineIndex);
    
    size_t maxIndex = params.cgp.nbNodesPerLayer * (lineIndex / params.cgp.nbNodesPerLayer);
    this->cgpLineMutator.initRandomCorrectLine(population.getLineForMutation(agent, lineIndex), lineIndex, maxIndex, rng);
}

void Representation::CGP::CGPMutator::crossoverAgents(
    std::array<std::reference_wrapper<const Individual>, 2> agents, EvoGraph::Graph& graph, 
    Population& population, std::vector<std::reference_wrapper<const Individual>>& newSubAgents, 
    const RepresentationParameters& params, RNG::RNG& rng)
{ 
    /// No crossover with CGP
}

bool Representation::CGP::CGPMutator::mutateLgpIndividual(const LGP::LgpIndividual& agent, LGP::LGPPopulation& population, const RepresentationParameters& params, RNG::RNG& rng)
{
    bool anyMutation = false;
    for(size_t idx = 0; idx < params.cgp.nbLayers * params.cgp.nbNodesPerLayer; idx++) {
        if (rng.getDouble(0.0, 1.0) < params.cgp.pMutateNode) {
            anyMutation = true;
            alterRandomlyLine(agent, idx, population, params, rng);
        }
    }

    // mutate the programs constants if they exists
    if (params.lgp.nbProgramConstant > 0 &&
        rng.getDouble(0.0, 1.0) < params.lgp.pConstantMutation) {
        anyMutation = true;
        alterRandomConstant(agent, population, params, rng);
    }

    for(size_t idx = 0; idx < agent.getUsedNbOutputs(population.getOutputs()); idx++) {
        if(rng.getDouble(0.0, 1.0) < params.lgp.pMutateOutput) {
            anyMutation = true;
            alterRandomOutputs(agent, population, idx, params, rng);
        }
    }

    // Identify introns
    if (anyMutation) {
        population.identifyIntrons(agent);
    }
    return anyMutation;
}

bool Representation::CGP::CGPMutator::alterRandomlyLine(
    const LGP::LgpIndividual& agent, size_t lineIndex, LGP::LGPPopulation& population, const RepresentationParameters& params, RNG::RNG& rng)
{
    
    size_t maxIndex = params.cgp.nbNodesPerLayer * (lineIndex / params.cgp.nbNodesPerLayer);
    this->cgpLineMutator.alterCorrectLine(population.getLineForMutation(agent, lineIndex), maxIndex, rng); // specified accessible registers
    return true;
}