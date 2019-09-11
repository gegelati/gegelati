#ifndef PROGRAM_MUTATOR_H
#define PROGRAM_MUTATOR_H

#include "program/program.h"
#include "program/line.h"
#include "mutator/mutationParameters.h"
#include "mutator/lineMutator.h"

namespace Mutator {
	/**
	* Namespace containing all functions to apply randomness to a Program::Program
	*/
	namespace ProgramMutator {

		/**
		* \brief (Re)Initialize a program randomly.
		*
		* Initialise a Program with a random number or Line, below the given
		* limit. If the Program already contains lines, they will be entirely
		* deleted by this function.
		*
		* \param[in,out] p the Program to initialize.
		* \param[in] params MutationParameters for the mutation.
		*/
		void initRandomProgram(Program::Program& p, const MutationParameters& params);

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
		bool deleteRandomLine(Program::Program& p);

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

		/**
		* \brief Swap two randomly selected instructions within the given
		* Program.
		*
		* This function selects two lines of the program randomly and swaps
		* them. If the given Program has less than two lines, nothing happens.
		* Random selection is based on the Mutator::RNG.
		*
		* \param[in,out] p the Program whose lines will be swapped.
		* \return true if the lines where successfully swapped, false if the
		*         Program has less than two lines.
		*/
		bool swapRandomLines(Program::Program& p);

		/**
		* \brief Alter a randomly selected Line in a given Program.
		*
		* If the given Program has more than 0 Line, this function selects a
		* Line (pseudo)-randomly in a given Program and calls the
		* Mutator::LineMutator:AlterCorrectLine function on it.
		* Random selection is based on the Mutator::RNG.
		*
		* \param[in,out] p the Program whose line will be altered.
		* \return true if a line was successfully altered, false if the
		*         Program has less than one line.
		*/
		bool alterRandomLine(Program::Program& p);


		/**
		* \brief Mutate the behavior of the Program with given probabilities.
		*
		* Following the probabilities given as arguments to the function, the
		* Program may be modified by deleting a random line, adding a new line
		* at a random position, modifying the line behavior, or swapping
		* instructions within the Program.
		*
		* Because of the probabilistic nature of this function, it may happen
		* that no alteration is peformed.
		*
		* \param[in,out] p the Program whose line will be altered.
		* \param[in] params MutationParameters for the mutation.
		* \return true if a modification was performed, false otherwise.
		*/
		bool mutateProgram(Program::Program& p, const MutationParameters& params);


	};
};

#endif 
