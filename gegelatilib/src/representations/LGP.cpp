#include "representations/LGP.h"

#include "individual.h"

std::unique_ptr<Representations::Representation> Representations::LGP::cloneOnlyRepresentation() const
{
    auto clone = std::make_unique<Representations::LGP>(
        this->dimensionFlow.getInputDimensions(), this->registersType,
        this->iSet, this->outputRegistersType,
        this->nbLinesMin, this->nbLinesMax,
        this->getName(), this->representationColor
    );
    return clone;
}

const Instructions::Set& Representations::LGP::getInstructionSet()
{
    return this->iSet;
}

void Representations::LGP::setLargestAddressSpace()
{
    this->largestAddressSpace = 0;
    for(size_t idx = 0; idx < this->iSet.getNbInstructions(); idx++) {
        const Instructions::Instruction& instruction = this->iSet.getInstruction(idx);
        for(const Data::DataType& instrType: instruction.getOperandTypes()) {
            for(const Dimensions::Requirement& inputReq: this->dimensionFlow.getInputDimensions()) {
                this->largestAddressSpace = std::max(this->largestAddressSpace, inputReq.getDataType().getAddressSpace(instrType));
            }
            this->largestAddressSpace = std::max(this->largestAddressSpace, this->registersType.getAddressSpace(instrType));
        }
    }
}

void Representations::LGP::setGenotypeConstraint()
{
    GraphBased::NodeConstraint instructionNodes;

    // Value Requirements for register
    instructionNodes.addConstraint(Dimensions::NumericRange<size_t>::between(0, this->registersType.totalElements() - 1));
    
    // Value requirements for instruction
    instructionNodes.addConstraint(Dimensions::NumericRange<size_t>::between(0, this->iSet.getNbInstructions() - 1));

    // Value requirements for input type and index
    for(size_t idx = 0; idx < this->iSet.getMaxNbOperands(); idx++) {
        instructionNodes.addConstraint(Dimensions::NumericRange<size_t>::between(0, this->dimensionFlow.getInputDimensions().size() + 1 - 1));
        instructionNodes.addConstraint(Dimensions::NumericRange<size_t>::between(0, this->largestAddressSpace - 1));
    }

    this->genotypeConstraint = std::make_unique<GraphBased::GenotypeConstraint>(instructionNodes, this->nbLinesMin, this->nbLinesMax);
}



void Representations::LGP::detectIntronNodes(GraphBased::Genotype& genotype) const
{
    if(genotype.getSize() == 0) {
        return;
    }

    GraphBased::NodeGroup& instructions = genotype.getNodeGroup(0);
    std::vector<bool> usefulRegisters(this->registersType.totalElements(), false);
    for(size_t idx = 0; idx < this->outputRegistersType.totalElements(); idx++) {
        usefulRegisters[idx] = true;
    }

    const std::vector<Dimensions::Requirement>& inputDimensions = this->dimensionFlow.getInputDimensions();
    const size_t nbSources = inputDimensions.size() + 1;
    for(size_t reverseIdx = instructions.getSize(); reverseIdx > 0; reverseIdx--) {
        GraphBased::GPNode& node = instructions.getNode(reverseIdx - 1);
        const size_t outputIndex = node.getValue(0).getScalar<size_t>();
        const size_t functionIndex = node.getValue(1).getScalar<size_t>();
        const Instructions::Instruction& instruction = this->iSet.getInstruction(functionIndex);
        const Data::DataType outputType = instruction.getOutputType();
        const size_t outputAddressSpace = this->registersType.getAddressSpace(outputType);
        const size_t outputLocation = outputIndex % outputAddressSpace;
        const size_t outputSize = outputType.totalElements();

        bool isUseful = false;
        for(size_t idx = 0; idx < outputSize; idx++) {
            isUseful = isUseful || usefulRegisters[outputLocation + idx];
            usefulRegisters[outputLocation + idx] = false;
        }
        node.setIsIntron(!isUseful);

        if(!isUseful) {
            continue;
        }

        for(size_t idxOperand = 0; idxOperand < instruction.getNbOperands(); idxOperand++) {
            const size_t valueIndex = idxOperand * 2 + 2;
            const size_t encodedSource = node.getValue(valueIndex).getScalar<size_t>();
            const size_t inputIndex = node.getValue(valueIndex + 1).getScalar<size_t>();
            const Data::DataType& operandType = instruction.getOperandTypes().at(idxOperand);

            for(size_t offset = 0; offset < nbSources; offset++) {
                const size_t sourceIndex = (encodedSource + offset) % nbSources;
                const Data::DataType& sourceType = sourceIndex == 0
                    ? this->registersType
                    : inputDimensions.at(sourceIndex - 1).getDataType();
                const size_t addressSpace = sourceType.getAddressSpace(operandType);
                if(addressSpace == 0) {
                    continue;
                }

                if(sourceIndex == 0) {
                    const size_t inputLocation = inputIndex % addressSpace;
                    for(size_t idx = 0; idx < operandType.totalElements(); idx++) {
                        usefulRegisters[inputLocation + idx] = true;
                    }
                }
                break;
            }
        }
    }
}

