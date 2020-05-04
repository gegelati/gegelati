#ifndef LAMBDA_INSTRUCTION_H
#define LAMBDA_INSTRUCTION_H

#include <functional>
#include <typeinfo>

#include "data/untypedSharedPtr.h"
#include "instructions/instruction.h"

namespace Instructions {

	/**
	* \brief Template instruction for simplifying the creation of an
	* Instruction from a c++ lambda function.
	*
	* Template parameters First and Rest can be any primitive type, class or
	* const c-style 1D array.
	*
	* Each template parameter corresponds to an argument of the function given
	* to the LambdaInstruction constructor, specifying its type.
	*/
	template< typename First, typename... Rest>
	class LambdaInstruction : public Instructions::Instruction {
	protected:

		/**
		* \brief Function executed for this Instruction.
		*/
		const std::function<double(const First, const Rest...)> func;

	public:
		/**
		* \brief delete the default constructor.
		*/
		LambdaInstruction() = delete;

		/**
		* \brief Constructor for the LambdaInstruction.
		*
		* \param[in] function the c++ std::function that will be executed for
		* this Instruction. The function must have the same types in its argument
		* list as specified by the template parameters. (checked at compile time)
		*/
		LambdaInstruction(std::function<double(First, Rest...)> function) : func{ function } {

			this->operandTypes.push_back(typeid(First));
			// Fold expression to push all other types
			(this->operandTypes.push_back(typeid(Rest)), ...);
		};

		/// Inherited from Instruction
		virtual bool checkOperandTypes(const std::vector<Data::UntypedSharedPtr>& arguments) const override {
			if (arguments.size() != this->operandTypes.size()) {
				return false;
			}

			// List of expected types
			const std::vector<std::reference_wrapper<const std::type_info>> expectedTypes{
				// First
				(!std::is_array<First>::value) ?
					typeid(First) :
					typeid(std::remove_all_extents_t<First>[]),
				(!std::is_array<Rest>::value) ?
					typeid(Rest) :
					typeid(std::remove_all_extents_t<Rest>[])... };

			for (auto idx = 0; idx < arguments.size(); idx++) {
				// Argument Type
				const std::type_info& argType = arguments.at(idx).getType();
				if (argType != expectedTypes.at(idx).get()) {
					return false;
				}
			}

			return true;
		};

		double execute(
			const std::vector<std::reference_wrapper<const Parameter>>& params,
			const std::vector<Data::UntypedSharedPtr>& args) const override {

			if (Instruction::execute(params, args) != 1.0) {
				return 0.0;
			}

			// const evaluated lambda expression are needed because type of arg will
			// not be the same if First is an array, and if it is not. 
			// Fort this reason, ternary operator can not be used.
			const auto& arg1 = [&]() {
				if constexpr (!std::is_array<First>::value) {
					return *(args.at(0).getSharedPointer<const First>());
				}
				else {
					return (args.at(0).getSharedPointer<const std::remove_all_extents_t<First>[]>()).get();
				};
			}();

			size_t i = args.size() - 1;
			// Using i-- as expansion seems to happen with parameters evaluated from right to left.
			double result = this->func(arg1,
				[&]() {
					if constexpr (!std::is_array<Rest>::value) {
						return *(args.at(i--).getSharedPointer<const Rest>());
					}
					else {
						return (args.at(i--).getSharedPointer<const std::remove_all_extents_t<Rest>[]>()).get();
					};
				}()...);
			return result;
		};
	};
};

#endif
