#ifndef LINE_MUTATOR_H
#define LINE_MUTATOR_H

#include "program/line.h"

namespace Mutator {
	/**
	* Namespace containing all functions to apply randomness to a Program::Line
	*/
	namespace Line {
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
	}
};
#endif