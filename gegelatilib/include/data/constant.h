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

#ifndef CONSTANT_H
#define CONSTANT_H

#include <cstdint>

namespace Data {
    /**
     * \brief Data type used in Program::Program to define constant values,
     * accessible to Instructions, and mutated during the training process.
     */
    struct Constant
    {
        /**
         *	\brief the value of the Constant
         */
        int32_t value;

        /**
         *	\brief const casts of a Constant to a 32 bits integer
         */
        operator int32_t() const;

        /**
         *	\brief const casts of a Constant to a double
         */
        operator double() const;

        /**
         * \brief Comparison operator for Constant.
         */
        bool operator==(const Constant& other) const;

        /**
         * \brief Comparison operator for Constant.
         */
        bool operator!=(const Constant& other) const;
    };
} // namespace Data

#endif // CONSTANT_H
