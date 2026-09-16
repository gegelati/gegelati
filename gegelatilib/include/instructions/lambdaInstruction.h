#ifndef LAMBDA_INSTRUCTION_H
#define LAMBDA_INSTRUCTION_H

#include <functional>
#include <stdexcept>
#include <type_traits>
#include <typeinfo>
#include <iostream>

#include "instructions/instruction.h"

namespace Instructions {

    /**
     * \brief An \c Instruction that evaluates a user-supplied callable (lambda or
     *        function object) over its operands.
     *
     * \tparam Output The declared output type of the instruction. Its fundamental
     *                element type must be fundamental (see the static_assert).
     * \tparam First  The type of the first (primary) operand.
     * \tparam Rest   The types of the remaining operands, in order.
     *
     * The stored callable is invoked with one argument per declared operand and
     * must return either a \c Data::DataValue or a scalar/array value that is
     * wrapped into a \c Data::DataValue automatically.
     */
    template <typename Output, typename First, typename... Rest>
    class LambdaInstruction : public Instruction
     {
        static_assert(
            std::is_fundamental<std::remove_all_extents_t<Output>>::value,
             "LambdaInstruction output must contain a fundamental type.");

      protected:
          /// \brief The wrapped callable invoked by \ref execute.
          ///
          /// It accepts the operands as `First` followed by the `Rest...` pack and
          /// returns a \c Data::DataValue.
        std::function<Data::DataValue(const First, const Rest...)> function;

      public:
        /// \brief Deleted default constructor.
        ///
        /// A \c LambdaInstruction must be created with a callable; default
        /// construction is therefore forbidden.
        LambdaInstruction() = delete;

        /**
         * \brief Constructs a \c LambdaInstruction from an arbitrary invocable.
         *
         * This is the constructor used for all callable types, including
         * lambdas, function pointers, functors, and \c std::function objects.
         *
         * The callable is adapted so that it receives the operands positionally
         * (\c First followed by the \c Rest... pack).
         *
         * The adapted callable inspects the return type of the user's function
         * at compile time:
         * - If the callable already returns a \c Data::DataValue, that value is
         *   forwarded unchanged.
         * - Otherwise, the scalar/array result is automatically wrapped into a
         *   \c Data::DataValue via \c Data::DataValue::scalar.
         *
         * The \c std::enable_if_t guard restricts this constructor to callables
         * that are invocable with the declared operand types.
         *
         * When \c CODE_GENERATION is defined, an optional \a printTemplate is
         * forwarded to the base \c Instruction to provide a formatting hint
         * for generated code.
         *
         * \tparam Function The type of the supplied callable. The constructor is
         *         only enabled when \c Function is invocable with the declared
         *         operand types (\c First followed by the \c Rest... pack).
         *
         * \param function The callable to evaluate over the operands.
         *
         * \param printTemplate Optional print template forwarded to the base
         *                      \c Instruction when \c CODE_GENERATION is defined.
         */
        template <typename Function,
                typename = std::enable_if_t<
                    std::is_invocable_v<Function, First, Rest...>>>
#ifdef CODE_GENERATION
        LambdaInstruction(Function function, const std::string& printTemplate = "")
            :Instruction(printTemplate),
#else
        LambdaInstruction(Function function) :Instruction(),
#endif
        
            function{[function](const First first, const Rest... rest) {
                using Return =
                    std::decay_t<
                        std::invoke_result_t<Function, First, Rest...>>;

                if constexpr (std::is_same_v<Return, Data::DataValue>) {
                    return function(first, rest...);
                } else {
                    return Data::DataValue::scalar(function(first, rest...));
                }
            }}
        {
            setUpOperand();
        }

