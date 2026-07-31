
#include "representation/lgp/lgpPopulation.h"

Representation::LGP::LgpIndividual& Representation::LGP::LGPPopulation::getLgpIndividualFromCst(const Individual& agent)
{
    return dynamic_cast<LgpIndividual&>(**this->getAgentFromCst(agent));
}

const Representation::Individual& Representation::LGP::LGPPopulation::createAgent(EvoGraph::Graph& graph)
{
    this->agents.insert(std::make_unique<LgpIndividual>(this->env, this->outputs, this->getRepresentationID()));
    return **this->agents.rbegin();
}

const Representation::Individual& Representation::LGP::LGPPopulation::copyAgent(const Individual& agent, EvoGraph::Graph& graph)
{
    const LgpIndividual* castedAgent = dynamic_cast<const LgpIndividual*>(&agent);
    if(castedAgent == nullptr){
        throw std::runtime_error("Representation::LGP::LGPPopulation::copyAgent: trying to copy an agent that is not a LgpIndividual.");
    }
    LgpIndividual& newAgent = this->getLgpIndividualFromCst(this->createAgent(graph));

    for(size_t idx = 0; idx < castedAgent->getNbLines(); idx++){
        newAgent.addNewLine(castedAgent->getLine(idx));
    }

    for(size_t idx = 0; idx < castedAgent->getEnvironment().getNbConstants(); idx++){
        this->setConstantAt(newAgent, idx, castedAgent->getConstantAt(idx));
    }

    for(size_t idx = 0; idx < castedAgent->getOutputIndices().size(); idx++) {
        this->setOutputIndex(newAgent, castedAgent->getOutputIndices().at(idx), idx);
    }

    this->identifyIntrons(newAgent);
    return **this->agents.rbegin();
}

void Representation::LGP::LGPPopulation::clearAgentsIntrons()
{
    for(const auto& agent: this->agents) {
        this->clearAgentIntrons(*agent);
    }
}

void Representation::LGP::LGPPopulation::clearAgentIntrons(const Individual& agent)
{
    Representation::LGP::LgpIndividual& lgpIndividual = this->getLgpIndividualFromCst(agent);
    this->identifyIntrons(agent);

    lgpIndividual.clearIntrons();
}

void Representation::LGP::LGPPopulation::emptyAgent(const Individual& agent, EvoGraph::Graph& graph)
{
    Representation::LGP::LgpIndividual& lgpIndividual = this->getLgpIndividualFromCst(agent);
    while (lgpIndividual.getNbLines() > 0) {
        lgpIndividual.removeLine(0);
    }
}

void Representation::LGP::LGPPopulation::setConstantAt(const Individual& agent, size_t index, const Data::Constant& value)
{
    this->getLgpIndividualFromCst(agent).getConstantHandler().setDataAt(typeid(Data::Constant), index, value);
}

void Representation::LGP::LGPPopulation::removeLine(const Individual& agent, size_t index)
{
    this->getLgpIndividualFromCst(agent).removeLine(index);
}

const Representation::LGP::LGPLine& Representation::LGP::LGPPopulation::addNewLine(const Individual& agent, size_t index)
{
    return this->getLgpIndividualFromCst(agent).addNewLine(index);
}

const Representation::LGP::LGPLine& Representation::LGP::LGPPopulation::addNewLine(const Individual& agent)
{
    Representation::LGP::LgpIndividual& lgpIndividual = this->getLgpIndividualFromCst(agent);
    return lgpIndividual.addNewLine(lgpIndividual.getNbLines());
}

void Representation::LGP::LGPPopulation::addNewLine(const Individual& agent, const LGPLine& newLine)
{
    this->getLgpIndividualFromCst(agent).addNewLine(newLine);
}

void Representation::LGP::LGPPopulation::addNewLine(const Individual& agent, const LGPLine& newLine, size_t index)
{
    this->getLgpIndividualFromCst(agent).addNewLine(newLine, index);
}

void Representation::LGP::LGPPopulation::swapLines(const Individual& agent, size_t index1, size_t index2)
{
    this->getLgpIndividualFromCst(agent).swapLines(index1, index2);
}

const Representation::LGP::LGPLine& Representation::LGP::LGPPopulation::getLine(const Individual& agent, size_t index) const
{
    if(this->containsAgent(agent)){
         return dynamic_cast<const LgpIndividual&>(agent).getLine(index);
    } else {
        throw std::runtime_error("Representation::LGP::LGPPopulation::getLine: the given agent is not in the population.");
    }
}


void Representation::LGP::LGPPopulation::setOutputIndex(const LgpIndividual& agent, size_t newOutputIndex, size_t location)
{
    this->getLgpIndividualFromCst(agent).setOutputIndex(newOutputIndex, location);
}

