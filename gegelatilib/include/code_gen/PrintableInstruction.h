/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
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

#ifdef CODE_GENERATION

#ifndef GEGELATI_PRINTABLEINSTRUCTION_H
#define GEGELATI_PRINTABLEINSTRUCTION_H
#include <string>
#include "instructions/instruction.h"

namespace Instructions {
    /**
     * \brief This abstract class is the base class for any instruction to be
     * used for the code generation.
     *
     * A PrintableInstruction declares a format that will be used to generate
     * the equivalent C code. The data in the format are represented with
     * a $ followed by a number. $0 correspond to the result of the function
     *
     */
    class PrintableInstruction : public virtual Instructions::Instruction
    {
      public:
//        bool isFormatValid();

        /**
        * \brief This function return the format of the line of code used to
        * represent the instruction
        *
        *
        */
        const std::string& getFormat() const;
//todo del
        /**
        * \brief This function return true for a PrintableInstruction and all
        * its derivative class
        *
        *
        */

        bool isPrintable() const override;
      protected:
        /**
        * \brief Protected constructor to force the class abstract nature.
        *
        * The definition of this constructor initialize the string format with
        * the format given as parameter
        *
        * \param[in] format of the line used to represent the instruction in the
        * C files generated
        *
        */
        PrintableInstruction(std::string format): format(format){};

        /**
         * \brief delete default constructor
         *
         * A printableInstruction without any format does not make sense
         */

        PrintableInstruction() = delete;

        /// format of the instruction used to generate the code
        std::string format;
    };
} // namespace Instructions

#endif // GEGELATI_PRINTABLEINSTRUCTION_H

#endif // CODE_GENERATION
