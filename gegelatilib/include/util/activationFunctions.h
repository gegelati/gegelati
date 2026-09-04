
#ifndef ACTIVATION_FUNCTIONS_H
#define ACTIVATION_FUNCTIONS_H

#include <cmath>
#include "newData/dataValue.h"
#include "newData/numericRange.h"

namespace Utils
{

    
    /// \brief Namespace containing activation functions used for scaling DataValues.
    namespace ActivationFunctions {

        struct Function {
            /// \brief Destroys the constraint through its polymorphic interface.
            virtual ~Function() = default;

            Data::DataRequirement output;
            std::vector<Data::DataRequirement> inputs;

            Function(const std::vector<Data::DataRequirement>& inputs, const Data::DataRequirement& output): inputs{inputs}, output{output} {}

            virtual Data::DataValue execute(const Data::DataValue& value) const = 0;

            virtual const Data::DataRequirement& outputDimension() const {return output;}

            virtual const std::vector<Data::DataRequirement>& inputDimensions() const {return inputs;}
            virtual std::string name() const  = 0;
        };

        template <typename T>
        struct Tanh final : Function {
            static_assert(
                std::is_floating_point_v<T>,
                "ActivationFunctions::tanh requires a floating-point type."
            );


            Tanh(const Data::DataRequirement& input)
            : Function({input}, Data::DataRequirement(input.getDataType(), Data::NumericRange<T>::between(static_cast<T>(-1.0), static_cast<T>(1.0)))) {}

            virtual Data::DataValue execute(const Data::DataValue& value) const override {
                // Apply tanh transformation
                size_t count = value.getType().totalElements();
                const T* data = value.getData<T>();
                std::vector<T> values(count);
                for (size_t idx = 0; idx < count; ++idx) {
                    values[idx] = static_cast<T>(std::tanh(data[idx]));
                }

                if (value.getRank() == 0) {
                    return Data::DataValue::scalar<T>(values[0]);
                } else if (value.getRank() == 1) {
                    return Data::DataValue::array1d(values);
                } else if (value.getRank() == 2) {
                    return Data::DataValue::array2d(values, value.getType().dimensions[0], value.getType().dimensions[1]);
                } else {
                    throw std::invalid_argument(
                        "ActivationFunctions::Tanh failed: unsupported value rank."
                    );
                }

            }


            virtual std::string name() const override {
                return "TanH";
            }
        };
            

        template <typename T>
        struct ArgMax final : Function {
            static_assert(
                std::is_arithmetic_v<T>,
                "ActivationFunctions::ArgMax requires a numeric type."
            );


            ArgMax(const Data::DataRequirement& input)
            : Function({input}, Data::DataRequirement(Data::DataRequirement::scalar<size_t>(Data::NumericRange<size_t>::between(0, input.getDataType().totalElements() - 1)))) {}

            virtual Data::DataValue execute(const Data::DataValue& value) const override {
                // Find argmax index
                size_t count = value.getType().totalElements();
                const T* data = value.getData<T>();
                size_t index = 0;
                T max = data[0];

                for (size_t idx = 1; idx < count; ++idx) {
                    if (data[idx] > max) {
                        max = data[idx];
                        index = idx;
                    }
                }
                return Data::DataValue::scalar<size_t>(index);
            }   

            virtual std::string name() const override {
                return "ArgMax";
            }
        };
    };
}

#endif