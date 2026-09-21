
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

        /**
         * \brief Abstract base class for activation functions used to transform DataValues.
         */
        struct Function {
             /// \brief Destroys the constraint through its polymorphic interface.
            virtual ~Function() = default;

            /// \brief The output requirement of the activation function.
            Requirement output;
            /// \brief The list of input requirements of the activation function.
            std::vector<Requirement> inputs;

            /**
             * \brief Constructs an activation function with the given input and output requirements.
             * \param inputs The list of requirements for the inputs.
             * \param output The requirement for the output.
             */
            Function(const std::vector<Requirement>& inputs, const Requirement& output): inputs{inputs}, output{output} {}

            /**
             * \brief Clone current function
             */
            virtual std::unique_ptr<Function> cloneUniquePtr() const = 0;

            /**
             * \brief Executes the activation function on the given value.
             * \param value The input DataValue to be transformed.
             * \return The resulting DataValue after applying the activation function.
             */
            virtual Data::DataValue execute(const Data::DataValue& value) const = 0;

            /**
             * \brief Returns the output dimension requirement.
             * \return A reference to the output Requirement.
             */
            virtual const Requirement& outputDimension() const {return output;}

            /**
             * \brief Returns the input dimension requirements.
             * \return A reference to the list of input Requirements.
             */
            virtual const std::vector<Requirement>& inputDimensions() const {return inputs;}

            /**
             * \brief return the name of the function
             */
            virtual std::string name() const  = 0;
        };

            /**
             * \brief Returns the name of the activation function.
             * \return The name of the function as a string.
             */
         /**
          * \brief Hyperbolic tangent (Tanh) activation function scaling values to the range (-1, 1).
          * \tparam T Floating-point type used for the calculation.
          */
        template <typename T>
        struct Tanh final : Function {
            static_assert(
                std::is_floating_point_v<T>,
                 "ActivationFunctions::tanh requires a floating-point type."
             );

             /**
              * \brief Constructs a Tanh activation function for the given input requirement.
              * \param input The requirement of the input.
              */
            Tanh(const Requirement& input)
             : Function({input}, Requirement(input.getDataType(), Dimensions::NumericRange<T>::between(static_cast<T>(-1.0), static_cast<T>(1.0)))) {}

            /**
             * \brief Clone current function
             */
            std::unique_ptr<Function> cloneUniquePtr() const
            {
                return std::make_unique<Tanh<T>>(this->inputs.at(0));
            }

             /**
              * \brief Applies the tanh transformation to the input value.
              * \param value The input DataValue.
              * \return A DataValue with each element scaled by tanh to the range (-1, 1).
              */
            virtual Data::DataValue execute(const Data::DataValue& value) const override {
                // Apply tanh transformation
                size_t count = value.getType().totalElements();
                std::shared_ptr<const T[]> data = value.getData<T>();
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

             /**
              * \brief Returns the name of this function.
              * \return The string "TanH".
              */
            virtual std::string name() const override {
                return "TanH";
            }
        };
            

         /**
          * \brief ArgMax activation function returning the index of the maximum value.
          * \tparam T Numeric type of the input data.
          */
        template <typename T>
        struct ArgMax final : Function {
            static_assert(
                std::is_arithmetic_v<T>,
                 "ActivationFunctions::ArgMax requires a numeric type."
             );

             /**
              * \brief Constructs an ArgMax activation function for the given input requirement.
              * \param input The requirement of the input.
              */
            ArgMax(const Requirement& input)
             : Function({input}, Requirement::scalar<size_t>(Dimensions::NumericRange<size_t>::between(0, input.getDataType().totalElements() - 1))) {}
            /**
             * \brief Clone current function
             */
            std::unique_ptr<Function> cloneUniquePtr() const
            {
                return std::make_unique<ArgMax<T>>(this->inputs.at(0));
            }

             /**
              * \brief Finds the index of the maximum element in the input value.
              * \param value The input DataValue.
              * \return A scalar DataValue holding the index of the maximum element.
              */
            virtual Data::DataValue execute(const Data::DataValue& value) const override {
                // Find argmax index
                size_t count = value.getType().totalElements();
                std::shared_ptr<const T[]> data = value.getData<T>();
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

              /**
               * \brief Returns the name of this function.
               * \return The string "ArgMax".
               */
            virtual std::string name() const override {
                return "ArgMax";
            }
        };
    };
}

#endif