#ifndef LAMBDA_INSTRUCTION_H
#define LAMBDA_INSTRUCTION_H

#include <functional>
#include <stdexcept>
#include <type_traits>
#include <typeinfo>

#include "data/untypedSharedPtr.h"
#include "instructions/instruction.h"

namespace Instructions {

    template <typename First, typename... Rest>
    class LambdaInstruction : public Instruction
    {
      protected:
        const std::function<double(const First, const Rest...)> func;
        std::function<Data::UntypedSharedPtr(const First, const Rest...)>
            resultFunc;

      public:
        LambdaInstruction() = delete;

#ifdef CODE_GENERATION
        LambdaInstruction(std::function<double(First, Rest...)> function,
                          const std::string& printTemplate = "")
            : Instruction(printTemplate), func{function}
        {
            setUpOperand();
        }
#else
        LambdaInstruction(std::function<double(First, Rest...)> function)
            : Instruction(), func{function}
        {
            setUpOperand();
        }
#endif

        template <typename Function,
                  typename = std::enable_if_t<
                      std::is_invocable_v<Function, First, Rest...>>>
        LambdaInstruction(Function function)
#ifdef CODE_GENERATION
            : Instruction(""),
#else
            : Instruction(),
#endif
              func{[function](const First first, const Rest... rest) {
                  using Return = std::invoke_result_t<Function, First, Rest...>;
                  if constexpr (std::is_convertible_v<Return, double>) {
                      return static_cast<double>(function(first, rest...));
                  }
                  return 0.0;
              }},
              resultFunc{[function](const First first, const Rest... rest) {
                  using Return = std::invoke_result_t<Function, First, Rest...>;
                  if constexpr (std::is_same_v<std::decay_t<Return>,
                                               Data::UntypedSharedPtr>) {
                      return function(first, rest...);
                  }
                  else {
                      using StoredReturn = std::decay_t<Return>;
                      return Data::UntypedSharedPtr{
                          new StoredReturn(function(first, rest...))};
                  }
              }}
        {
            setUpOperand();
        }

        bool checkOperandTypes(
            const std::vector<Data::UntypedSharedPtr>& arguments) const override
        {
            if (arguments.size() != this->operandTypes.size()) {
                return false;
            }
            const std::vector<std::reference_wrapper<const std::type_info>>
                expectedTypes{
                    (!std::is_array<First>::value)
                        ? typeid(First)
                        : typeid(std::remove_all_extents_t<First>[]),
                    (!std::is_array<Rest>::value)
                        ? typeid(Rest)
                        : typeid(std::remove_all_extents_t<Rest>[])...};
            for (size_t index = 0; index < arguments.size(); index++) {
                if (arguments.at(index).getType() !=
                    expectedTypes.at(index).get()) {
                    return false;
                }
            }
            return true;
        }

        double execute(
            const std::vector<Data::UntypedSharedPtr>& args) const override
        {
#ifndef NDEBUG
            if (!this->checkOperandTypes(args)) {
                return 0.0;
            }
#endif
            return doExecution(args, std::index_sequence_for<Rest...>{});
        }

        Data::UntypedSharedPtr executeResult(
            const std::vector<Data::UntypedSharedPtr>& args) const override
        {
#ifndef NDEBUG
            if (!this->checkOperandTypes(args)) {
                return Data::UntypedSharedPtr{new double(0.0)};
            }
#endif
            return doResultExecution(args,
                                     std::index_sequence_for<Rest...>{});
        }

      private:
        template <size_t... Index>
        double doExecution(const std::vector<Data::UntypedSharedPtr>& args,
                           std::index_sequence<Index...>) const
        {
            return this->func(
                getData<First>(args, 0), getData<Rest>(args, Index + 1)...);
        }

        template <size_t... Index>
        Data::UntypedSharedPtr doResultExecution(
            const std::vector<Data::UntypedSharedPtr>& args,
            std::index_sequence<Index...>) const
        {
            return this->resultFunc(
                getData<First>(args, 0), getData<Rest>(args, Index + 1)...);
        }

        template <typename T,
                  typename MINUS_EXTENT = typename std::remove_extent<T>::type,
                  typename RETURN_TYPE = typename std::conditional<
                      !std::is_array<MINUS_EXTENT>::value,
                      typename std::remove_all_extents<T>::type*,
                      MINUS_EXTENT*>::type>
        static auto getData(const std::vector<Data::UntypedSharedPtr>& args,
                            size_t index)
        {
            if constexpr (std::is_array<T>::value) {
                auto returnedPtr = args.at(index)
                    .template getSharedPointer<
                        const std::remove_all_extents_t<T>[]>();
                return (RETURN_TYPE)returnedPtr.get();
            }
            else {
                return *args.at(index).template getSharedPointer<const T>();
            }
        }

        void setUpOperand()
        {
            this->operandTypes.push_back(typeid(First));
            (this->operandTypes.push_back(typeid(Rest)), ...);
        }
    };

    /** LambdaInstruction variant with an explicitly declared output type. */
    template <typename Output, typename First, typename... Rest>
    class TypedLambdaInstruction : public LambdaInstruction<First, Rest...>
    {
        static_assert(
            std::is_fundamental<std::remove_all_extents_t<Output>>::value,
            "TypedLambdaInstruction output must contain a fundamental type.");

      public:
        template <typename Function>
        explicit TypedLambdaInstruction(Function function)
            : LambdaInstruction<First, Rest...>(function)
        {
        }

        Data::UntypedSharedPtr executeResult(
            const std::vector<Data::UntypedSharedPtr>& args) const override
        {
            Data::UntypedSharedPtr result =
                LambdaInstruction<First, Rest...>::executeResult(args);
            if constexpr (!std::is_array<Output>::value) {
                if (result.getType() != typeid(Output)) {
                    throw std::invalid_argument(
                        "Lambda result type does not match declared output.");
                }
            }
            else if (result.getRank() == 0) {
                throw std::invalid_argument(
                    "Lambda array result has no declared shape.");
            }
            return result;
        }
    };

} // namespace Instructions

#endif
