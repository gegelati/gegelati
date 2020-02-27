#ifndef INSTRUCTION_ADD_H
#define INSTRUCTION_ADD_H

#include <typeinfo>
#include <type_traits>
#include <memory>

#include "data/supportedType.h"
#include "data/primitiveType.h"
#include "instruction.h"

namespace Instructions {

	/**
	* \brief Template class for add instruction on all types of data: double, int, ...
	*
	* Although the given template parameter T is a primitive type, the
	* SupportedType actually used by the instruction will be from the PrimitiveType<T> type.
	*/
	template <class T> class AddPrimitiveType : public Instruction {

		static_assert(std::is_fundamental<T>::value, "Template class AddPrimitiveType<T> can only be used for primitive types.");

	public:
		/**
		*  \brief Constructor for the AddPrimitiveType class.
		*/
		AddPrimitiveType();

		virtual double execute(
			const std::vector<std::reference_wrapper<const Parameter>>& params,
			const std::vector<std::shared_ptr<const Data::SupportedType>>& args) const override;
	};


	template <class T> AddPrimitiveType<T>::AddPrimitiveType() {
		this->operandTypes.push_back(typeid(Data::PrimitiveType<T>));
		this->operandTypes.push_back(typeid(Data::PrimitiveType<T>));
	}


	template <class T> double AddPrimitiveType<T>::execute(
		const std::vector<std::reference_wrapper<const Parameter>>& params,
		const std::vector<std::shared_ptr<const Data::SupportedType>>& args) const {

		if (Instruction::execute(params, args) != 1.0) {
			return 0.0;
		}

		return *std::dynamic_pointer_cast<const Data::PrimitiveType<T>>(args.at(0)) + (double)*std::dynamic_pointer_cast<const Data::PrimitiveType<T>>(args.at(1));;
	}
}

#endif