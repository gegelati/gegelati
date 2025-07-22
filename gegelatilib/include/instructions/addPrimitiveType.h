/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2021) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
 * Thomas Bourgoin <tbourgoi@insa-rennes.fr> (2021)
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

#ifndef INSTRUCTION_ADD_H
#define INSTRUCTION_ADD_H

#include <memory>
#include <type_traits>
#include <typeinfo>

#include "data/untypedSharedPtr.h"
#include "instructions/instruction.h"

namespace Instructions {

    /**
     * \brief Template class for add instruction on all types of data: double,
     * int, ...
     */
    template <class T> class AddPrimitiveType : public Instruction
    {
        static_assert(std::is_fundamental<T>::value,
                      "Template class AddPrimitiveType<T> can only be used for "
                      "primitive types.");

#ifdef CODE_GENERATION
      public:
        /**
         * \brief Constructor for the AddPrimitiveType class, so it can be use
         * during the code gen.
         *
         * \param[in] printTemplate std::string use at the generation. Check
         * Instructions::Instruction for more details.
         */
        AddPrimitiveType(const std::string& printTemplate = "");
#endif // CODE_GENERATION

      public:
#ifndef CODE_GENERATION
        /**
         *  \brief Constructor for the AddPrimitiveType class.
         */
        AddPrimitiveType();
#endif // CODE_GENERATION
       /// Inherited from Instruction
        virtual double execute(
            const std::vector<Data::UntypedSharedPtr>& args) const override;

      private:
        /**
         * \brief Function call in constructor to setup the operand
         * of the instruction.
         */
        void setUpOperand();
    };
#ifndef CODE_GENERATION
    template <class T> AddPrimitiveType<T>::AddPrimitiveType()
    {
        setUpOperand();
    }
#endif // CODE_GENERATION
    template <class T>
    double AddPrimitiveType<T>::execute(
        const std::vector<Data::UntypedSharedPtr>& args) const
    {

#ifndef NDEBUG
        if (Instruction::execute(args) != 1.0) {
            return 0.0;
        }
#endif

        return *(args.at(0).getSharedPointer<const T>()) +
               (double)*(args.at(1).getSharedPointer<const T>());
    }

#ifdef CODE_GENERATION
    template <class T>
    AddPrimitiveType<T>::AddPrimitiveType(const std::string& printTemplate)
        : Instruction(printTemplate)
    {
        setUpOperand();
    }
#endif // CODE_GENERATION

    template <class T> void AddPrimitiveType<T>::setUpOperand()
    {
        this->operandTypes.push_back(typeid(T));
        this->operandTypes.push_back(typeid(T));
    }
} // namespace Instructions

#endif
