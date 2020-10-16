/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

#ifndef PROGRAM_MUTATOR_H
#define PROGRAM_MUTATOR_H

#include "mutator/lineMutator.h"
#include "mutator/mutationParameters.h"
#include "program/line.h"
#include "program/program.h"

namespace Mutator {
    /**
     * Namespace containing all functions to apply randomness to a
     * Program::Program
     */
    namespace ProgramMutator {

        /**
         * \brief (Re)Initialize a program randomly.
         *
         * Initialise a Program with a random number or Line, below the given
         * limit. If the Program already contains lines, they will be entirely
         * deleted by this function.
         *
         * After altering the Program, its intron Line are identified with a
         * call to Program::identifyIntrons().
         *
         * \param[in,out] p the Program to initialize.
         * \param[in] params MutationParameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        void initRandomProgram(Program::Program& p,
                               const MutationParameters& params,
                               Mutator::RNG& rng);

        /**
         * \brief Deletes a randomly selected Line of the given Program.
         *
         * Unless a single Line (or less) remains in the given Progeam, this
         * function randomly selects a line of the Program and deletes it.
         * Random selection is based on the given Mutator::RNG.
         *
         * This method does NOT update automatically the intron property of the
         * Lines of the Program. Hence, the resulting Program may not execute
         * correctly until Program::identifyIntrons() method is called.
         *
         * \param[in,out] p the Program whose lines will be altered.
         * \param[in] rng Random Number Generator used in the mutation process.
         * \return true if a line could be added, false otherwise.
         *
         */
        bool deleteRandomLine(Program::Program& p, Mutator::RNG& rng);

        /**
         * \brief Insert a new Line at a randomly selected position within the
         * given Program.
         *
         * This function randomly selects a position in the Program.lines and
         * insert a randomly initialized line (using Mutator::Line::
         * initRandomCorrectLine).
         * Random selection is based on the given Mutator::RNG.
         *
         * This method does NOT update automatically the intron property of the
         * Lines of the Program. Hence, the resulting Program may not execute
         * correctly until Program::identifyIntrons() method is called.
         *
         * \param[in,out] p the Program whose lines will be altered.
         * \param[in] rng Random Number Generator used in the mutation process.
         *
         */
        void insertRandomLine(Program::Program& p, Mutator::RNG& rng);

        /**
         * \brief Swap two randomly selected instructions within the given
         * Program.
         *
         * This function selects two lines of the program randomly and swaps
         * them. If the given Program has less than two lines, nothing happens.
         * Random selection is based on the given Mutator::RNG.
         *
         * This method does NOT update automatically the intron property of the
         * Lines of the Program. Hence, the resulting Program may not execute
         * correctly until Program::identifyIntrons() method is called.
         *
         * \param[in,out] p the Program whose lines will be swapped.
         * \param[in] rng Random Number Generator used in the mutation process.
         * \return true if the lines where successfully swapped, false if the
         *         Program has less than two lines.
         */
        bool swapRandomLines(Program::Program& p, Mutator::RNG& rng);

        /**
         * \brief Alter a randomly selected Line in a given Program.
         *
         * If the given Program has more than 0 Line, this function selects a
         * Line (pseudo)-randomly in a given Program and calls the
         * Mutator::LineMutator:AlterCorrectLine function on it.
         * Random selection is based on the given Mutator::RNG.
         *
         * This method does NOT update automatically the intron property of the
         * Lines of the Program. Hence, the resulting Program may not execute
         * correctly until Program::identifyIntrons() method is called.
         *
         * \param[in,out] p the Program whose line will be altered.
         * \param[in] rng Random Number Generator used in the mutation process.
         * \return true if a line was successfully altered, false if the
         *         Program has less than one line.
         */
        bool alterRandomLine(Program::Program& p, Mutator::RNG& rng);

        /**
         * \brief Alter a program's constant.
         *
         * If the constants are used, this function selects one of them
         * in a pseudo-random way and modifies it
         * Random selection is based on the given Mutator::RNG.
         *
         * This method does NOT update automatically the intron property of the
         * Lines of the Program. Hence, the resulting Program may not execute
         * correctly until Program::identifyIntrons() method is called.
         *
         * \param[in,out] p the Program whose line will be altered.
         * \param[in] params the mutation parameters
         * \param[in] rng Random Number Generator used in the mutation process.
         * \return true if a constant was successfully altered, false if the
         *         Program has less than one line.
         */
        bool alterRandomConstant(Program::Program& p,
                                 const MutationParameters& params,
                                 Mutator::RNG& rng);

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
         * After altering the Program, its intron Line are identified with a
         * call to Program::identifyIntrons().
         *
         * \param[in,out] p the Program whose line will be altered.
         * \param[in] params MutationParameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         * \return true if a modification was performed, false otherwise.
         */
        bool mutateProgram(Program::Program& p,
                           const MutationParameters& params, Mutator::RNG& rng);
    }; // namespace ProgramMutator
};     // namespace Mutator

#endif
