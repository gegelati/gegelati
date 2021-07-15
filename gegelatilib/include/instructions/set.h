/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019)
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

#ifndef SET_H
#define SET_H

#include <functional>

#include "instructions/instruction.h"

namespace Instructions {
    /**
     * \brief Class for storing a set of Instruction.
     *
     * Together with storage capabilities, this class provide methods to compute
     * useful information on the ordered set of Instruction it contains.
     */
    class Set
    {
      protected:
        /**
         * \brief Ordered container of the Instruction of the set.
         */
        std::vector<std::reference_wrapper<const Instruction>> instructions;

      public:
        /**
         * \brief Default constructor.
         *
         * This constructor simply creates an empty list of instructions.
         */
        Set() : instructions{} {};

        /**
         * \brief Add an instruction to the set.
         *
         * Instruction added to the set are stored and indexed in the order in
         * which they are added.
         *
         * \param[in] instruction the Instruction to be added to the Set.
         * \return true if the instruction was successfully added.
         */
        bool add(const Instruction& instruction);

        /**
         * \brief Get the number of Instruction contained in the Set.
         *
         * \return the int corresponding to the number of Instruction in the
         * Set.
         */
        unsigned int getNbInstructions() const;

        /**
         * \brief Get the Instruction at the given index i in the Set.
         *
         * \param[in] i the unsigned integer index of the Instruction to
         * retrieve. \throws std::out_of_range if the given index exceeds the
         * number of Instruction in the Set. \return a reference to the
         * Instruction at the given index.
         */
        const Instruction& getInstruction(const uint64_t i) const;

        /**
         * \brief Get the maximum number of operands for Instruction currently
         * contained in the Set.
         *
         * \return the unsigned int value corresponding to the largest number
         * of operands needed to execute an Instruction in the Set.
         */
        unsigned int getMaxNbOperands() const;
    };
} // namespace Instructions

#endif
