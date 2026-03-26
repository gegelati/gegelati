
#include "algorithm/lgp/lgpManager.h"

Algorithm::LGP::LGPAgent& Algorithm::LGP::LGPManager::getLGPAgentFromCst(const Agent& agent)
{
    return dynamic_cast<LGPAgent&>(**this->getAgentFromCst(agent));
}

const Algorithm::Agent& Algorithm::LGP::LGPManager::createAgent(EvoGraph::Graph& graph)
{
    this->agents.insert(std::make_unique<LGPAgent>(this->env, this->outputs, this->getAlgorithmID()));
    return **this->agents.rbegin();
}

const Algorithm::Agent& Algorithm::LGP::LGPManager::copyAgent(const Agent& agent, EvoGraph::Graph& graph)
{
    const LGPAgent* castedAgent = dynamic_cast<const LGPAgent*>(&agent);
    if(castedAgent == nullptr){
        throw std::runtime_error("Algorithm::LGP::LGPManager::copyAgent: trying to copy an agent that is not a LGPAgent.");
    }
    LGPAgent& newAgent = this->getLGPAgentFromCst(this->createAgent(graph));

    for(size_t idx = 0; idx < castedAgent->getNbLines(); idx++){
        newAgent.addNewLine(castedAgent->getLine(idx));
    }

    for(size_t idx = 0; idx < castedAgent->getEnvironment().getParams().nbProgramConstant; idx++){
        this->setConstantAt(newAgent, idx, castedAgent->getConstantAt(idx));
    }

    this->identifyIntrons(newAgent);
    return **this->agents.rbegin();
}

void Algorithm::LGP::LGPManager::emptyAgent(const Agent& agent, EvoGraph::Graph& graph)
{
    Algorithm::LGP::LGPAgent& lgpAgent = this->getLGPAgentFromCst(agent);
    while (lgpAgent.getNbLines() > 0) {
        lgpAgent.removeLine(0);
    }
}

const Output::OutputHandler& Algorithm::LGP::LGPManager::getOutputs() const
{
    return this->outputs;
}

void Algorithm::LGP::LGPManager::setConstantAt(const Agent& agent, size_t index, const Data::Constant& value)
{
    this->getLGPAgentFromCst(agent).getConstantHandler().setDataAt(typeid(Data::Constant), index, value);
}

void Algorithm::LGP::LGPManager::removeLine(const Agent& agent, size_t index)
{
    this->getLGPAgentFromCst(agent).removeLine(index);
}

const Algorithm::LGP::LGPLine& Algorithm::LGP::LGPManager::addNewLine(const Agent& agent, size_t index)
{
    return this->getLGPAgentFromCst(agent).addNewLine(index);
}

const Algorithm::LGP::LGPLine& Algorithm::LGP::LGPManager::addNewLine(const Agent& agent)
{
    Algorithm::LGP::LGPAgent& lgpAgent = this->getLGPAgentFromCst(agent);
    return lgpAgent.addNewLine(lgpAgent.getNbLines());
}

void Algorithm::LGP::LGPManager::addNewLine(const Agent& agent, const LGPLine& newLine)
{
    this->getLGPAgentFromCst(agent).addNewLine(newLine);
}

void Algorithm::LGP::LGPManager::swapLines(const Agent& agent, size_t index1, size_t index2)
{
    this->getLGPAgentFromCst(agent).swapLines(index1, index2);
}

const Algorithm::LGP::LGPLine& Algorithm::LGP::LGPManager::getLine(const Agent& agent, size_t index) const
{
    if(this->containsAgent(agent)){
         return dynamic_cast<const LGPAgent&>(agent).getLine(index);
    } else {
        throw std::runtime_error("Algorithm::LGP::LGPManager::getLine: the given agent is not in the manager.");
    }
}


void Algorithm::LGP::LGPManager::setOutputIndex(const LGPAgent& agent, size_t newOutputIndex, size_t location)
{
    this->getLGPAgentFromCst(agent).setOutputIndex(newOutputIndex, location);
}

Algorithm::LGP::LGPLine& Algorithm::LGP::LGPManager::getLineForMutation(const Agent& agent, size_t index)
{
    return this->getLGPAgentFromCst(agent).getLineForMutation(index);
}

