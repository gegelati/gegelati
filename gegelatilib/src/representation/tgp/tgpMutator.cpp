
#include <array>
#include "representation/tgp/tgpMutator.h"

bool Representation::TGP::TGPMutator::isConfigurationValid(const RepresentationParameters& params, const Output::OutputHandler& outputs) const
{

    if(outputs.sizeContinuous() != 0 && outputs.sizeDiscrete() != 0){
        throw std::runtime_error("TGPMutator::initRandomPopulation: TGP does not support mixed discrete and continuous outputs.");
    } else if (outputs.sizeContinuous() == 0 && outputs.sizeDiscrete() == 0) {
        throw std::runtime_error("TGPMutator::initRandomPopulation: No outputs defined.");
    }
    return true;
}

void Representation::TGP::TGPMutator::initRandomSpecificIndividual(const Individual& individual, EvoGraph::Graph& graph, Population& population, const RepresentationParameters& params, RNG::RNG& rng)
{
    // If first individual, check validity
    if(population.getIndividuals().size() == 1){
        this->isConfigurationValid(params, population.getOutputs());
    }

    population.emptyIndividual(individual, graph);

    LGP::LGPPopulation& lgpPopulation = dynamic_cast<LGP::LGPPopulation&>(population);
    if(&lgpPopulation == nullptr){
        throw std::invalid_argument("TGPMutator::initRandomIndividual: the given population is not a LGPPopulation.");
    }

    const LGP::LgpIndividual& lgpIndividual = dynamic_cast<const LGP::LgpIndividual&>(individual);
    if(&lgpIndividual == nullptr){
        throw std::invalid_argument("TGPMutator::initRandomIndividual: the created individual is not a TGPIndividual.");
    }

    // insert random constants in the program
    Data::Constant c_value;
    for (int i = 0; i < params.lgp.nbProgramConstant; i++) {
        c_value = {rng.getDouble(params.lgp.minConstValue,
                                 params.lgp.maxConstValue)};
        lgpPopulation.setConstantAt(individual, i, c_value);
    }

    // Insert line, TGP will recursively call this method to create a whole TGP graph.
    this->insertRandomSubTree(lgpIndividual, 0, params.tgp.maxInitDepth, lgpPopulation, params, rng);

    // Identify Introns
    lgpPopulation.identifyIntrons(individual);
}

void Representation::TGP::TGPMutator::insertRandomSubTree(const LGP::LgpIndividual& individual, size_t destinationIndexLine, size_t maxDepthTree, LGP::LGPPopulation& population, const RepresentationParameters& params, RNG::RNG& rng)
{
    std::vector<size_t> lineDestinationIndexToInsert;
    lineDestinationIndexToInsert.push_back(destinationIndexLine);

    while(lineDestinationIndexToInsert.size() > 0) {
        destinationIndexLine = lineDestinationIndexToInsert.front();
        lineDestinationIndexToInsert.erase(lineDestinationIndexToInsert.begin());
        // Always add a line at index 0, because lines are inserted in the opposite order as the distribution
        size_t lineIndex = 0;        
        const LGP::LGPLine& line = population.addNewLine(individual, 0);
        
        // If current index is at max depth, don't allows the line to select registers.
        
        bool maxDepthReached = (this->getNodeDepth(destinationIndexLine) == maxDepthTree - 1);

        this->tgpLineMutator.initRandomCorrectLine(population.getLineForMutation(individual, lineIndex), destinationIndexLine, maxDepthReached, rng);

        for(size_t idx = 0; idx < params.tgp.maxNbEdgePerNode; idx++) {
            // Operand is a register
            if(line.getOperand(idx).first == 0) {
                lineDestinationIndexToInsert.push_back(line.getOperand(idx).second);
            }
        }
    }
}