Representation::LGP::LGPLine& Representation::LGP::LGPPopulation::getLineForMutation(const Individual& agent, size_t index)
{
    return this->getLgpIndividualFromCst(agent).getLineForMutation(index);
}

uint64_t Representation::LGP::LGPPopulation::identifyIntrons(const Individual& agent)
{
    LgpIndividual& lgpIndividual = this->getLgpIndividualFromCst(agent);


    // Create fake registers to identify accessed addresses.
    const Data::DataHandler& fakeRegisters =
        this->env.getFakeDataSources().at(0);
    // Number of introns within the Program.
    uint64_t nbIntrons = 0;
    // Set of useful register
    std::set<uint64_t> usefulRegisters;
    for(size_t index: lgpIndividual.getOutputIndices()) {
        usefulRegisters.insert(index);
    }
    
    for(int64_t idxLine = static_cast<int64_t>(lgpIndividual.getNbLines()) - 1; idxLine >= 0; idxLine--){
        const LGPLine& currentLine = lgpIndividual.getLine(static_cast<size_t>(idxLine));

        uint64_t destinationIndex = currentLine.getDestinationIndex();
        auto destinationRegister = usefulRegisters.find(destinationIndex);
        if (destinationRegister != usefulRegisters.end()) {
            // The LGPLine is useful (i.e. not an introns)
            lgpIndividual.setIntronValue(static_cast<size_t>(idxLine), false);
            // Remove the destination register from the list of useful operands
            usefulRegisters.erase(*destinationRegister);
            // Add register operands to the list of useful registers
            const Instructions::Instruction& instruction =
                this->env.getInstructionSet().getInstruction(
                    currentLine.getInstructionIndex());
            size_t nbOperands = instruction.getNbOperands();
            for (auto idxOperand = 0; idxOperand < nbOperands; idxOperand++) {
                // Is the operand a register (i.e. its index is 0)
                if (currentLine.getOperand(idxOperand).first == 0) {
                    // The operand is a register, add the accessed register to
                    // the list of useful registers.
                    const std::type_info& operandType =
                        instruction.getOperandTypes().at(idxOperand);
                    uint64_t location =
                        currentLine.getOperand(idxOperand).second;
                    uint64_t registerIdx =
                        location % fakeRegisters.getAddressSpace(operandType);
                    std::vector<size_t> accessedAddresses =
                        fakeRegisters.getAddressesAccessed(operandType,
                                                           registerIdx);
                    for (size_t accessedAddress : accessedAddresses) {
                        usefulRegisters.insert(accessedAddress);
                    }
                }
            }
        }
        else {
            // The destination of the line is not within useful registers
            // the line does not contribute to the result of the Program
            // it is an intron.
            lgpIndividual.setIntronValue(static_cast<size_t>(idxLine), true);
            nbIntrons++;
        }
    }

    return nbIntrons;
}

bool Representation::LGP::LGPPopulation::hasIdenticalBehavior(const Individual& agent1, const Individual& agent2) const
{
    const LgpIndividual* lgpIndividual1 = dynamic_cast<const LgpIndividual*>(&agent1);
    const LgpIndividual* lgpIndividual2 = dynamic_cast<const LgpIndividual*>(&agent2);
    if(lgpIndividual1 == nullptr || lgpIndividual2 == nullptr){
        throw std::runtime_error("Representation::LGP::LGPPopulation::hasIdenticalBehavior: one of the agents is not a LgpIndividual");
    }

    size_t thisLineIdx = 0;
    size_t otherLineIdx = 0;

    auto nextNonIntronIdx = [](const LgpIndividual* lgp, size_t& lineIdx) {
        while (lineIdx < lgp->getNbLines() && lgp->isIntron(lineIdx)) {
            lineIdx++;
        }
    };

    // Look for the first non intron line in both programs
    nextNonIntronIdx(lgpIndividual1, thisLineIdx);
    nextNonIntronIdx(lgpIndividual2, otherLineIdx);

    // Scan the two programs
    while (thisLineIdx < lgpIndividual1->getNbLines() &&
           otherLineIdx < lgpIndividual2->getNbLines()) {

        // Check that two non-intron lines were reached
        if (thisLineIdx < lgpIndividual1->getNbLines() &&
            otherLineIdx < lgpIndividual2->getNbLines()) {

            // Compare the two lines
            const LGPLine& line1 = lgpIndividual1->getLine(thisLineIdx);
            const LGPLine& line2 = lgpIndividual2->getLine(otherLineIdx);

            if (line1 != line2) {
                return false;
            }

            // Look for the next non intron line in both programs
            thisLineIdx++;
            nextNonIntronIdx(lgpIndividual1, thisLineIdx);
            otherLineIdx++;
            nextNonIntronIdx(lgpIndividual2, otherLineIdx);
        }
    }

    if ((thisLineIdx < lgpIndividual1->getNbLines()) ^
        (otherLineIdx < lgpIndividual2->getNbLines())) {
        // XOR: only one of the two program reached its last line
        return false;
    }

    // Check constant values
    for (size_t idx = 0; idx < this->env.getNbConstants(); idx++) {
        Data::Constant cst1 = lgpIndividual1->getConstantAt(idx);
        Data::Constant cst2 = lgpIndividual2->getConstantAt(idx);
        if (cst1 != cst2) {
            return false;
        }
    }

    for (size_t idx = 0; idx < lgpIndividual1->getOutputIndices().size(); idx++) {
        if(lgpIndividual1->getOutputIndices().at(idx) != lgpIndividual2->getOutputIndices().at(idx)) {
            return false;
        }
    }

    // Everything was identical, return true
    return true;
}


