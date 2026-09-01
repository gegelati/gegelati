#ifndef LAMBDA_INSTRUCTION_H
#define LAMBDA_INSTRUCTION_H

#include <functional>
#include <stdexcept>
#include <type_traits>
#include <typeinfo>
#include <iostream>

#include "instructions/instruction.h"

namespace Instructions {

    template <typename First, typename... Rest>
    class LambdaInstruction : public Instruction
    {
      protected:
        std::function<Data::DataValue(const First, const Rest...)> function;

      public:
        LambdaInstruction() = delete;

#ifdef CODE_GENERATION
        LambdaInstruction(std::function<double(First, Rest...)> function,
                          const std::string& printTemplate = "")
            : Instruction(printTemplate),
              function{[function](const First first, const Rest... rest) {
                  return Data::DataValue::scalar(function(first, rest...));
              }}
        {
            setUpOperand();
        }
#else
        LambdaInstruction(std::function<double(First, Rest...)> function)
            : Instruction(),
              function{[function](const First first, const Rest... rest) {
                  return Data::DataValue::scalar(function(first, rest...));
              }}
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
              function{[function](const First first, const Rest... rest) {
                  using Return =
                      std::decay_t<std::invoke_result_t<Function, First, Rest...>>;
                  if constexpr (std::is_same_v<Return, Data::DataValue>) {
                      return function(first, rest...);
                  }
                  else {
                      return Data::DataValue::scalar(function(first, rest...));
                  }
              }}
        {
            setUpOperand();
        }

        bool checkOperandTypes(
            const std::vector<Data::DataView>& arguments) const override
        {
            if (arguments.size() != this->operandTypes.size()) {
                std::cout<<"here"<<std::endl;
                return false;
            }
            for (size_t index = 0; index < arguments.size(); index++) {
                if (arguments.at(index).getType() !=
                    this->operandTypes.at(index)) {
                        std::cout<<"here2 "<<index<<" "<<arguments.at(index).getElementType().name()<<" " <<this->operandTypes.at(index).elementType->name()<<std::endl;
                    return false;
                }
            }
            return true;
        }

        Data::DataValue execute(
            const std::vector<Data::DataView>& args) const override
        {
            if (!this->checkOperandTypes(args)) {
                throw std::invalid_argument("LambdaInstruction::execute: Instruction operand type mismatch.");
            }
            return doExecution(args, std::index_sequence_for<Rest...>{});
        }

      private:
        template <size_t... Index>
        Data::DataValue doExecution(
            const std::vector<Data::DataView>& args,
            std::index_sequence<Index...>) const
        {
            return this->function(getData<First>(args, 0),
                                  getData<Rest>(args, Index + 1)...);
        }

        template <typename T>
        static auto getData(const std::vector<Data::DataView>& args, size_t index) {
            const Data::DataView& view = args.at(index);
            if constexpr (std::is_array<T>::value) {
                if constexpr (std::rank_v<T> == 1) {
                    return view.template getArray<std::remove_extent_t<T>>(); // Returns const T*
                } else if constexpr (std::rank_v<T> >= 2) {
                    using Element = std::remove_const_t<std::remove_all_extents_t<T>>;

                    constexpr size_t Cols = std::extent_v<T, 1>;
                    const Element* data = view.template getArray<Element>();

                    return reinterpret_cast<const Element (*)[Cols]>(data);
                }
            } else {
                return view.template getScalar<std::remove_const_t<T>>(); // Returns const T&
            }
        }

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

        void setUpOperand()
        {
            this->operandTypes.push_back(operandType<First>());
            (this->operandTypes.push_back(operandType<Rest>()), ...);
        }
    };

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

        Data::DataValue execute(
            const std::vector<Data::DataView>& args) const override
        {
            Data::DataValue result =
                LambdaInstruction<First, Rest...>::execute(args);
            if constexpr (!std::is_array<Output>::value) {
                if (*result.getType().elementType != typeid(Output)) {
                    throw std::invalid_argument(
                        "TypedLambdaInstruction:Execute: Lambda result type does not match declared output.");
                }
            }
            else if (result.getType().dimensions[0] == 0) {
                throw std::invalid_argument(
                    "TypedLambdaInstruction:Execute: Lambda array result has no declared type.");
            }
            return result;
        }
    };

} // namespace Instructions

#endif