void Representation::TGP::TGPMutator::crossoverIndividuals(
    std::array<std::reference_wrapper<const Individual>, 2> individuals, EvoGraph::Graph& graph, 
    Population& population, std::vector<std::reference_wrapper<const Individual>>& newSubIndividuals, 
    const RepresentationParameters& params, RNG::RNG& rng)
{ 

    // Casted individual 1 and 2
    const LGP::LgpIndividual& lgpIndividual1 = dynamic_cast<const LGP::LgpIndividual&>(individuals[0].get());
    const LGP::LgpIndividual& lgpIndividual2 = dynamic_cast<const LGP::LgpIndividual&>(individuals[1].get());
    auto lgpIndividuals = std::array<std::reference_wrapper<const LGP::LgpIndividual>, 2>{lgpIndividual1, lgpIndividual2};
    if(&lgpIndividual1 == nullptr || &lgpIndividual2 == nullptr){
        throw std::invalid_argument("LGPMutator::crossoverIndividuals: the given individuals are not LgpIndividuals.");
    }
    LGP::LGPPopulation& lgpPopulation = dynamic_cast<LGP::LGPPopulation&>(population);
    if(&lgpPopulation == nullptr){
        throw std::invalid_argument("LGPMutator::initRandomIndividual: the given population is not a LGPPopulation.");
    }

    if(lgpIndividual1.getNbLines() < 2 || lgpIndividual2.getNbLines() < 2){
        return; // Crossover cannot be done if a program contains less than two lines
    }
    
    /// get node with tree size below or equal too maxDepth / 2
    std::array<size_t, 2> chosenIndexNodes;

    size_t maxDepthCross = params.tgp.maxDepth / 2;

    std::array<std::map<size_t, std::reference_wrapper<const LGP::LGPLine>>, 2> linesToCross;
    std::array<std::set<size_t>, 2> removedIdxLines;
    for(size_t idxIndividual = 0; idxIndividual < 2; idxIndividual++) {
        const LGP::LgpIndividual& curIndividual = lgpIndividuals.at(idxIndividual);
        std::vector<size_t> possibleNodes;
        size_t nbLines = curIndividual.getNbLines();
        for(size_t idx = 0; idx < nbLines; idx++) {
            size_t destIndex = curIndividual.getLine(idx).getDestinationIndex();
            if(this->getRealNodeDepth(curIndividual, destIndex) < maxDepthCross && this->getNodeDepth(destIndex) + maxDepthCross <= params.tgp.maxDepth && destIndex != 0) {
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

            size_t idxLine = this->getIndexLineFromDest(curIndividual, destIndex);
            const LGP::LGPLine& lineCrossed = curIndividual.getLine(idxLine);
            linesToCross.at(idxIndividual).insert({destIndex, lineCrossed});
            removedIdxLines.at(idxIndividual).insert(idxLine);

            std::vector<bool> subTree = this->hasSubTree(curIndividual, idxLine);
            for(size_t idxOp = 0; idxOp < params.tgp.maxNbEdgePerNode; idxOp ++ ) {
                if(subTree.at(idxOp)) {
                    destinationIndexToInclude.push_back(lineCrossed.getOperand(idxOp).second);
                }
            }
        }
    }

    for(size_t idxIndividual = 0; idxIndividual < 2; idxIndividual++) {
        // Get individual and lines of the other individual
        const LGP::LgpIndividual& curIndividual = lgpIndividuals.at(idxIndividual);
        const LGP::LgpIndividual& otherIndividual = lgpIndividuals.at(1 - idxIndividual);
        const auto& mapLines = linesToCross.at(1 - idxIndividual);

        // Add lines in opposite order.
        auto itLines = mapLines.rbegin();
        while(itLines != mapLines.rend()) {
            lgpPopulation.addNewLine(curIndividual, itLines->second, 0);
            itLines++;
        }
        this->changeNodeIndex(curIndividual, lgpPopulation, 0, linesToCross.at(idxIndividual).begin()->first);
    }
    for(size_t idxIndividual = 0; idxIndividual < 2; idxIndividual++) {

        auto itRemovedIdx = removedIdxLines.at(idxIndividual).rbegin();
        while(itRemovedIdx != removedIdxLines.at(idxIndividual).rend()) {
            // Remove the old lines
            lgpPopulation.removeLine(lgpIndividuals.at(idxIndividual), *itRemovedIdx + linesToCross.at(1 - idxIndividual).size());
            itRemovedIdx++;
        }

        lgpPopulation.identifyIntrons(lgpIndividuals.at(idxIndividual));
    }
}

bool Representation::TGP::TGPMutator::mutateLgpIndividual(const LGP::LgpIndividual& individual, LGP::LGPPopulation& population, const RepresentationParameters& params, RNG::RNG& rng)
{
    bool anyMutation = false;
    if (rng.getDouble(0.0, 1.0) < params.lgp.pMutate) {
        anyMutation = true;
        alterRandomLine(individual, population, params, rng);
    }

    // mutate the programs constants if they exists
    if (params.lgp.nbProgramConstant > 0 &&
        rng.getDouble(0.0, 1.0) < params.lgp.pConstantMutation) {
        anyMutation = true;
        alterRandomConstant(individual, population, params, rng);
    }

    if(individual.getUsedNbOutputs(population.getOutputs()) > 1) {
        for(size_t idx = 0; idx < individual.getUsedNbOutputs(population.getOutputs()); idx++) {
    
            size_t nbZeroRegUsed = std::count_if(
                individual.getOutputIndices().begin(), individual.getOutputIndices().end(), []
                (size_t outputIdx) {return outputIdx == 0;});
    
            if(nbZeroRegUsed == 0) {
                throw std::runtime_error("TGPMutator::mutateLgpIndividual: zero output index should always be used at least once.");
            }
    
            // One of the output must always be zero, so mutation is allowed if zero is used multiple time, or if current idx is not using zero (meaning it is used elsewhere)
            if((nbZeroRegUsed > 1 || individual.getOutputIndices().at(idx) != 0) && rng.getDouble(0.0, 1.0) < params.lgp.pMutateOutput) {
                alterRandomOutputs(individual, population, idx, params, rng);
            }
        }

    }

    // Identify introns
    if (anyMutation) {
        population.identifyIntrons(individual);
    }
    return anyMutation;
}


bool Representation::TGP::TGPMutator::alterRandomOutputs(const LGP::LgpIndividual& individual, LGP::LGPPopulation& population, size_t location,
                                              const RepresentationParameters& params, RNG::RNG& rng)
{

    std::set<size_t> availableRegisters;
    for(size_t idx = 0; idx < individual.getNbLines(); idx++) {
        availableRegisters.insert(individual.getLine(idx).getDestinationIndex());
    }

    auto it = availableRegisters.begin();
    std::advance(it, rng.getUnsignedInt64(0, availableRegisters.size() - 1));
    population.setOutputIndex(individual, *it, location);
    return true;
}

std::vector<bool> Representation::TGP::TGPMutator::hasSubTree(const LGP::LgpIndividual& individual, size_t idx) {
    std::vector<bool> result;

    const LGP::LGPLine& line = individual.getLine(idx);
    for(size_t idxOp = 0; idxOp < line.getEnvironment().getMaxNbOperands(); idxOp++) {
        // Operand is a register
        result.push_back(line.getOperand(idxOp).first == 0);
    }
    return result;
}

size_t Representation::TGP::TGPMutator::getIndexLineFromDest(const LGP::LgpIndividual& individual, size_t destIdx) {
    size_t idxLine = 0;
    while(individual.getLine(idxLine).getDestinationIndex() != destIdx) {
        idxLine++;
        if(idxLine == individual.getNbLines()) {
            throw std::runtime_error("TGPMutator::getIndexLineFromDest: line not found");
        }
    }
    return idxLine;
}

size_t Representation::TGP::TGPMutator::getNodeDepth(size_t destIndex) {
    return std::floor(std::log2(destIndex + 1));
}

size_t Representation::TGP::TGPMutator::getRealNodeDepth(const LGP::LgpIndividual& individual, size_t destIndex) {
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

        const LGP::LGPLine& line = individual.getLine(this->getIndexLineFromDest(individual, destIndex));

        std::vector<bool> subTree = this->hasSubTree(individual, this->getIndexLineFromDest(individual, destIndex));
        for(size_t idxOp = 0; idxOp < subTree.size(); idxOp ++ ) {
            if(subTree.at(idxOp)) {
                destinationIndexToInclude.push_back(line.getOperand(idxOp).second);
            }
        }
    }

    return highestDepth - this->getNodeDepth(destIndex);
}

