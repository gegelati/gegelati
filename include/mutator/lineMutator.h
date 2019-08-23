#ifndef LINE_MUTATOR_H
#define LINE_MUTATOR_H

#include "program/line.h"

namespace Mutator {
	/**
	* Namespace containing all functions to apply randomness to a Program::Line
	*/
	namespace LineMutator {
		/**
		* \brief Randomly initializes a Program::Line.
		*
		* Using the Mutator::RNG functions, this function initializes all
		* attributes of the given Program::Line to randomly selected values.
		* The attributes are initialized in such a way that the created line
		* is executable within its Environment. This means that the
		* instructionIndex, the destinationIndex, all operands pairs (sources
		* and localisation) will be initialized randomly such
		* that the corresponding instruction can be executed with valid data.
		* Parameters and operands of the line that are unused by the selected
		* Instruction will be initialized
		*
		*
		* \param[in,out] line the Program::Line whose attributes are being
		* initialized.
		* \throw std::runtime_error if the provided Environment has no
		* dataSource that can successfully provide data for any of its
		* Instruction.
		*/
		void initRandomCorrectLine(Program::Line& line);

		/**
		* \brief This function randomly modifies the behavior of a
		* Program::Line.
		*
		* Probability of altering the different attributes of the Line is 
		* computed as if the given Program::Line was encoded with a word of 
		* Program::Program::computeLineSize() bits, and a single bit must be 
		* flipped. This means that if the instructions represents n bits over 
		* m, then there is a probability of n/m that the instruction of the
		* line is altered.
		* To avoid non-executable line, the alteration of instruction, 
		* destination, or operands dataSource index are based on a uniform 
		* probability within acceptable values.
		* If the selected Instruction is no longer compatible with types 
		* provided by the line operands, or vice versa, then the function 
		* will set things right by selecting new valid operands randomly.
		* Although this is biased toward operands volatility, this is the 
		* a controlled complexity, and yet random, way to ensure that a valid 
		* line is obtained. The alternative (from TPG reference PhD) would be 
		* to repeatedly alter the line until a valid one is obtained.
		*/
		void alterCorrectLine(Program::Line& line);
	}
};
#endif