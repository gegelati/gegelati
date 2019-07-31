#ifndef INSTRUCTION_H
#define INSTRUCTION_H

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
protected:
	/**
	* \brief Protected constructor to force the class abstract nature.
	*/
	Instruction();
	
	/**
	* \brief Number of parameters required when calling the instruction.
	*/
	int nbParameters;
	
	/**
	* \brief List of the types of the operands needed to execute the instruction.
	*/
	std::vector<std::type_info> operandTypes;
};

#endif