void Representation::TGP::TGPMutator::destroySubTree(const LGP::LgpIndividual& individual, size_t idxSubTree, LGP::LGPPopulation& population) 
{
    // Recursively iterate to erase the lines in the hierarchy
    std::vector<size_t> destinationIdxToDestroy;
    destinationIdxToDestroy.push_back(idxSubTree);
    while(destinationIdxToDestroy.size() > 0) {
        size_t destinationIdx = destinationIdxToDestroy.front();
        destinationIdxToDestroy.erase(destinationIdxToDestroy.begin());

        size_t idxLine = this->getIndexLineFromDest(individual, destinationIdx);

        const LGP::LGPLine& destroyedLine = individual.getLine(idxLine);
        std::vector<bool> result = this->hasSubTree(individual, idxLine);
        for(size_t idx = 0; idx < result.size(); idx++) {
            // Operand is a register
            if(result.at(idx)) {
                destinationIdxToDestroy.push_back(destroyedLine.getOperand(idx).second);
            }
        }
        population.removeLine(individual, idxLine);
    }
}

bool Representation::TGP::TGPMutator::alterRandomLine(const LGP::LgpIndividual& individual, LGP::LGPPopulation& population, 
                                              const RepresentationParameters& params, RNG::RNG& rng)
{
    if (individual.getNbLines() < 1) {
        return false;
    }
    // Select a random index.
    const uint64_t lineIndex = rng.getUnsignedInt64(0, individual.getNbLines() - 1);
    const LGP::LGPLine& line = individual.getLine(lineIndex);
    
    std::vector<bool> oldSubTree = this->hasSubTree(individual, lineIndex);

    // If current index is at max depth, don't allows the line to select registers.
    bool maxDepthReached = (this->getNodeDepth(line.getDestinationIndex()) == params.tgp.maxDepth - 1);
    this->tgpLineMutator.alterCorrectLine(population.getLineForMutation(individual, lineIndex), maxDepthReached, rng); // specified accessible registers

    if(!maxDepthReached) {
        std::vector<bool> newSubTree = this->hasSubTree(individual, lineIndex);
        for(size_t idx = 0; idx < newSubTree.size(); idx++) {
            size_t idxSubTree = params.tgp.maxNbEdgePerNode * line.getDestinationIndex() + 1 + idx;
            if(oldSubTree[idx] && !newSubTree[idx]) {
                // Destroy old sub tree
                this->destroySubTree(individual, idxSubTree, population);
            } else if(!oldSubTree[idx] && newSubTree[idx]) {
                // Create new sub tree
                this->insertRandomSubTree(individual, idxSubTree, params.tgp.maxDepth, population,  params, rng);
            }
        }
    }
    return true;
} 

void Representation::TGP::TGPMutator::changeNodeIndex(const LGP::LgpIndividual& individual, LGP::LGPPopulation& population, size_t lineIndex, size_t destIndex)
{
    const LGP::LGPLine& line = individual.getLine(lineIndex);
    if(line.getDestinationIndex() != destIndex) {

        std::vector<bool> subTree = this->hasSubTree(individual, lineIndex);
        for(size_t idx = 0; idx < subTree.size(); idx++) {
            if(subTree.at(idx)) {
                this->changeNodeIndex(individual, population, this->getIndexLineFromDest(individual, individual.getLine(lineIndex).getOperand(idx).second), subTree.size() * destIndex + 1 + idx);
                bool success = population.getLineForMutation(individual, lineIndex).setOperand(idx, 0, subTree.size() * destIndex + 1 + idx);
                if(!success) {
                    throw std::runtime_error("TGPMutator::changeNodeIndex: operand of sub tree modification did not success.");
                }
            }
        }
        population.getLineForMutation(individual, lineIndex).setDestinationIndex(destIndex);
    }
}
