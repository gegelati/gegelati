#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <typeinfo>
#include <vector> 

#include "parameter.h"
#include "supportedTypes.h"

namespace Instructions {
	/**
	* \brief This abstract class is the base class for any instruction to be used in a Program.
	*
	* An Instruction declares a list of operands it needs to be executed, as well
	* as a set of parameters. This information will be used to fetch the required
	* operands and parameters from any ProgramLine, and to ensure the
	* compatibility of the type of the fetched operands with the instruction
	* before executing it.
	*/
	class Instruction {

	public:
		/**
		* \brief Get the list of operand types needed by the Instruction.
		*
		* \return a const reference on the list of operand type_info of the Instruction.
		*/
		const std::vector<std::reference_wrapper<const std::type_info>>& getOperandTypes() const;

		/**
		* \brief Get the number of operands required to execute the Instruction.
		*
		* \return an unsigned int value corresponding to the number of operands required by the Instruction.
		*/
		unsigned int getNbOperands() const;


		/**
		* \brief Get the number of parameters required to execute the Instruction.
		*
		* \return an unsigned int value corresponding to the number of parameters required by the Instruction.
		*/
		unsigned int getNbParameters() const;

		/**
		* \brief Check if a given vector contains elements whose types corresponds to the types of the Instruction operands.
		*
		* \param[in] arguments a const list of reference_wrapper to any type of object. (not doable at compile time)
		*/
		bool checkOperandTypes(const std::vector<std::reference_wrapper<SupportedType>>& arguments) const;

		/**
		* \brief Check if a given vector contains the right number of parameters for the Instruction.
		*
		* \param[in] params a const list of reference_wrapper to Parameters.
		*/
		bool checkParameters(const std::vector<std::reference_wrapper<Parameter>>& params) const;

		/**
		* \brief Execute the Instruction for the given parameters and arguments.
		*
		* Derived class should implement their own behavior for this method. In cas
		* of invalid argument or parameters, for type or number or value reason, this
		* method should always return 0.0.
		*
		* \param[in] params the vector of reference_wrapper to the Parameter passed to the Instruction.
		* \param[in] args the vector of reference_wrapper to the SupportedType passed to the Instruction.
		* \return the default implementation of the Intruction class returns 0.0 if the given params or arguments are not valid.
		*         Otherwise, 1.0 is returned.
		*/
		virtual double execute(
			const std::vector<std::reference_wrapper<Parameter>>& params,
			const std::vector<std::reference_wrapper<SupportedType>>& args) const;

	protected:
		/**
		* \brief Protected constructor to force the class abstract nature.
		*
		* The definition of this constructor initialize an empty operandType list and
		* sets the number of required parameters to 0.
		*/
		Instruction();

		/**
		* \brief Number of parameters required when calling the instruction.
		*/
		unsigned int nbParameters;

		/**
		* \brief List of the types of the operands needed to execute the instruction.
		*/
		std::vector<std::reference_wrapper<const std::type_info>> operandTypes;
	};

}
#endif