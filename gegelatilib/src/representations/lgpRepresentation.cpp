#include "representations/lgpRepresentation.h"

std::unique_ptr<Evolution::Representation> Representations::LGPRepresentation::cloneUniquePtr() const
{
    return std::make_unique<Representations::LGPRepresentation>(*this);
}

bool Representations::LGPRepresentation::isValid(const Evolution::Individual& indiv)
{
    if(indiv.getSize() > this->nbNodesMax || indiv.getSize() < this->nbNodesMin) {
        return false;
    }

    std::vector<size_t> ranges = {this->nbRegisters, 4, 2, 8, 2, 8};
    for(const Node::GPNode& node: indiv.getEffectiveGenotype()) {
        if(node.getSize() != ranges.size()) {
            return false;
        }

        for(size_t idxNode = 0; idxNode < ranges.size(); idxNode++) {
            if(!std::holds_alternative<size_t>(node.getValue(idxNode))) {
                return false;
            }
            if(node.getValue(idxNode) >= Node::NodeType(ranges.at(idxNode))){
                return false;
            }
        }
    }

    return true;
}


std::vector<double> Representations::LGPRepresentation::executeIndividual(const Evolution::Individual& indiv)
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
        for(auto& index: {2, 4}){
    
            size_t inputType = std::get<size_t>(node.getValue(index));
            size_t inputIndex = std::get<size_t>(node.getValue(index + 1));

            const std::type_info& operandType = instruction.getOperandTypes().at(0).get();
            const Data::DataHandler& dataSource = (inputType==0) ? this->registers : this->registers;// TODO

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