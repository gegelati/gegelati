#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <typeinfo>
#include <vector> 

#include "parameter.h"
#include "supportedTypes.h"


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
	*`\return a const reference on the list of operand type_info of the Instruction.
	*/
	const std::vector<std::reference_wrapper<const std::type_info>>& getOperandTypes() const;

	/**
	* \brief Get the number of parameters required to execute the Instruction.
	*
	* \return an Int value corresponding to the number of parameters required by the Instruction.
	*/
	int getNbParameters() const;

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
	int nbParameters;
	
	/**
	* \brief List of the types of the operands needed to execute the instruction.
	*/
	std::vector<std::reference_wrapper<const std::type_info>> operandTypes;
};

#endif