std::unique_ptr<Representation::ExecutionEngine> Representation::LGP::LGPPopulation::createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources, bool isTraining) const
{
    if(dataSources.empty()) {
        dataSources = this->env.getDataSources();
    }
    std::unique_ptr<const LGPEnvironment> privateEnv =
        std::make_unique<const LGPEnvironment>(this->env.getInstructionSet(), this->env.getNbRegisters(), this->env.getNbConstants(), dataSources);
    return std::make_unique<LGPExecutionEngine>(*privateEnv, this->outputs, this->representationID, isTraining);
}


const std::string Representation::LGP::LGPPopulation::lineSeparator("&#92;n");
void Representation::LGP::LGPPopulation::readOperands(std::string& str, LGPLine& line)
{
    std::string::size_type pos = 0;
    std::string::size_type pos2;
    std::string::size_type pos3;

    uint64_t o_idx = 0;
    uint64_t dataIndex = 0;
    uint64_t location = 0;

    // operands are stored in str with the following format :
    // op1_param1|op1_param2#...|param_N

    for (int i = 0; i < this->env.getMaxNbOperands(); ++i) {
        pos2 = str.find("|");
        dataIndex = std::stoi(str.substr(pos, pos2));
        pos2++; // skip the '|'
        pos3 = str.find("#");
        location = std::stoi(str.substr(pos2, pos3 - pos2));
        str = str.substr(pos3 + 1,
                         str.size() -
                             pos3); // store the rest of the string and iterate

        line.setOperand(o_idx, dataIndex, location, true);
        o_idx++;
    }
}

void Representation::LGP::LGPPopulation::readLines(std::string instructionsStr, const Individual& agent)
{
    LgpIndividual& lgpIndividual = this->getLgpIndividualFromCst(agent);
    // a line is stored in the .dot file with the following format
    // inst_idx|dest_idx&op1_param1|op1_param2#...#opN_param1|opN_param2

    // stores the whole agent
    std::string instruction;

    // used to seek delimiters in the variable "instruction"
    std::string::size_type pos;
    std::string::size_type pos1;
    std::string::size_type pos2;

    // instruction index of a line
    uint64_t instructionIdx;
    // destination index of a line
    uint64_t destinationIdx;
    // store operands in a new string
    std::string operands;

    // as long as there are lines in the agent, parse those lines
    bool cont = true;
    while (cont) {

        // Create a line and get the line created.
        lgpIndividual.addNewLine();
        LGPLine& line = lgpIndividual.getLineForMutation(lgpIndividual.getNbLines() - 1);

        pos = instructionsStr.find(this->lineSeparator);
        instruction = instructionsStr.substr(0, pos);
        instructionsStr = instructionsStr.substr(pos + this->lineSeparator.size(),
                                instructionsStr.size());

        // extract everything before pos (ie |)
        // corresponds to instruction index
        pos1 = instruction.find("|");
        instructionIdx = std::stoi(instruction.substr(0, pos1));

        // extract destination index;
        pos2 = instruction.find("&");
        pos1++; // skip the '|'
        destinationIdx = std::stoi(instruction.substr(
            pos1, pos2 - pos1)); // extract and convert to int

        // extract operands as a string
        pos2++; // skip the '$'
        operands = instruction.substr(pos2, instruction.size());

        // add indexes to line
        line.setInstructionIndex(instructionIdx);
        line.setDestinationIndex(destinationIdx);

        // parse operands
        readOperands(operands, line);
        if (instructionsStr.size() <= 3) {
            
            cont = false;
        }
    }
    this->identifyIntrons(agent);
}