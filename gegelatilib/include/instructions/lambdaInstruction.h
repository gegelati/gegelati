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
                setUpOutput();
                setUpOperand();
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

                return doExecution(args, std::index_sequence_for<Rest...>{});
            }

      private:
            /**
             * \brief Invokes the wrapped callable with the data extracted from the operands.
             *
             * Array data returned by \c DataView::getData is temporarily owned by the
             * \c shared_ptr objects stored in \c data. This ensures that any temporary
             * contiguous copy created for a non-contiguous view remains alive until the
             * callable has finished executing.
             *
             * Scalars are passed directly, while array arguments are converted back to
             * the raw pointer types expected by the wrapped callable.
             *
             * \tparam Index Compile-time indices used to extract the remaining operands.
             * \param args The operand data views.
             * \param indexSequence Compile-time indices for the remaining operands.
             * \return The \c Data::DataValue returned by the callable.
             */
            template <size_t... Index>
            Data::DataValue doExecution(
                const std::vector<Data::DataView>& args,
                std::index_sequence<Index...>) const
            {
                // Keep array data alive until the callable has finished.
                auto data = std::make_tuple(
                    getData<First>(args[0]),
                    getData<Rest>(args[Index + 1])...
                );

                return this->function(
                    getDataPtr<First>(std::get<0>(data)),
                    getDataPtr<Rest>(std::get<Index + 1>(data))...
                );
            }

            /**
             * \brief Extracts an operand from a data view.
             *
             * Scalar operands are returned directly through \c DataView::getScalar.
             * Array operands are returned as a \c shared_ptr owning either the original
             * data through a non-owning deleter or a temporary contiguous copy when the
             * view is non-contiguous.
             *
             * The returned \c shared_ptr is kept alive by \c doExecution while the
             * callable is being invoked.
             *
             * \tparam T The declared operand type.
             * \param view The data view containing the operand.
             * \return The scalar value/reference or shared array data.
             */
            template <typename T>
            static auto getData(const Data::DataView& view)
            {
                if constexpr (!std::is_array_v<T>) {
                    return view.template getScalar<std::remove_cv_t<T>>();
                }
                else {
                    using Element = std::remove_all_extents_t<T>;

                    return view.template getData<std::remove_cv_t<Element>>();
                }
            }

            /**
             * \brief Converts extracted operand data to the type expected by the callable.
             *
             * Scalar data is returned unchanged. One-dimensional arrays are converted
             * to pointers to their first element. Two-dimensional arrays are converted
             * to pointers to rows with their compile-time column count.
             *
             * For array operands, the \c shared_ptr itself is not returned because the
             * callable expects a raw pointer. The owning \c shared_ptr remains alive in
             * \c doExecution for the duration of the call.
             *
             * \tparam T The declared operand type.
             * \tparam Data The type returned by \c getData.
             * \param data The extracted operand data.
             * \return The value or raw pointer expected by the callable.
             */
            template <typename T, typename Data>
            static auto getDataPtr(Data& data)
            {
                if constexpr (!std::is_array_v<T>) {
                    return data;
                }
                else if constexpr (std::rank_v<T> == 1) {
                    return data.get();
                }
                else {
                    using Element = std::remove_all_extents_t<T>;
                    constexpr size_t Cols = std::extent_v<T, 1>;

                    return reinterpret_cast<const Element (*)[Cols]>(data.get());
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
             * \brief Defines the declared output type.
             *
             * Registers the \c DataType of `First` and of each `Rest...` operand in
             * the base \c Instruction's operand type list.
             */
            void setUpOutput()
            {
                this->outputType = operandType<Output>();
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
