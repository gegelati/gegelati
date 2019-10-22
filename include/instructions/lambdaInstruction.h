#ifndef LAMBDA_INSTRUCTION_H
#define LAMBDA_INSTRUCTION_H

#include <functional>

#include "supportedTypes.h"
#include "instructions/instruction.h"

namespace Instructions {

	/**
	* \brief Template instruction for simplifying the creation of an
	* Instruction from a c++ lambda function.
	*
	* Although the given template parameter T is a primitive type, the
	* SupportedType actually used by the instruction will be from the
	* PrimitiveType<T> type.
	*/
	template <class T> class LambdaInstruction : public Instruction {
		static_assert(std::is_fundamental<T>::value, "Template class LambdaInstruction<T> can only be used for primitive types.");
	protected:

		/**
		* \brief Function executed for this Instruction.
		*/
		const std::function<double(T, T)> func;

	public:
		/**
		* \brief delete the default constructor.
		*/
		LambdaInstruction() = delete;

		/**
		* \brief Constructor for the LambdaInstruction.
		*
		* \param[in] function the c++ std::function that will be executed for
		* this Instruction.
		*/
		LambdaInstruction(std::function<double(T, T)> function) : func{ function } {
			this->operandTypes.push_back(typeid(PrimitiveType<T>));
			this->operandTypes.push_back(typeid(PrimitiveType<T>));
		};


		double execute(
			const std::vector<std::reference_wrapper<const Parameter>>& params,
			const std::vector<std::reference_wrapper<const SupportedType>>& args) const override {

			if (Instruction::execute(params, args) != 1.0) {
				return 0.0;
			}

			const PrimitiveType<T>& arg1 = dynamic_cast<const PrimitiveType<T>&>(args.at(0).get());
			const PrimitiveType<T>& arg2 = dynamic_cast<const PrimitiveType<T>&>(args.at(1).get());
			double result = this->func((T)arg1, (T)arg2);
			return result;
		}

	};
};

#endif