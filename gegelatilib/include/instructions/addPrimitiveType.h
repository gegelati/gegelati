#ifndef INSTRUCTION_ADD_H
#define INSTRUCTION_ADD_H

#include <typeinfo>
#include <type_traits>
#include <memory>

#include "data/untypedSharedPtr.h"
#include "instruction.h"

namespace Instructions {

	/**
	* \brief Template class for add instruction on all types of data: double, int, ...
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
			const std::vector<Data::UntypedSharedPtr>& args) const override;
	};


	template <class T> AddPrimitiveType<T>::AddPrimitiveType() {
		this->operandTypes.push_back(typeid(T));
		this->operandTypes.push_back(typeid(T));
	}


	template <class T> double AddPrimitiveType<T>::execute(
		const std::vector<std::reference_wrapper<const Parameter>>& params,
		const std::vector<Data::UntypedSharedPtr>& args) const {

		if (Instruction::execute(params, args) != 1.0) {
			return 0.0;
		}

		return *(args.at(0).getSharedPointer<const T>()) + (double)*(args.at(1).getSharedPointer<const T>());
	}
}

#endif