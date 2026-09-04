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
std::unique_ptr<const Node::GenotypeTemplate> Representations::LGPRepresentation::getGenotypeTemplate() const
{


    size_t maxInputSourceIdx = 8;

    // Instruction node template is fixed during evolution, so created only once.
    if(this->instructionNodesTemplate->size() == 0) {

        // Value Template for register
        std::shared_ptr<Node::NodeValueConfiguration> configRegister(
            std::make_shared<Node::NodeValueConfiguration>(std::make_pair(size_t(0), this->nbRegisters)));
        this->instructionNodesTemplate->addValueTemplate(std::make_shared<Node::NodeValueTemplate>(configRegister));
        
        // Value template for instruction
        std::shared_ptr<Node::NodeValueConfiguration> configFunction(
            std::make_shared<Node::NodeValueConfiguration>(std::make_pair(size_t(0), size_t(this->iSet.getNbInstructions()))));
        this->instructionNodesTemplate->addValueTemplate(std::make_shared<Node::NodeValueTemplate>(configFunction));
    
        // Value templates for input type and index
        std::shared_ptr<Node::NodeValueConfiguration> configNbInput(
            std::make_shared<Node::NodeValueConfiguration>(std::make_pair(size_t(0), this->dimensionFlow.getInputDimensions().size() + 1)));
        std::shared_ptr<Node::NodeValueConfiguration> configMaxInput(
            std::make_shared<Node::NodeValueConfiguration>(std::make_pair(size_t(0), maxInputSourceIdx)));
        for(size_t idx = 0; idx < this->iSet.getMaxNbOperands(); idx++) {
            this->instructionNodesTemplate->addValueTemplate(std::make_shared<Node::NodeValueTemplate>(configNbInput));
            this->instructionNodesTemplate->addValueTemplate(std::make_shared<Node::NodeValueTemplate>(configMaxInput));
        }
    }

    return std::make_unique<Node::GenotypeTemplate>(
        this->instructionNodesTemplate,
        std::make_pair(this->nbNodesMin, this->nbNodesMax)
    );
}

bool Representations::LGPRepresentation::isValid(const Evolution::Individual& indiv) const
{

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
    }
    return true;
}


Data::DataValue Representations::LGPRepresentation::executeIndividualRaw(
    const Evolution::Individual& indiv, const std::vector<Data::DataView>& inputSources) const
{
    // Get effective nodes
    std::vector<std::vector<std::reference_wrapper<const Node::GPNode>>> effectiveNodes = indiv.getGenotype().getEffectiveNodes();

    /// Registers used as internal memory. TODO AAAAAAAA not sure creating register here is the most efficient..
    Data::DataValue registers = Data::DataValue::zeros<double>(Data::DataType::array1d<double>(this->nbRegisters));
    Data::DataView registerView = registers.view();

    for(const Node::GPNode& node: effectiveNodes.at(0)) {

        size_t outputIndex = std::get<size_t>(node.getValue(0));
        size_t functionIndex = std::get<size_t>(node.getValue(1));

        const Instructions::Instruction& instruction = this->iSet.getInstruction(functionIndex);
        std::vector<Data::DataView> operands;

        size_t nbOperands = instruction.getNbOperands();
        for(size_t idxOp = 0; idxOp < nbOperands; idxOp++){
            size_t nodeIndex = idxOp * 2 + 2; // +2 is to ignore output and function index, then times too for both type and index
    
            size_t inputType = std::get<size_t>(node.getValue(nodeIndex));
            size_t inputIndex = std::get<size_t>(node.getValue(nodeIndex + 1));

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