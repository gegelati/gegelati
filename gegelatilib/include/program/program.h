/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
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

#ifndef PROGRAM_H
#define PROGRAM_H

#include <algorithm>
#include <vector>

#include "data/constantHandler.h"
#include "environment.h"
#include "program/line.h"

namespace Program {
    /**
     * \brief The Program class contains a list of program lines that can be
     * executed within a well defined Environment.
     */
    class Program
    {
      protected:
        /// Environment within which the Program will be executed.
        const Environment& environment;

        /**
         * \brief Lines of the program and intron property.
         *
         * Each element of this vector stores a pointer to a Line, and a
         * boolean value indicating whether this Line is an Intron whithin the
         * program.
         *
         * Introns are Lines of the program that do not contribute to its final
         * result, stored in the first register. Hence, skipping these lines
         * during a program execution can speed up the Program execution.
         */
        std::vector<std::pair<Line*, bool>> lines;

        /**
         *   \brief Constants of the Program
         *
         *   A Program contains a set of constants in a dedicated
         *	 Data::DataHandler
         **/
        Data::ConstantHandler constants;

        /// Delete the default constructor.
        Program() = delete;

      public:
        /**
         * \brief Main constructor of the Program.
         *
         * \param[in] e the reference to the Environment that will be referenced
         * in the Program attributes.
         */
        Program(const Environment& e)
            : environment{e}, constants{e.getNbConstant()}
        {
            constants.resetData(); // force all constant to 0 at first.
        };

        /**
         * \brief Copy constructor of the Program.
         *
         * This copy constructor realises a deep copy of the Line of the given
         * Program, instead of the default shallow copy.
         *
         * \param[in] other a const reference the the copied Program.
         */
        Program(const Program& other)
            : environment{other.environment}, lines{other.lines},
              constants{other.constants}
        {
            // Replace lines with their copy
            // Keep intro info
            std::transform(
                lines.begin(), lines.end(), lines.begin(),
                [](std::pair<Line*, bool>& otherLine)
                    -> std::pair<Line*, bool> {
                    return {new Line(*(otherLine.first)), otherLine.second};
                });
        };

        /**
         * Disable Program default assignment operator.
         *
         * Until we see the need for it, there si no reason to enable assignment
         * operator of Program.
         */
        Program& operator=(const Program& other) = delete;

        /**
         * \brief Destructor for the Program class.
         *
         * This destructor deallocates all memory allocated for Program lines
         * (if any).
         */
        ~Program();

        /**
         * \brief Add a new line to the Program with only 0 bits.
         *
         * The new line is inserted at the end of the program.
         *
         * \return a non-const reference to the newly added Line.
         */
        Line& addNewLine();

        /**
         * \brief Add a new line to the Program with only 0 bits.
         *
         * The new line is inserted at the given position of the program.
         *
         * \param[in] idx the position at which the line should be inserted.
         * \return a non-const reference to the newly added Line.
         * \throw std::out_of_range if the given position is beyond the end of
         * the Program.
         */
        Line& addNewLine(const uint64_t idx);

        /**
         * \brief Clear all intron instructions in the Program.
         *
         * All introns lines from the Program are removed by this method.
         * The behavior of the Program should not be modified after a call
         * to this function.
         *
         * Introns should have been identified before calling this methos, as
         * this method does NOT call the identifyIntrons method.
         */
        void clearIntrons();

        /**
         * \brief Remove a Line from the Program.
         *
         * Remove the Line at the given index from the Program and free the
         * associated memory.
         *
         * \param[in] idx the integer index of the Program Line to remove.
         * \throw std::out_of_range if the index is too large.
         */
        void removeLine(const uint64_t idx);

        /**
         * \brief Swap two existing lines from the Program.
         *
         * \param[in] idx0 the index of the first line to swap.
         * \param[in] idx1 the index of the second line to swap.
         * \throw std::out_of_range if any of the two index is too large.
         */
        void swapLines(const uint64_t idx0, const uint64_t idx1);

        /**
         * \brief Get the environment associated to the Program at construction.
         *
         * \return a const reference to the Environment of the Program and all
         *its
         ** Line.
         */
        const Environment& getEnvironment() const;

        /**
         * \brief Get the number of lines in the Program.
         *
         * \return the number of line contained in the Program.
         */
        size_t getNbLines() const;

        /**
         * \brief Get a const ref to a Line of the Program.
         *
         * \param[in] index The integer index of the retrieved Line within the
         * Program. \return a const reference to the indexed Line of the
         * Program. \throw std::out_of_range if the index is too large.
         */
        const Line& getLine(uint64_t index) const;

        /**
         * \brief Get a non-const ref to a Line of the Program.
         *
         * \param[in] index The integer index of the retrieved Line within the
         * Program. \return a const reference to the indexed Line of the
         * Program. \throw std::out_of_range if the index is too large.
         */
        Line& getLine(uint64_t index);

        /**
         * \brief Checks whether a Line at the given index is an intron.
         *
         * \param[in] index The integer index of the checked Line within the
         * Program.
         * \return true if the Line is an intron, false otherwise.
         * \throw std::out_of_range if the index is too large.
         */
        bool isIntron(uint64_t index) const;

        /**
         * \brief Scan the Line of the Program to identify introns.
         *
         * This method update the boolean value associated to each Line of the
         * Program to indicate if this Line is an intron or not.
         *
         * \return the number of intron Lines idendified.
         */
        uint64_t identifyIntrons();

        /**
         *  \brief get the constantHandler object of the Program
         *
         *  This method gives a reference to the constantHandler associated
         *  with the program
         *
         *  \return the constantHandler of the program
         */
        Data::ConstantHandler& getConstantHandler();

        /**
         *  \brief get a const reference to the constantHandler object of the
         * Program
         *
         *  This method gives a const reference to the constantHandler
         * associated with the program
         *
         *  \return the constantHandler of the program through a const reference
         */
        const Data::ConstantHandler& cGetConstantHandler() const;

        /**
         *	\brief Get the value of a constant at a given index
         *
         *	Although this method is not required as the data is accessible from
         *	the constantHandler, it allows a shortcut and add readability.
         *
         *	\param[in] index the position at which we access the constant
         *	\return the value of the constant at the given index
         */
        const Data::Constant getConstantAt(size_t index) const;

        /**
         * \brief Check if two Program have the same behavior.
         *
         * Two Program have the same behaviour if their sequence of non-intron
         * Lines are strictly identical (i.e. same instructions and operands, in
         * the same order, and used Constant with identical values).
         *
         * \param[in] other the Program whose behavior is compared.
         */
        bool hasIdenticalBehavior(const Program& other) const;
    };
} // namespace Program
#endif
