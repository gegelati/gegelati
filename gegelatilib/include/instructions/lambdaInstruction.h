#ifndef LAMBDA_INSTRUCTION_H
#define LAMBDA_INSTRUCTION_H

#include <functional>

#include "data/untypedSharedPtr.h"
#include "data/primitiveType.h"
#include "instructions/instruction.h"

namespace Instructions {

	/**
	* \brief Template instruction for simplifying the creation of an
	* Instruction from a c++ lambda function.
	*
	* Template parameter T can either be a primitive type (i.e. int, char,
	* float, ...), a type inheriting from SupportedType, or an array whose type
	* inherits from SupportedType.
	*
	* When the given template parameter T is a primitive type, the
	* SupportedType actually used by the instruction will be from the
	* PrimitiveType<T> type. Nevertheless, from the lambda function point
	* of view, this will be transparent and the primitive type will be used.
	*
	* When the given template type is a SupportedType child class, or an array
	* the lambda expression will used these types directly.
	*
	* Currently, LambdaInstructions all require two operands, with the same
	* type determined by T.
	*
	* The second template parameter is using the SFINAE to specialize the class
	* implementation differently, depending on the template parameter.
	*/
	template <class T, typename Enable = void> class LambdaInstruction : public Instruction {};

	/**
	* \brief Specialization of the LambdaInstruction template class for
	* primitive types.
	*/
	template <class T>
	class LambdaInstruction <T, typename std::enable_if<std::is_fundamental<T>::value>::type> : public Instruction
	{
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
			this->operandTypes.push_back(typeid(Data::PrimitiveType<T>));
			this->operandTypes.push_back(typeid(Data::PrimitiveType<T>));
		};


		double execute(
			const std::vector<std::reference_wrapper<const Parameter>>& params,
			const std::vector<Data::UntypedSharedPtr>& args) const override {

			if (Instruction::execute(params, args) != 1.0) {
				return 0.0;
			}

			const Data::PrimitiveType<T>& arg1 = *(args.at(0).getSharedPointer<const Data::PrimitiveType<T>>());
			const Data::PrimitiveType<T>& arg2 = *(args.at(1).getSharedPointer<const Data::PrimitiveType<T>>());
			double result = this->func((T)arg1, (T)arg2);
			return result;
		}
	};

	/**
	* \brief Specialization of the LambdaInstruction template class for
	* types derived from SupportedType.
	*/
	//template <class T>
	//class LambdaInstruction < T, typename std::enable_if <std::is_base_of<Data::SupportedType, T>::value>::type> : public Instruction
	//{
	//protected:
	//	/**
	//	* \brief Function executed for this Instruction.
	//	*/
	//	const std::function<double(T, T)> func;
	//
	//public:
	//	/**
	//	* \brief delete the default constructor.
	//	*/
	//	LambdaInstruction() = delete;
	//
	//	/**
	//	* \brief Constructor for the LambdaInstruction.
	//	*
	//	* \param[in] function the c++ std::function that will be executed for
	//	* this Instruction.
	//	*/
	//	LambdaInstruction(std::function<double(T, T)> function) : func{ function } {
	//		this->operandTypes.push_back(typeid(T));
	//		this->operandTypes.push_back(typeid(T));
	//	};
	//
	//	double execute(
	//		const std::vector<std::reference_wrapper<const Parameter>>& params,
	//		const std::vector<std::shared_ptr<const Data::SupportedType>>& args) const override {
	//
	//		// Check if operands and types have the right type.
	//		if (Instruction::execute(params, args) != 1.0) {
	//			return 0.0;
	//		}
	//
	//		const T& arg1 = *std::dynamic_pointer_cast<const T>(args.at(0));
	//		const T& arg2 = *std::dynamic_pointer_cast<const T>(args.at(1));
	//		double result = this->func(arg1, arg2);
	//		return result;
	//	}
	//};

	/**
	* \brief Specialization of the LambdaInstruction template class for
	* types derived from SupportedType and for array of type derived from
	* SupportedType.
	*/
	//template <class T>
	//class LambdaInstruction < T, typename std::enable_if <std::is_array<T>::value && std::is_base_of <Data::SupportedType, typename std::remove_all_extents<T>::type>::value>::type> : public Instruction
	//{
	//protected:
	//	/**
	//	* \brief Function executed for this Instruction.
	//	*/
	//	const std::function<double(const T, const T)> func;
	//
	//public:
	//	/**
	//	* \brief delete the default constructor.
	//	*/
	//	LambdaInstruction() = delete;
	//
	//	/**
	//	* \brief Constructor for the LambdaInstruction.
	//	*
	//	* \param[in] function the c++ std::function that will be executed for
	//	* this Instruction.
	//	*/
	//	LambdaInstruction(std::function<double(const T, const T)> function) : func{ function } {
	//		this->operandTypes.push_back(typeid(T));
	//		this->operandTypes.push_back(typeid(T));
	//	};
	//
	//	double execute(
	//		const std::vector<std::reference_wrapper<const Parameter>>& params,
	//		const std::vector<std::shared_ptr<const Data::SupportedType>>& args) const override {
	//
	//		// Check if operands and types have the right type.
	//		if (Instruction::execute(params, args) != 1.0) {
	//			return 0.0;
	//		}
	//
	//		std::shared_ptr<const T> arg1 = std::dynamic_pointer_cast<const T>(args.at(0));
	//		std::shared_ptr<const T> arg2 = std::dynamic_pointer_cast<const T>(args.at(1));
	//		double result = this->func(arg1.get(), arg2.get());
	//		return result;
	//	}
	//};
};

#endif