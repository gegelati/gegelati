#include "representations/lgpRepresentation.h"

std::unique_ptr<Evolution::Representation> Representations::LGPRepresentation::cloneUniquePtr() const
{
    auto clone = std::make_unique<Representations::LGPRepresentation>(
        this->dimensionFlow.getInputDimensions(),
        this->nbOutputRegisters,
        this->iSet,
        this->nbRegisters,
        this->nbNodesMin,
        this->nbNodesMax,
        this->representationName,
        this->representationColor
    );
    //this->copyOutputFunctionsTo(*clone);
    return clone;
}

void Representations::LGPRepresentation::setGenotypeRequirements() 
{

    size_t maxInputSourceIdx = 8;
    Node::NodeRequirements instructionNodes;

    // Value Requirements for register
    instructionNodes.addValueRequirements(
        Dimensions::Requirement::scalar<size_t>(Dimensions::NumericRange<size_t>::atMost(this->nbRegisters - 1))
    );
    
    // Value requirements for instruction
    instructionNodes.addValueRequirements(
        Dimensions::Requirement::scalar<size_t>(Dimensions::NumericRange<size_t>::atMost(this->iSet.getNbInstructions() - 1))
    );

    // Value requirements for input type and index
    Dimensions::Requirement nbInputReq = Dimensions::Requirement::scalar<size_t>(Dimensions::NumericRange<size_t>::atMost(this->dimensionFlow.getInputDimensions().size() + 1 - 1));
    Dimensions::Requirement maxInputReq = Dimensions::Requirement::scalar<size_t>(Dimensions::NumericRange<size_t>::atMost(maxInputSourceIdx - 1));
    for(size_t idx = 0; idx < this->iSet.getMaxNbOperands(); idx++) {
        instructionNodes.addValueRequirements(Node::NodeValueRequirement(nbInputReq));
        instructionNodes.addValueRequirements(Node::NodeValueRequirement(maxInputReq));
    }

    this->genotypeRequirements.addNodeRequirements(instructionNodes, this->nbNodesMin, this->nbNodesMax);
}

bool Representations::LGPRepresentation::isValid(const Evolution::Individual& indiv) const
{
    /*
    // Return false if genotype length is out of bounds.
    if(indiv.getSize() > this->nbNodesMax || indiv.getSize() < this->nbNodesMin) {
        return false;
    }
    size_t maxInputSourceIdx = 8;

    // Ranges should look like {nbRegister, NbInstr, NbTypeInput, MaxInput, NbTypeInput, MaxInput...}.
    std::vector<size_t> ranges = {this->nbRegisters, this->iSet.getNbInstructions()};
    for(size_t idx = 0; idx < this->iSet.getMaxNbOperands(); idx++) {
        ranges.push_back(this->dimensionFlow.getInputDimensions().size() + 1);
        ranges.push_back(maxInputSourceIdx);
    }

    std::vector<std::vector<std::reference_wrapper<const Node::GPNode>>> effectiveNodes = indiv.getGenotype().getEffectiveNodes();

    // Verify each (effective) node corresponds to the required specifications.
    for(const Node::GPNode& node: effectiveNodes.at(0)) {
        if(node.getSize() != ranges.size()) {
            return false;
        }

        for(size_t idxNode = 0; idxNode < ranges.size(); idxNode++) {
            if(!std::holds_alternative<size_t>(node.getValue(idxNode))) {
                return false;
            }
            if(node.getValue(idxNode) >= Node::NodeValue(ranges.at(idxNode))){
                return false;
            }
        }
    }*/
    return true;
}


Data::DataValue Representations::LGPRepresentation::executeIndividualRaw(
    const Evolution::Individual& indiv, const std::vector<Data::DataView>& inputSources) const
{
    // Get effective nodes
    std::vector<std::vector<std::reference_wrapper<const Node::GPNode>>> effectiveNodes = indiv.getGenotype().getEffectiveNodes();

    /// Registers used as internal memory.
    Data::DataValue registers = Data::DataValue::zeros<double>(Data::DataType::array1d<double>(this->nbRegisters));
    Data::DataView registerView = registers.view();

    for(const Node::GPNode& node: effectiveNodes.at(0)) {

        size_t outputIndex = node.getValue(0).getScalar<size_t>();
        size_t functionIndex = node.getValue(1).getScalar<size_t>();

        const Instructions::Instruction& instruction = this->iSet.getInstruction(functionIndex);
        std::vector<Data::DataView> operands;

        size_t nbOperands = instruction.getNbOperands();
        for(size_t idxOp = 0; idxOp < nbOperands; idxOp++){
            size_t nodeIndex = idxOp * 2 + 2; // +2 is to ignore output and function index, then times too for both type and index
    
            size_t inputType = node.getValue(nodeIndex).getScalar<size_t>();
            size_t inputIndex = node.getValue(nodeIndex + 1).getScalar<size_t>();

            const Data::DataType& operandType = instruction.getOperandTypes().at(idxOp);
            const Data::DataView& dataSource = (inputType==0) ? registerView : inputSources.at(inputType - 1);

            uint64_t operandLocation = dataSource.scaleLocation(operandType, inputIndex);
            operands.push_back(dataSource.getSubView(operandType, operandLocation));

        }

        registers.setSubValue(instruction.execute(operands), outputIndex);
    }


    // GetOutput
    Data::DataValue output = registers.getSubValue<double>(Data::DataType::array1d<double>(this->nbOutputRegisters), 0);

    // Replace Nan values by -inf.
    const double* values = output.getData<double>();
    for(size_t idx = 0; idx < this->nbOutputRegisters; idx++) {
        if(std::isnan(values[idx])) {
            output.setScalarAt<double>(-std::numeric_limits<double>::infinity(), idx);
        }
    }

    // Return value of first register
    return output;
}