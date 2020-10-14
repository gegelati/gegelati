/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019)
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

#ifndef CONSTANT_HANDLER_H
#define CONSTANT_HANDLER_H

#include <functional>
#include <memory>
#include <typeinfo>
#include <vector>

#include "data/constant.h"
#include "data/primitiveTypeArray.h"
#include "data/untypedSharedPtr.h"

namespace Data {

    /**
     * \brief Data::DataHandler used by Program::Program to handle their set of
     * Constant values.
     */
    class ConstantHandler : public PrimitiveTypeArray<Constant>
    {
      public:
        /**
         * \brief Default constructor of the ConstantHandler class.
         */
        ConstantHandler(size_t nb_constants)
            : PrimitiveTypeArray<Constant>{nb_constants} {};

        /// Default destructor
        virtual ~ConstantHandler() = default;

        /**
         * \brief Default copy constructor.
         */
        ConstantHandler(const ConstantHandler& other) = default;
    };
} // namespace Data

#endif
