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


#ifdef CODE_GENERATION

#ifndef GEGELATI_LAMBDAPRINTABLEINSTRUCTION_H
#define GEGELATI_LAMBDAPRINTABLEINSTRUCTION_H
#include "PrintableInstruction.h"
namespace Instructions {
    /**
     * \brief Template instruction for simplifying the creation of an
     * PrintableInstruction from a c++ lambda function.
     *
     * Template parameters First and Rest can be any primitive type, class or
     * const c-style 1D and 2D array.
     *
     * Each template parameter corresponds to an argument of the function given
     * to the LambdaInstruction constructor, specifying its type.
     */
    template <typename First, typename... Rest>
    class LambdaPrintableInstruction : public Instructions::PrintableInstruction
    {
      protected:
        /**
         * \brief Function executed for this Instruction.
         */
        const std::function<double(const First, const Rest...)> func;

      public:
        LambdaPrintableInstruction() = delete;

        /**
         * \brief Constructor for the LambdaInstruction.
         *
         * \param[in] format the std::string used to compute the function in the
         * generated code. The result of the function is represented with $0.
         * The first parameter correspond to $1 then $2...
         *
         * \param[in] function the c++ std::function that will be executed for
         * this Instruction. The function must have the same types in its
         * argument list as specified by the template parameters. (checked at
         * compile time)
         *
         **/
        LambdaPrintableInstruction(std::string format, std::function<double(First, Rest...)> function)
        : func{function}
        {
            this->format = format;
            this->operandTypes.push_back(typeid(First));
            // Fold expression to push all other types
            (this->operandTypes.push_back(typeid(Rest)), ...);
        };

        /// Inherited from Instruction
        virtual bool checkOperandTypes(
                const std::vector<Data::UntypedSharedPtr>& arguments) const override
        {
            if (arguments.size() != this->operandTypes.size()) {
                return false;
            }

            // List of expected types
            const std::vector<std::reference_wrapper<const std::type_info>>
                    expectedTypes{
                    // First
                    (!std::is_array<First>::value)
                    ? typeid(First)
                    : typeid(std::remove_all_extents_t<First>[]),
                    (!std::is_array<Rest>::value)
                    ? typeid(Rest)
                    : typeid(std::remove_all_extents_t<Rest>[])...};

            for (auto idx = 0; idx < arguments.size(); idx++) {
                // Argument Type
                const std::type_info& argType = arguments.at(idx).getType();
                if (argType != expectedTypes.at(idx).get()) {
                    return false;
                }
            }

            return true;
        };

        double execute(
                const std::vector<Data::UntypedSharedPtr>& args) const override
        {

#ifndef NDEBUG
            if (Instruction::execute(args) != 1.0) {
                return 0.0;
            }
#endif
            double result =
                    doExecution(args, std::index_sequence_for<Rest...>{});

            return result;
        };

    private:
        /**
         * \brief Template function to handle variadic parameter pack expansion.
         *
         * In order to call the LambdaInstruction::func function, the arguments
         * given as a std::vector to the execute method must be expanded using
         * the template parameter expansion pack. In order to index arguments in
         * args, a second layer of template function is needed to expand an
         * std::index_sequence making it possible to index args at compile time.
         * This is the purpose of this method.
         *
         * \param[in] args The argument for the func execution, stored in an
         * std::vector.
         * \tparam I the std::index_sequence used to access args.
         */
        template <size_t... I>
        double doExecution(const std::vector<Data::UntypedSharedPtr>& args,
                           std::index_sequence<I...>) const
        {
            return this->func(
                    getDataFromUntypedSharedPtr<First>(args, 0),
                    getDataFromUntypedSharedPtr<Rest>(args, I + 1)...);
        }

        /**
         * \brief Function to retrieve the shared pointer from any datatype in
         * the execute method.
         *
         * An inline lambda expression could be used in the execute method, with
         * a variadic parameter pack expansion. Unfortunately not supported by
         * GCC7.5
         *
         * Template parameter T is the Type of the retrieved argument.
         *
         * \param[in] args the UntypedSharedPtr of all arguments.
         * \param[in] idx the current index in the args list.
         * \return the appropriate argument for this->func.
         */
        template <typename T,
                typename MINUS_EXTENT = typename std::remove_extent<T>::type,
                typename RETURN_TYPE = typename std::conditional<
                        !std::is_array<MINUS_EXTENT>::value,
                        typename std::remove_all_extents<T>::type*,
                        MINUS_EXTENT*>::type>
        constexpr auto getDataFromUntypedSharedPtr(
                const std::vector<Data::UntypedSharedPtr>& args, size_t idx) const
        {
            if constexpr (!std::is_array<T>::value) {
                return *(args.at(idx).getSharedPointer<const T>());
            }
            else {
                return (RETURN_TYPE)(
                        args.at(idx)
                                .getSharedPointer<
                                        const std::remove_all_extents_t<T>[]>())
                        .get();
            };
        };
    };
} // namespace Instructions

#endif // GEGELATI_LAMBDAPRINTABLEINSTRUCTION_H
#endif // CODE_GENERATION