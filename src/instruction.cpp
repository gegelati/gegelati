#include "instruction.h"

Instruction::Instruction() : operandTypes(), nbParameters(0) {
}

std::vector<std::reference_wrapper<const std::type_info>>::const_iterator Instruction::getOperandTypes() const {
	return this->operandTypes.cbegin();
}

int Instruction::getNbParameters() const {
	return this->nbParameters;
}