           /**
            * \brief Validates that the supplied arguments match the declared operands.
            *
            * Checks that the number of arguments equals the number of declared
            * operand types and that each argument's data type matches the
            * corresponding operand type.
            *
            * \param arguments The argument data views to validate.
            * \return True if the count and every argument type match; otherwise
            *         false.
             */
        bool checkOperandTypes(
            const std::vector<Data::DataView>& arguments) const override
        {
            if (arguments.size() != this->operandTypes.size()) {
                return false;
            }
            for (size_t index = 0; index < arguments.size(); index++) {
                if (arguments.at(index).getType() !=
                    this->operandTypes.at(index)) {
                    return false;
                }
            }
            return true;
        }
             /**
              * \brief Executes the instruction by invoking the stored callable.
              *
              * The supplied arguments are validated via \ref checkOperandTypes, then
              * the wrapped callable is invoked through doExecution. For a
              * non-array \c Output the resulting element type is checked against
              * \c typeid(Output); for an array \c Output the result must declare a
              * non-empty type.
              *
              * \param args The argument data views to evaluate.
              * \return The resulting \c Data::DataValue produced by the callable.
              * \throw std::invalid_argument If the operand types do not match, the
              *        result element type does not match the declared \c Output, or an
              *        array result has no declared type.
               */
        Data::DataValue execute(
            const std::vector<Data::DataView>& args) const override
        {
            if (!this->checkOperandTypes(args)) {
                throw std::invalid_argument(
                    "LambdaInstruction::execute: Instruction operand type mismatch.");
            }

            Data::DataValue result =
                doExecution(args, std::index_sequence_for<Rest...>{});

            if constexpr (!std::is_array<Output>::value) {
                if (*result.getType().elementType != typeid(Output)) {
                    throw std::invalid_argument(
                        "LambdaInstruction::execute: Lambda result type does not match declared output.");
                }
            }
            else if (result.getType().dimensions[0] == 0) {
                throw std::invalid_argument(
                    "LambdaInstruction::execute: Lambda array result has no declared type.");
            }

            return result;
        }

      private:
             /**
              * \brief Invokes the wrapped callable by unpacking the arguments.
              *
              * The first argument is extracted as `First` and the remaining
              * arguments are extracted as the `Rest...` pack using a compiled
              * index sequence, then forwarded to the stored callable.
              *
              * \tparam Index Compile-time index pack spanning the `Rest...` operands.
              * \param args The argument data views to unpack.
              * \param [in] indexSequence The compile-time index sequence used to
              *                           extract the `Rest...` arguments.
              * \return The \c Data::DataValue returned by the callable.
              */
        template <size_t... Index>
        Data::DataValue doExecution(
            const std::vector<Data::DataView>& args,
            std::index_sequence<Index...>) const
          {
            return this->function(getData<First>(args, 0),
                                  getData<Rest>(args, Index + 1)...);
          }

             /**
              * \brief Extracts a typed value from an argument data view.
              *
              * Scalar arguments are returned by reference via
              * \c DataView::getScalar. One-dimensional arrays are returned as a
              * pointer to their element type, and two-dimensional arrays are
              * returned as a pointer to a row of the fixed column count.
              *
              * \tparam T The operand type (scalar, 1D array, or 2D array).
              * \param args The argument data views.
              * \param index The index of the argument to extract.
              * \return The extracted value: a scalar reference, a 1D array pointer,
              *         or a 2D row-array pointer depending on \c T.
              */
        template <typename T>
        static auto getData(const std::vector<Data::DataView>& args, size_t index) {
            const Data::DataView& view = args.at(index);
            if constexpr (std::is_array<T>::value) {
                if constexpr (std::rank_v<T> == 1) {
                    return view.template getData<std::remove_extent_t<T>>(); // Returns const T*
                } else if constexpr (std::rank_v<T> >= 2) {
                    using Element = std::remove_const_t<std::remove_all_extents_t<T>>;

                    constexpr size_t Cols = std::extent_v<T, 1>;
                    const Element* data = view.template getData<Element>();

                    return reinterpret_cast<const Element (*)[Cols]>(data);
                }
            } else {
                return view.template getScalar<std::remove_const_t<T>>(); // Returns const T&
            }
        }
               /**
                * \brief Determines the \c Data::DataType matching a given operand type.
                *
                * Scalar types map to a scalar \c DataType, 1D arrays to a 1D data type,
                * and 2D arrays to a 2D data type built from the compile-time
                * dimensions.
                *
                * \tparam T The operand type to classify.
                * \return The corresponding \c Data::DataType.
                */
        template <typename T> static const Data::DataType operandType()
        {
            if constexpr (std::is_array<T>::value) {
                constexpr size_t first_dim = std::extent<T>::value;
                using ElementType = std::remove_extent_t<T>;

                if constexpr (std::is_array<ElementType>::value) {
                    // 2D array: T[first_dim][second_dim]
                    constexpr size_t second_dim = std::extent<ElementType>::value;
                    return Data::DataType::array2d<std::remove_all_extents_t<T>>(
                        first_dim, second_dim
                    );
                } else {
                    // 1D array: T[first_dim]
                    return Data::DataType::array1d<std::remove_all_extents_t<T>>(
                        first_dim
                    );
                }
            }
            else {
                return Data::DataType::scalar<T>();
            }
        }
            /**
             * \brief Populates the declared operand types.
             *
             * Registers the \c DataType of `First` and of each `Rest...` operand in
             * the base \c Instruction's operand type list.
             */
        void setUpOperand()
        {
            this->operandTypes.push_back(operandType<First>());
            (this->operandTypes.push_back(operandType<Rest>()), ...);
        }
    };

} // namespace Instructions

#endif
