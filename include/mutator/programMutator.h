#ifndef PROGRAM_MUTATOR_H
#define PROGRAM_MUTATOR_H

#include "program/program.h"
#include "program/line.h"
#include "mutator/lineMutator.h"

namespace Mutator {
	/**
	* Namespace containing all functions to apply randomness to a Program::Program
	*/
	namespace ProgramMutator {
		/**
		* \brief Deletes a randomly selected Line of the given Program.
		*
		* Unless a single Line (or less) remains in the given Progeam, this
		* function randomly selects a line of the Program and deletes it.
		* Random selection is based on the Mutator::RNG.
		*
		* \param[in,out] p the Program whose lines will be altered.
		* \return true if a line could be added, false otherwise.
		*
		*/
		bool deleteRandomLine(Program::Program & p);

		/**
		* \brief Insert a new Line at a randomly selected position within the given Program.
		*
		* This function randomly selects a position in the Program.lines and
		* insert a randomly initialized line (using Mutator::Line::
		* initRandomCorrectLine).
		* Random selection is based on the Mutator::RNG.
		*
		* \param[in,out] p the Program whose lines will be altered.
		*
		*/
		void insertRandomLine(Program::Program& p);
	};
};

#endif 
