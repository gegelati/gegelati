#include "representations/lgpRepresentation.h"

std::unique_ptr<Evolution::Representation> Representations::LGPRepresentation::cloneUniquePtr() const
{
    return std::make_unique<Representations::LGPRepresentation>(
        this->iSet,
        this->nbRegisters,
        this->nbNodesMin,
        this->nbNodesMax,
        this->representationName,
        this->representationColor
    );
}

void Representations::LGPRepresentation::setInputDimensions(const std::vector<std::reference_wrapper<const Data::DataHandler>>& inputSources)
{
    Evolution::Representation::setInputDimensions(inputSources);
    this->nbInputSources++;
    if(this->nbRegisters > this->maxInputSourceIdx) {
        this->maxInputSourceIdx = this->nbRegisters;
    }
}


std::unique_ptr<const Node::GenotypeTemplate> Representations::LGPRepresentation::getGenotypeTemplate() const
{
    // Instruction node template is fixed during evolution, so created only once.
    if(this->instructionNodesTemplate->size() == 0) {

        // Value Template for register
        std::shared_ptr<Node::NodeValueConfiguration> configRegister(std::make_shared<Node::NodeValueConfiguration>(std::make_pair(size_t(0), this->nbRegisters)));
        this->instructionNodesTemplate->addValueTemplate(std::make_shared<Node::NodeValueTemplate>(configRegister));
        
        // Value template for instruction
        std::shared_ptr<Node::NodeValueConfiguration> configFunction(std::make_shared<Node::NodeValueConfiguration>(std::make_pair(size_t(0), size_t(this->iSet.getNbInstructions()))));
        this->instructionNodesTemplate->addValueTemplate(std::make_shared<Node::NodeValueTemplate>(configFunction));
    
        // Value templates for input type and index
        std::shared_ptr<Node::NodeValueConfiguration> configNbInput(std::make_shared<Node::NodeValueConfiguration>(std::make_pair(size_t(0), this->nbInputSources)));
        std::shared_ptr<Node::NodeValueConfiguration> configMaxInput(std::make_shared<Node::NodeValueConfiguration>(std::make_pair(size_t(0), this->maxInputSourceIdx)));
        for(size_t idx = 0; idx < this->iSet.getMaxNbOperands(); idx++) {
            this->instructionNodesTemplate->addValueTemplate(std::make_shared<Node::NodeValueTemplate>(configNbInput));
            this->instructionNodesTemplate->addValueTemplate(std::make_shared<Node::NodeValueTemplate>(configMaxInput));
        }
    }

      auto gt(std::make_unique<Node::GenotypeTemplate>(
        this->instructionNodesTemplate,
        std::make_pair(this->nbNodesMin, this->nbNodesMax)));

    return std::move(gt);
}

bool Representations::LGPRepresentation::isValid(const Evolution::Individual& indiv) const
{
    if(this->nbInputSources == 0 || this->maxInputSourceIdx == 0) {
        throw std::runtime_error("Representations::LGPRepresentation::isValid: cannot define if an individual is valid without input dimensions set.");
    }

    // Return false if genotype length is out of bounds.
    if(indiv.getSize() > this->nbNodesMax || indiv.getSize() < this->nbNodesMin) {
        return false;
    }

    // Ranges should look like {nbRegister, NbInstr, NbTypeInput, MaxInput, NbTypeInput, MaxInput...}.
    std::vector<size_t> ranges = {this->nbRegisters, this->iSet.getNbInstructions()};
    for(size_t idx = 0; idx < this->iSet.getMaxNbOperands(); idx++) {
        ranges.push_back(this->nbInputSources);
        ranges.push_back(this->maxInputSourceIdx);
    }

    // Verify each (effective) node corresponds to the required specifications.
    for(const Node::GPNode& node: indiv.getEffectiveGenotype()) {
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


std::vector<double> Representations::LGPRepresentation::executeIndividual(
    const Evolution::Individual& indiv, const std::vector<std::reference_wrapper<const Data::DataHandler>>& inputSources) const
{
    // Define Inputs
    // Define function/instruction or index of it
    std::vector<std::reference_wrapper<const Node::GPNode>> genotype = indiv.getEffectiveGenotype();

    /// Registers used as internal memory. TODO AAAAAAAA not sure creating register here is the most efficient..
    Data::PrimitiveTypeArray<double> registers(this->nbRegisters);

    for(const Node::GPNode& node: genotype) {

        size_t outputIndex = std::get<size_t>(node.getValue(0));
        size_t functionIndex = std::get<size_t>(node.getValue(1));

        const Instructions::Instruction& instruction = this->iSet.getInstruction(functionIndex);
        std::vector<Data::UntypedSharedPtr> operands;

        size_t nbOperands = instruction.getNbOperands();
        for(size_t idxOp = 0; idxOp < nbOperands; idxOp++){
            size_t nodeIndex = idxOp * 2 + 2; // +2 is to ignore output and function index, then times too for both type and index
    
            size_t inputType = std::get<size_t>(node.getValue(nodeIndex));
            size_t inputIndex = std::get<size_t>(node.getValue(nodeIndex + 1));

            const std::type_info& operandType = instruction.getOperandTypes().at(0).get();
            const Data::DataHandler& dataSource = (inputType==0) ? registers : inputSources[inputType - 1];

            uint64_t operandLocation = dataSource.scaleLocation(inputIndex, operandType);
            Data::UntypedSharedPtr data = dataSource.getDataAt(operandType, operandLocation);

            operands.push_back(data);
        }

        double result = instruction.execute(operands);
        registers.setDataAt(typeid(double), outputIndex, result);
    }

    // Return value of first register
    return {*(registers.getDataAt(typeid(double), 0).getSharedPointer<const double>())};
}