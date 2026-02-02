
#include "algorithm/lgp/lgpManager.h"

std::shared_ptr<Algorithm::LGP::LGPAgent> Algorithm::LGP::LGPManager::getLGPAgentFromCst(std::shared_ptr<const Agent> agent)
{
    auto iterator = this->agents.find(agent);
    if(iterator == this->agents.end() || *iterator != agent){
        throw std::invalid_argument("LGPManager::getLGPAgentFromCst: the given agent is not managed by this manager.");
    }

    return std::dynamic_pointer_cast<LGPAgent>(*iterator);
}

std::shared_ptr<const Algorithm::LGP::LGPAgent> Algorithm::LGP::LGPManager::cGetLGPAgentFromCst(std::shared_ptr<const Agent> agent) const
{
    auto iterator = this->agents.find(agent);
    if(iterator == this->agents.end() || *iterator != agent){
        throw std::invalid_argument("LGPManager::cGetLGPAgentFromCst: the given agent is not managed by this manager.");
    }

    return std::dynamic_pointer_cast<const LGPAgent>(*iterator);
}

std::shared_ptr<const Algorithm::Agent> Algorithm::LGP::LGPManager::createAgent(std::shared_ptr<EvoGraph::Graph> graph)
{
    this->agents.insert(std::make_shared<LGPAgent>(this->env, this->outputs, this->getAlgorithmName()));
    return *this->agents.rbegin();
}

std::shared_ptr<const Algorithm::Agent> Algorithm::LGP::LGPManager::copyAgent(std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph)
{
    auto castedAgent = this->getLGPAgentFromCst(agent);
    auto newAgent = this->getLGPAgentFromCst(this->createAgent(graph));

    for(size_t idx = 0; idx < castedAgent->getNbLines(); idx++){
        newAgent->addNewLine(castedAgent->getLine(idx));
    }

    for(size_t idx = 0; idx < castedAgent->getEnvironment()->getParams().nbProgramConstant; idx++){
        this->setConstantAt(newAgent, idx, castedAgent->getConstantAt(idx));
    }

    this->identifyIntrons(newAgent);
    this->agents.insert(newAgent);
    return *this->agents.rbegin();
}

void Algorithm::LGP::LGPManager::deleteAgent(std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph)
{
    this->agents.erase(this->getLGPAgentFromCst(agent));   
}

const Output::OutputHandler& Algorithm::LGP::LGPManager::getOutputs() const
{
    return this->outputs;
}

void Algorithm::LGP::LGPManager::setConstantAt(std::shared_ptr<const Agent> agent, size_t index, const Data::Constant& value)
{
    this->getLGPAgentFromCst(agent)->getConstantHandler().setDataAt(typeid(Data::Constant), index, value);
}

void Algorithm::LGP::LGPManager::removeLine(std::shared_ptr<const LGPAgent> agent, size_t index)
{
    this->getLGPAgentFromCst(agent)->removeLine(index);
}

Algorithm::LGP::LGPLine& Algorithm::LGP::LGPManager::addNewLine(std::shared_ptr<const LGPAgent> agent, size_t index)
{
    return this->getLGPAgentFromCst(agent)->addNewLine(index);
}

Algorithm::LGP::LGPLine& Algorithm::LGP::LGPManager::addNewLine(std::shared_ptr<const LGPAgent> agent)
{
    return this->getLGPAgentFromCst(agent)->addNewLine(agent->getNbLines());
}

void Algorithm::LGP::LGPManager::addNewLine(std::shared_ptr<const LGPAgent> agent, const LGPLine& newLine)
{
    this->getLGPAgentFromCst(agent)->addNewLine(newLine);
}

void Algorithm::LGP::LGPManager::swapLines(std::shared_ptr<const LGPAgent> agent, size_t index1, size_t index2)
{
    this->getLGPAgentFromCst(agent)->swapLines(index1, index2);
}

Algorithm::LGP::LGPLine& Algorithm::LGP::LGPManager::getLine(std::shared_ptr<const LGPAgent> agent, size_t index)
{
    return this->getLGPAgentFromCst(agent)->getLine(index);
}

uint64_t Algorithm::LGP::LGPManager::identifyIntrons(std::shared_ptr<const Agent> agent)
{
    std::shared_ptr<LGPAgent> lgpAgent = this->getLGPAgentFromCst(agent);


    // Create fake registers to identify accessed addresses.
    const Data::DataHandler& fakeRegisters =
        this->env->getFakeDataSources().at(0);
    // Number of introns within the Program.
    uint64_t nbIntrons = 0;
    // Set of useful register
    std::set<uint64_t> usefulRegisters;
    for(size_t idx = 0; idx < this->outputs.size(); idx++) {
        usefulRegisters.insert(idx);
    }
    
    for(int64_t idxLine = static_cast<int64_t>(lgpAgent->getNbLines()) - 1; idxLine >= 0; idxLine--){
        LGPLine& currentLine = lgpAgent->getLine(static_cast<size_t>(idxLine));

        uint64_t destinationIndex = currentLine.getDestinationIndex();
        auto destinationRegister = usefulRegisters.find(destinationIndex);
        if (destinationRegister != usefulRegisters.end()) {
            // The LGPLine is useful (i.e. not an introns)
            lgpAgent->setIntronValue(static_cast<size_t>(idxLine), false);
            // Remove the destination register from the list of useful operands
            usefulRegisters.erase(*destinationRegister);
            // Add register operands to the list of useful registers
            const Instructions::Instruction& instruction =
                this->env->getInstructionSet().getInstruction(
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
            lgpAgent->setIntronValue(static_cast<size_t>(idxLine), true);
            nbIntrons++;
        }
    }

    return nbIntrons;
}

bool Algorithm::LGP::LGPManager::hasIdenticalBehavior(std::shared_ptr<const Agent> agent1, std::shared_ptr<const Agent> agent2) const
{
    std::shared_ptr<const LGPAgent> lgpAgent1 = this->cGetLGPAgentFromCst(agent1);
    std::shared_ptr<const LGPAgent> lgpAgent2 = this->cGetLGPAgentFromCst(agent2);

    size_t thisLineIdx = 0;
    size_t otherLineIdx = 0;

    auto nextNonIntronIdx = [](std::shared_ptr<const LGPAgent> lgp, size_t& lineIdx) {
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
    for (size_t idx = 0; idx < this->env->getParams().nbProgramConstant; idx++) {
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
        dataSources = this->env->getDataSources();
    }
    std::shared_ptr<const Environment> privateEnv =
        std::make_shared<const Environment>(this->env->getInstructionSet(), this->env->getParams(),
                                            dataSources, this->env->getNbContinuousActions());
    return std::make_unique<LGPExecutionEngine>(*privateEnv, this->outputs, this->algorithmName, isTraining);
}