
#ifndef ACTIVATION_FUNCTIONS_H
#define ACTIVATION_FUNCTIONS_H

#include <cmath>
#include <vector>

#include "data/dataValue.h"
#include "dimensions/requirement.h"
#include "dimensions/numericRange.h"

namespace Dimensions
{

    
    /// \brief Namespace containing activation functions used for scaling DataValues.
    namespace ActivationFunctions {

        struct Function {
            /// \brief Destroys the constraint through its polymorphic interface.
            virtual ~Function() = default;

            Requirement output;
            std::vector<Requirement> inputs;

            Function(const std::vector<Requirement>& inputs, const Requirement& output): inputs{inputs}, output{output} {}

            virtual Data::DataValue execute(const Data::DataValue& value) const = 0;

            virtual const Requirement& outputDimension() const {return output;}

            virtual const std::vector<Requirement>& inputDimensions() const {return inputs;}
            virtual std::string name() const  = 0;
        };

        template <typename T>
        struct Tanh final : Function {
            static_assert(
                std::is_floating_point_v<T>,
                "ActivationFunctions::tanh requires a floating-point type."
            );


            Tanh(const Requirement& input)
            : Function({input}, Requirement(input.getDataType(), Dimensions::NumericRange<T>::between(static_cast<T>(-1.0), static_cast<T>(1.0)))) {}

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
                } else {
                    return Data::DataValue::array2d(values, value.getType().dimensions[0], value.getType().dimensions[1]);
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


            ArgMax(const Requirement& input)
            : Function({input}, Requirement::scalar<size_t>(Dimensions::NumericRange<size_t>::between(0, input.getDataType().totalElements() - 1))) {}

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