/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
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

#ifndef LGP_LINE_H
#define LGP_LINE_H

#include "representation/lgp/environment.h"
#include <cstring>

namespace Representation::LGP {
    /**
     * Class used to store information of a single line of a Program.
     */
    class LGPLine
    {

      protected:
        /// Environment within which the Program will be executed.
        const LGPEnvironment& environment;

        /// index of the Instruction of the Set of the Environment.
        uint64_t instructionIndex;

        /// index of the register to which the result of this line should be
        /// written.
        uint64_t destinationIndex;

        /// Array storing the operands pair (each with an index for the
        /// DataHandlers of the Environment, and a location within it.)
        std::pair<uint64_t, uint64_t>* const operands;

        /// Delete the default constructor.
        LGPLine() = delete;

      public:
        /**
         * \brief Constructor for a LGPLine of a program.
         *
         * Size of attributes will be determined by the given Environment.
         *
         * \param[in] env the const reference to the Environment for this
         * Program::LGPLine.
         */
        LGPLine(const LGPEnvironment& env)
            : environment{env}, instructionIndex{0}, destinationIndex{0},
              operands{(std::pair<uint64_t, uint64_t>*)calloc(
                  env.getMaxNbOperands(),
                  sizeof(std::pair<uint64_t, uint64_t>))} {};

        /**
         * \brief Copy constructor of a LGPLine performing a deep copy.
         *
         * Contrary to the default copy constructor, this one duplicates
         * all pointer based attributes.
         *
         * \param[in] other the const reference to the copied Program::LGPLine.
         */
        LGPLine(const LGPLine& other)
            : environment{other.environment},
              instructionIndex{other.instructionIndex},
              destinationIndex{other.destinationIndex},
              operands{(std::pair<uint64_t, uint64_t>*)calloc(
                  other.environment.getMaxNbOperands(),
                  sizeof(std::pair<uint64_t, uint64_t>))}
        {
            // Check needed to avoid compilation warnings
            if (this->operands != NULL) {
                // Copy operand values
                for (auto idx = 0; idx < this->environment.getMaxNbOperands();
                     idx++) {
                    this->operands[idx] = other.operands[idx];
                }
            }
        };

        /**
         * Disable LGPLine default assignment operator.
         *
         * Until we see the need for it, there si no reason to enable assignment
         * operator of LGPLine
         */
        LGPLine& operator=(const LGPLine& other) = delete;

        /**
         * Destructor of a Program::LGPLine.
         *
         * Dealocates the memory allocated for attributes.
         */
        ~LGPLine()
        {
            free((void*)this->operands);
        }

        /**
         * \brief Get the environment within which the LGPLine was created.
         *
         * \return a const reference to the Environment of the LGPLine.
         */
        const LGPEnvironment& getEnvironment() const;

        /**
         * \brief Getter for the destinationIndex of this LGPLine.
         *
         * \return the value of the destinationIndex attribute.
         */
        uint64_t getDestinationIndex() const;

        /**
         * \brief Setter for the destinationIndex of this LGPLine.
         *
         * Optionnaly, the validity of the given value can be checked with
         * regards to the Environment of the LGPLine. If the given value is not
         * valid (i.e. it exceeds the number of available destinations) the
         * attribute will not be overwritten.
         *
         * \param[in] dest the new value for the destinationIndex attribute.
         * \param[in] check whether the validity of the given new value.
         *
         * \return true if checks are not activated, otherwise, return whether
         * the value was valid and thus the writing was performed or not.
         */
        bool setDestinationIndex(const uint64_t dest, const bool check = true);

        /**
         * \brief Getter for the instructionIndex of this LGPLine.
         *
         * \return the value of the instructionIndex attribute.
         */
        uint64_t getInstructionIndex() const;

        /**
         * \brief Setter for the instructionIndex of this LGPLine.
         *
         * Optionnaly, the validity of the given value can be checked with
         * regards to the Environment of the LGPLine. If the given value is not
         * valid (i.e. it exceeds the number of available instructions) the
         * attribute will not be overwritten.
         *
         * \param[in] instr the new value for the instructionIndex attribute.
         * \param[in] check whether the validity of the given new value.
         *
         * \return true if checks are not activated, otherwise, return whether
         * the value was valid and thus the writing was performed or not.
         */
        bool setInstructionIndex(const uint64_t instr, const bool check = true);

        /**
         *
         * \brief Getter for the operands of this LGPLine.
         *
         * \param[in] idx the index of the accessed operand.
         * \return the value of the operand pair at the given index.
         * \throw std::range_error if the given index exceeds the number of
         * Operands of the LGPLine.
         */
        const std::pair<uint64_t, uint64_t>& getOperand(
            const uint64_t idx) const;

        /**
         * \brief Setter for the operands of this LGPLine.
         *
         * Optionnaly, the validity of the given values can be checked with
         * regards to the Environment of the LGPLine. If the given dataIndex
         * value is not valid (i.e. the dataHandler index exceeds the number
         * of DataHandler of the Environment) the attribute will not be
         * overwritten. The location may exceeds the largestAddressSpace
         * of the indexed DataHandler, since it will be scaled by the
         * programExecutionEngine when fetching the operands, however it.
         * may not exceed the largestAddressSpace of the environment to make
         * it possible to store it with the right number of bits.
         *
         * \param[in] idx the index of the Operand to write.
         * \param[in] dataIndex index of a DataHandler.
         * \param[in] location the location of the operand value within the
         *            DataHandler.
         * \param[in] check whether the validity of the given new value.
         *
         * \return true if checks are not activated, otherwise, return whether
         * the value was valid and thus the writing was performed or not.
         * \throw std::range_error if the given index exceeds the number of
         * Operands of the LGPLine.
         */
        bool setOperand(const uint64_t idx, const uint64_t dataIndex,
                        const uint64_t location, const bool check = true);
        /**
         * \brief Comparison operator between LGPLine.
         *
         * \param[in] other the line with which the current LGPLine is compared.
         * \return true if all attributes (except the Environment) of the two
         * Lines are identical.
         */
        bool operator==(const LGPLine& other) const;

        /**
         * \brief Opposite of the operator==
         */
        bool operator!=(const LGPLine& other) const;
    };
}; // namespace Program



#endif // LGP_LINE
