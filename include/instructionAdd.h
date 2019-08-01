#ifndef INSTRUCTION_ADD_H
#define INSTRUCTION_ADD_H

#include <typeinfo>
#include <type_traits>

#include "instruction.h"

/**
* \brief Template class for add instruction on all types of data: double, int, ...
*/
template <class T> class InstructionAdd : public Instruction {

	static_assert(std::is_fundamental<T>::value, "Template class InstructionAdd<T> can only be used for primitive types.");

public:
	/**
	*  \brief Constructor for the InstructionAdd class.
	*/
	InstructionAdd();

	double execute(
		const std::vector<std::reference_wrapper<Parameter>>& params,
		const std::vector<std::reference_wrapper<SupportedType>>& args) const;
};


template <class T> InstructionAdd<T>::InstructionAdd(){
	this->operandTypes.push_back(typeid(PrimitiveType<T>));
	this->operandTypes.push_back(typeid(PrimitiveType<T>));
}


template <class T> double InstructionAdd<T>::execute(
	const std::vector<std::reference_wrapper<Parameter>>& params,
	const std::vector<std::reference_wrapper<SupportedType>>& args) const {

	if (Instruction::execute(params, args) != 1.0) {
		return 0.0;
	} 

	return dynamic_cast<PrimitiveType<T>&>(args.at(0).get()) + dynamic_cast<PrimitiveType<T>&>(args.at(1).get());;
}

#endif