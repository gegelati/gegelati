#include "representations/lgpRepresentation.h"

#include "evolution/individual.h"

std::unique_ptr<Evolution::Representation> Representations::LGPRepresentation::cloneOnlyRepresentation() const
{
    auto clone = std::make_unique<Representations::LGPRepresentation>(
        this->dimensionFlow.getInputDimensions(), this->nbOutputRegisters,
        this->iSet, this->nbRegisters,
        this->nbNodesMin, this->nbNodesMax,
        this->representationName, this->representationColor
    );
    return clone;
}
void Representations::LGPRepresentation::setGenotypeConstraint()
{
    size_t maxInputSourceIdx = 8;
    Node::NodeConstraint instructionNodes;

    // Value Requirements for register
    instructionNodes.addConstraint(Dimensions::NumericRange<size_t>::between(0, this->nbRegisters - 1));
    
    // Value requirements for instruction
    instructionNodes.addConstraint(Dimensions::NumericRange<size_t>::between(0, this->iSet.getNbInstructions() - 1));

    // Value requirements for input type and index
    for(size_t idx = 0; idx < this->iSet.getMaxNbOperands(); idx++) {
        instructionNodes.addConstraint(Dimensions::NumericRange<size_t>::between(0, this->dimensionFlow.getInputDimensions().size() + 1 - 1));
        instructionNodes.addConstraint(Dimensions::NumericRange<size_t>::between(0, maxInputSourceIdx - 1));
    }

    this->genotypeConstraint = std::make_unique<Node::GenotypeConstraint>(instructionNodes, this->nbNodesMin, this->nbNodesMax);
}



void Representations::LGPRepresentation::setGenotypeGenerator()
{
    size_t maxInputSourceIdx = 8;
    Node::NodeGenerator instructionNodes;

    // Value Requirements for register
    instructionNodes.addGenerator(Dimensions::NumericUniformGenerator<size_t>(0, this->nbRegisters - 1));
    
    // Value requirements for instruction
    instructionNodes.addGenerator(Dimensions::NumericUniformGenerator<size_t>(0, this->iSet.getNbInstructions() - 1));

    // Value requirements for input type and index
    for(size_t idx = 0; idx < this->iSet.getMaxNbOperands(); idx++) {
        instructionNodes.addGenerator(Dimensions::NumericUniformGenerator<size_t>(0, this->dimensionFlow.getInputDimensions().size() + 1 - 1));
        instructionNodes.addGenerator(Dimensions::NumericUniformGenerator<size_t>(0, maxInputSourceIdx - 1));
    }

    this->genotypeGenerator = std::make_unique<Node::GenotypeGenerator>(instructionNodes, this->nbNodesMin, this->nbNodesMax);
}

std::unique_ptr<Node::GenotypeGenerator> Representations::LGPRepresentation::getGenotypeGenerator() const
{
    return std::move(this->genotypeGenerator->cloneUniquePtr());
}

Data::DataValue Representations::LGPRepresentation::executeGenotype(
    const Evolution::Genotype& genotype, const std::vector<Data::DataView>& inputSources) const
{
    // Get effective nodes
    std::vector<std::vector<std::reference_wrapper<const Node::GPNode>>> effectiveNodes = genotype.getEffectiveNodes();

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