Data::DataValue Representations::LGP::executeGenotype(
    const GraphBased::Genotype& genotype, const std::vector<Data::DataView>& inputSources) const
{
    // Effective nodes are iterated in place, no intermediate container.
    const GraphBased::NodeGroup& instructionGroup = genotype.getNodeGroup(0);
    const size_t nbSources = this->dimensionFlow.getInputDimensions().size() + 1;

    /// Registers used as internal memory.
    Data::DataValue registers = Data::DataValue::zeros<double>(this->registersType);
    Data::DataView registerView = registers.view();

    std::vector<Data::DataView> operands;
    operands.reserve(this->iSet.getMaxNbOperands());
    size_t nbExecuted = 0;

    for(size_t idxNode = 0; idxNode < instructionGroup.getSize(); idxNode++) {
        const GraphBased::GPNode& node = instructionGroup.getNode(idxNode);
        if(node.getIsIntron()) {
            continue;
        }

        size_t outputIndex = node.getValue(0).getScalar<size_t>();
        size_t functionIndex = node.getValue(1).getScalar<size_t>();

        const Instructions::Instruction& instruction = this->iSet.getInstruction(functionIndex);
        operands.clear();

        size_t nbOperands = instruction.getNbOperands();
        for(size_t idxOp = 0; idxOp < nbOperands; idxOp++){
            size_t nodeIndex = idxOp * 2 + 2; // +2 is to ignore output and function index, then times too for both type and index
    
            size_t inputType = node.getValue(nodeIndex).getScalar<size_t>();
            size_t inputIndex = node.getValue(nodeIndex + 1).getScalar<size_t>();

            const Data::DataType& operandType = instruction.getOperandTypes()[idxOp];
            const Data::DataView* dataSource = (inputType==0) ? &registerView : &inputSources.at(inputType - 1);
            size_t addressSpace = dataSource->getType().getAddressSpace(operandType);
            while(addressSpace == 0) {
                inputType = (inputType + 1) % nbSources;
                dataSource = (inputType==0) ? &registerView : &inputSources.at(inputType - 1);
                addressSpace = dataSource->getType().getAddressSpace(operandType);
            }
            // Same as DataView::scaleLocation, reusing the address space already computed.
            size_t operandLocation = inputIndex % addressSpace;
            if(dataSource->getType().rank >= 2) {
                operandLocation = dataSource->getType().getValidAddress(operandType, operandLocation);
            }
            operands.push_back(dataSource->getSubView(operandType, operandLocation));
        }

        size_t outputLocation = registers.scaleLocation(instruction.getOutputType(), outputIndex);
        instruction.executeInto(operands, registers, outputLocation);
        nbExecuted++;
    }


    // GetOutput
    //Data::DataValue output = registers.getSubValue<double>(Data::DataType::array1d<double>(this->nbOutputRegisters), 0);
    
    
    // Replace Nan values by -inf.
    /*std::shared_ptr<const double[]> values = registers.getData<double>();
    for(size_t idx = 0; idx < this->nbOutputRegisters; idx++) {
        if(std::isnan(values[idx])) {
            registers.setScalarAt<double>(-std::numeric_limits<double>::infinity(), idx);
        }
    }*/
    
    // Return value of first register
    return registers.getSubValue<double>(this->outputRegistersType, 0);
}