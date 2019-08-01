#ifndef INSTRUCTION_MULT_BY_CONST_PARAM
#define INSTRUCTION_MULT_BY_CONST_PARAM

#include "instruction.h"
/**
* \brief Instruction for multiplying a unique argument of type T by a constant parameter of type U.
*/
template <class T, class U> class InstructionMultByConstParam: public Instruction {
	static_assert(std::is_fundamental<T>::value && std::is_fundamental<U>::value, "Template class InstructionMultByConstParam<T,U> can only be used for primitive types.");
public:
	/**
	*  \brief Constructor for the InstructionMultByConstParam class.
	*/
	InstructionMultByConstParam();

	double execute(
		const std::vector<std::reference_wrapper<Parameter>>& params,
		const std::vector<std::reference_wrapper<SupportedType>>& args) const;
};

template <class T, class U> InstructionMultByConstParam<T, U>::InstructionMultByConstParam() {
	this->operandTypes.push_back(typeid(PrimitiveType<T>));
	this->nbParameters = 1;
}

template<class T, class U> double InstructionMultByConstParam<T, U>::execute(
	const std::vector<std::reference_wrapper<Parameter>>& params, 
	const std::vector<std::reference_wrapper<SupportedType>>& args) const
{
	if (Instruction::execute(params, args) != 1.0) {
		return 0.0;
	}

	return dynamic_cast<PrimitiveType<T>&>(args.at(0).get()) * (U)params.at(0).get() ;
}
;

#endif