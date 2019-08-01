#include "instructions/instruction.h"

#include <iostream>

using namespace Instructions;

Instruction::Instruction() : operandTypes(), nbParameters(0) {
}

const std::vector<std::reference_wrapper<const std::type_info>>& Instruction::getOperandTypes() const {
	return this->operandTypes;
}

int Instruction::getNbParameters() const {
	return this->nbParameters;
}

bool Instruction::checkOperandTypes(const std::vector<std::reference_wrapper<SupportedType>> & arguments) const
{
	if(arguments.size() != this->operandTypes.size()){
		return false;
	}

	for (int i = 0; i < arguments.size(); i++) {
		  if (typeid(arguments.at(i).get()) != this->operandTypes.at(i).get()) {
			  return false;
		  }
	}
	return true;
}

bool Instruction::checkParameters(const std::vector<std::reference_wrapper<Parameter>>& params) const
{
	return (params.size() == this->nbParameters);
}

double Instruction::execute(const std::vector<std::reference_wrapper<Parameter>>& params, const std::vector<std::reference_wrapper<SupportedType>>& arguments) const
{
	if (!this->checkParameters(params) || !this->checkOperandTypes(arguments)) {
		return 0.0;
	}
	else {
		return 1.0;
	}
}
