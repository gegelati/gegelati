#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <vector> 

/**
*/
class Instruction {
protected:
	/**
	* @brief Protected constructor to force the class abstract nature.
	*/
	Instruction();
	
	/**
	* @brief Number of parameters required when calling the instruction.
	*/
	int nbParameters;
	
	std::vector<std::type_info> operandTypes;
};

#endif