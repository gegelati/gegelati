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
};


template <class T> InstructionAdd<T>::InstructionAdd(){
	this->operandTypes.push_back(typeid(PrimitiveType<T>));
	this->operandTypes.push_back(typeid(PrimitiveType<T>));
}

#endif