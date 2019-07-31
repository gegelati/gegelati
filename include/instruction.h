#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <typeinfo>
#include <vector> 

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
	* \brief Get a const_iterator on the operand types nedded by the Instruction. 
	* 
	*`\return a const_iterator on the list of operand type_info of the Instruction.
	*/
	std::vector<std::reference_wrapper<const std::type_info>>::const_iterator getOperandTypes() const;

	/**
	* \brief Get the number of parameters required to execute the Instruction.
	*
	* \return an Int value corresponding to the number of parameters required by the Instruction.
	*/
	int getNbParameters() const;
	
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