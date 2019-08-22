#ifndef INSTRUCTION_MULT_BY_CONST_PARAM
#define INSTRUCTION_MULT_BY_CONST_PARAM

#include "instruction.h"

namespace Instructions {

	/**
	* \brief Instruction for multiplying a unique argument of type T by a constant parameter of type U.
	*/
	template <class T, class U> class MultByConstParam : public Instruction {
		static_assert(std::is_fundamental<T>::value&& std::is_fundamental<U>::value, "Template class MultByConstParam<T,U> can only be used for primitive types.");
		static_assert(std::is_same<int16_t, U>() || std::is_same<float, U>(), "Param type can either be int16_t or float.");
	public:
		/**
		*  \brief Constructor for the MultByConstParam class.
		*/
		MultByConstParam();

		double execute(
			const std::vector<std::reference_wrapper<const Parameter>>& params,
			const std::vector<std::reference_wrapper<const SupportedType>>& args) const;
	};

	template <class T, class U> MultByConstParam<T, U>::MultByConstParam() {
		this->operandTypes.push_back(typeid(PrimitiveType<T>));
		this->nbParameters = 1;
	}

	template<class T, class U> double MultByConstParam<T, U>::execute(
		const std::vector<std::reference_wrapper<const Parameter>>& params,
		const std::vector<std::reference_wrapper<const SupportedType>>& args) const
	{
		if (Instruction::execute(params, args) != 1.0) {
			return 0.0;
		}

		const U pValue = (const U&)params.at(0).get();
		return dynamic_cast<const PrimitiveType<T>&>(args.at(0).get()) *(double)pValue ;
	}
	;
}

#endif