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

#ifndef INST_MULT_BY_CONST_H
#define INST_MULT_BY_CONST_H

#include <memory>
#include <type_traits>
#include <typeinfo>

#include "data/constantHandler.h"
#include "data/untypedSharedPtr.h"
#include "instruction.h"

namespace Instructions {

    /**
     * \brief Template class for multiplying a unique argument of type T by a
     * constant parameter
     */
    template <class T> class MultByConstant : public Instruction
    {
        static_assert(std::is_fundamental<T>::value,
                      "Template class MultByConstParam<> can only be used for "
                      "primitive types.");

      public:
        /**
         *  \brief Constructor for the MultByConstParam class.
         */
        MultByConstant();

        double execute(
            const std::vector<Data::UntypedSharedPtr>& args) const override;
    };

    template <class T> MultByConstant<T>::MultByConstant()
    {
        this->operandTypes.push_back(typeid(T));
        this->operandTypes.push_back(typeid(Data::Constant));
        this->nbParameters = 1;
    }

    template <class T>
    inline double MultByConstant<T>::execute(
        const std::vector<Data::UntypedSharedPtr>& args) const
    {
#ifndef NDEBUG
        if (Instruction::execute(args) != 1.0)
            return 0;
#endif
        const Data::Constant constantValue = (const Data::Constant&)*(
            args.at(1).getSharedPointer<const Data::Constant>());
        return *(args.at(0).getSharedPointer<const T>()) *
               (double)constantValue;
    }
} // namespace Instructions
#endif // INST_MULT_BY_CONST_H