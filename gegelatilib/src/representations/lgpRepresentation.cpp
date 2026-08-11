#include "representations/lgpRepresentation.h"

std::unique_ptr<Evolution::Representation> Representations::LGPRepresentation::cloneUniquePtr() const
{
    return std::make_unique<Representations::LGPRepresentation>(*this);
}

void Representations::LGPRepresentation::setInputDimensions(const std::vector<std::reference_wrapper<const Data::DataHandler>>& inputSources)
{
    Evolution::Representation::setInputDimensions(inputSources);
    this->nbInputSources++;
    if(this->nbRegisters > this->maxInputSourceIdx) {
        this->maxInputSourceIdx = this->nbRegisters;
    }
}


std::vector<Node::NodeValueTemplate> Representations::LGPRepresentation::getGenotypeTemplate() const
{
    
    Node::NodeValueTemplate registerValueTemplate = std::make_pair(size_t(0), this->nbRegisters);
    Node::NodeValueTemplate functionValueTemplate = std::make_pair(size_t(0), size_t(this->iSet.getNbInstructions()));
    Node::NodeValueTemplate nbInputSourcesValueTemplate = std::make_pair(size_t(0), this->nbInputSources);
    Node::NodeValueTemplate maxInputSourceIdxValueTemplate = std::make_pair(size_t(0), this->maxInputSourceIdx);

    std::vector<Node::NodeValueTemplate> genotypeTemplate{registerValueTemplate, functionValueTemplate};
    for(size_t idx = 0; idx < this->iSet.getMaxNbOperands(); idx++) {
        genotypeTemplate.push_back(nbInputSourcesValueTemplate);
        genotypeTemplate.push_back(maxInputSourceIdxValueTemplate);
    }
    return genotypeTemplate;
}

bool Representations::LGPRepresentation::isValid(const Evolution::Individual& indiv)
{
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
    const Evolution::Individual& indiv, const std::vector<std::reference_wrapper<const Data::DataHandler>>& inputSources)
{
    // Define Inputs
    // Define function/instruction or index of it
    std::vector<std::reference_wrapper<const Node::GPNode>> genotype = indiv.getEffectiveGenotype();

    registers.resetData();

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
            const Data::DataHandler& dataSource = (inputType==0) ? this->registers : inputSources[inputType - 1];

            uint64_t operandLocation = dataSource.scaleLocation(inputIndex, operandType);
            Data::UntypedSharedPtr data = dataSource.getDataAt(operandType, operandLocation);

            operands.push_back(data);
        }

        double result = instruction.execute(operands);
        this->registers.setDataAt(typeid(double), outputIndex, result);
    }

    // Return value of first register
    return {*(this->registers.getDataAt(typeid(double), 0).getSharedPointer<const double>())};
}