uint64_t Algorithm::LGP::LGPManager::identifyIntrons(const Agent& agent)
{
    LGPAgent& lgpAgent = this->getLGPAgentFromCst(agent);


    // Create fake registers to identify accessed addresses.
    const Data::DataHandler& fakeRegisters =
        this->env.getFakeDataSources().at(0);
    // Number of introns within the Program.
    uint64_t nbIntrons = 0;
    // Set of useful register
    std::set<uint64_t> usefulRegisters;
    for(size_t index: lgpAgent.getOutputIndices()) {
        usefulRegisters.insert(index);
    }
    
    for(int64_t idxLine = static_cast<int64_t>(lgpAgent.getNbLines()) - 1; idxLine >= 0; idxLine--){
        const LGPLine& currentLine = lgpAgent.getLine(static_cast<size_t>(idxLine));

        uint64_t destinationIndex = currentLine.getDestinationIndex();
        auto destinationRegister = usefulRegisters.find(destinationIndex);
        if (destinationRegister != usefulRegisters.end()) {
            // The LGPLine is useful (i.e. not an introns)
            lgpAgent.setIntronValue(static_cast<size_t>(idxLine), false);
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
            lgpAgent.setIntronValue(static_cast<size_t>(idxLine), false);
            nbIntrons++;
        }
    }

    return nbIntrons;
}

bool Algorithm::LGP::LGPManager::hasIdenticalBehavior(const Agent& agent1, const Agent& agent2) const
{
    const LGPAgent* lgpAgent1 = dynamic_cast<const LGPAgent*>(&agent1);
    const LGPAgent* lgpAgent2 = dynamic_cast<const LGPAgent*>(&agent2);
    if(lgpAgent1 == nullptr || lgpAgent2 == nullptr){
        throw std::runtime_error("Algorithm::LGP::LGPManager::hasIdenticalBehavior: one of the agents is not a LGPAgent");
    }

    size_t thisLineIdx = 0;
    size_t otherLineIdx = 0;

    auto nextNonIntronIdx = [](const LGPAgent* lgp, size_t& lineIdx) {
        while (lineIdx < lgp->getNbLines() && lgp->isIntron(lineIdx)) {
            lineIdx++;
        }
    };

    // Look for the first non intron line in both programs
    nextNonIntronIdx(lgpAgent1, thisLineIdx);
    nextNonIntronIdx(lgpAgent2, otherLineIdx);

    // Scan the two programs
    while (thisLineIdx < lgpAgent1->getNbLines() &&
           otherLineIdx < lgpAgent2->getNbLines()) {

        // Check that two non-intron lines were reached
        if (thisLineIdx < lgpAgent1->getNbLines() &&
            otherLineIdx < lgpAgent2->getNbLines()) {

            // Compare the two lines
            const LGPLine& line1 = lgpAgent1->getLine(thisLineIdx);
            const LGPLine& line2 = lgpAgent2->getLine(otherLineIdx);

            if (line1 != line2) {
                return false;
            }

            // Look for the next non intron line in both programs
            thisLineIdx++;
            nextNonIntronIdx(lgpAgent1, thisLineIdx);
            otherLineIdx++;
            nextNonIntronIdx(lgpAgent2, otherLineIdx);
        }
    }

    if ((thisLineIdx < lgpAgent1->getNbLines()) ^
        (otherLineIdx < lgpAgent2->getNbLines())) {
        // XOR: only one of the two program reached its last line
        return false;
    }

    // Check constant values
    for (size_t idx = 0; idx < this->env.getParams().nbProgramConstant; idx++) {
        Data::Constant cst1 = lgpAgent1->getConstantAt(idx);
        Data::Constant cst2 = lgpAgent2->getConstantAt(idx);
        if (cst1 != cst2) {
            return false;
        }
    }

    // Everything was identical, return true
    return true;
}


std::unique_ptr<Algorithm::ExecutionEngine> Algorithm::LGP::LGPManager::createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources, bool isTraining) const
{
    if(dataSources.empty()) {
        dataSources = this->env.getDataSources();
    }
    std::unique_ptr<const Environment> privateEnv =
        std::make_unique<const Environment>(this->env.getInstructionSet(), this->env.getParams(),
                                            dataSources, this->env.getNbContinuousActions());
    return std::make_unique<LGPExecutionEngine>(*privateEnv, this->outputs, this->algorithmID, isTraining);
}


const std::string Algorithm::LGP::LGPManager::lineSeparator("&#92;n");
void Algorithm::LGP::LGPManager::readOperands(std::string& str, LGPLine& line)
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

void Algorithm::LGP::LGPManager::readLines(std::string instructionsStr, const Agent& agent)
{
    LGPAgent& lgpAgent = this->getLGPAgentFromCst(agent);
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
        lgpAgent.addNewLine();
        LGPLine& line = lgpAgent.getLineForMutation(lgpAgent.getNbLines() - 1);

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