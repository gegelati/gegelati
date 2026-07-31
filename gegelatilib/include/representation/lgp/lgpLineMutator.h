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

#ifndef LGP_LINE_MUTATOR_H
#define LGP_LINE_MUTATOR_H

#include <typeinfo>

#include "mutator/rng.h"
#include "representation/lgp/lgpLine.h"

namespace Representation::LGP {
    /**
     * Class containing all functions to apply randomness to a LGPLine
     */
    class LGPLineMutator {
    
    public:
        /**
         * \brief Function to initialize a single operand of a LGPLine.
         *
         * This function uses the RNG::RNG to set (if possible) the nth operand
         * pair of a given Line to a valid value according to the provided Environment
         * and the selected Instruction.
         *
         * \param[in] instruction the selected Instruction for this line.
         * \param[in,out] line the LGPLine  to initialize.
         * \param[in] operandIdx the index of the operand of the Line to initialize.
         * \param[in] initOperandDataSource should the operand data source be
         * (re-)initialized?
         * \param[in] initOperandLocation should the operand location
         * be (re)initialized?
         * \param[in] forceChange should the value be mandatorily
         * new?
         * \param[in] rng Random Number Generator used in the mutation process.
         * \return true if the operand was successfully initialized, false if no valid
         * data source could be found for this Instruction and operandIdx couple.
         */
        bool initRandomCorrectLineOperand(
            const Instructions::Instruction& instruction, LGPLine& line,
            const uint64_t& operandIdx, const bool initOperandDataSource,
            const bool initOperandLocation, const bool forceChange, RNG::RNG& rng);

        /**
         * \brief Randomly initializes a LGPLine.
         *
         * Using the RNG::RNG functions, this function initializes all
         * attributes of the given LGPLine to randomly selected values.
         * The attributes are initialized in such a way that the created line
         * is executable within its Environment. This means that the
         * instructionIndex, the destinationIndex, all operands pairs (sources
         * and localisation) will be initialized randomly such
         * that the corresponding instruction can be executed with valid data.
         * Operands of the line that are unused by the selected
         * Instruction will be initialized
         *
         * \param[in,out] line the LGPLine whose attributes are being
         * initialized.
         * \param[in] rng Random Number Generator used in the mutation process.
         * \throw std::runtime_error if the provided Environment has no
         * dataSource that can successfully provide data for any of its
         * Instruction.
         */
        void initRandomCorrectLine(LGPLine& line, RNG::RNG& rng);

        /**
         * \brief This function randomly modifies the behavior of a
         * LGPLine.
         *
         * Probability of altering the different attributes of the Line is
         * computed as if the given LGPLine was encoded with a word of
         * computeLineSize() bits, and a single bit must be
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
         *
         * \param[in,out] line the LGPLine whose attributes are being
         * altered.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        void alterCorrectLine(LGPLine& line, RNG::RNG& rng);
    }; // class LineMutator
}; // namespace Mutator
#endif
