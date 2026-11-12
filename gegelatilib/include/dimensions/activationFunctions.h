
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
                auto data = value.getData<T>();
                std::vector<T> values(count);
                for (size_t idx = 0; idx < count; ++idx) {
                    values[idx] = static_cast<T>(std::tanh(data[idx]));
                }

                return Data::DataValue::fromRange(values, value.getType());
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
         * \brief Logistic (Sigmoid) activation function scaling values to the range (0, 1).
         * \tparam T Floating-point type used for the calculation.
         */
        template <typename T>
        struct Sigmoid final : Function {
            static_assert(
                std::is_floating_point_v<T>,
                "ActivationFunctions::Sigmoid requires a floating-point type."
            );

            /**
             * \brief Constructs a Sigmoid activation function for the given input requirement.
             * \param input The requirement of the input.
             */
            Sigmoid(const Requirement& input)
             : Function({input}, Requirement(input.getDataType(), Dimensions::NumericRange<T>::between(static_cast<T>(0.0), static_cast<T>(1.0)))) {}

            /**
             * \brief Clone current function
             */
            std::unique_ptr<Function> cloneUniquePtr() const override
            {
                return std::make_unique<Sigmoid<T>>(this->inputs.at(0));
            }

            /**
             * \brief Applies the logistic transformation 1 / (1 + exp(-x)) to each element.
             * \param value The input DataValue.
             * \return A DataValue with each element scaled to the range [0, 1].
             */
            virtual Data::DataValue execute(const Data::DataValue& value) const override {
                size_t count = value.getType().totalElements();
                auto data = value.getData<T>();
                std::vector<T> values(count);
                for (size_t idx = 0; idx < count; ++idx) {
                    // Two branches avoid overflowing exp() for large magnitudes.
                    const T x = data[idx];
                    if (x >= static_cast<T>(0.0)) {
                        values[idx] = static_cast<T>(1.0) / (static_cast<T>(1.0) + std::exp(-x));
                    } else {
                        const T e = std::exp(x);
                        values[idx] = e / (static_cast<T>(1.0) + e);
                    }
                }

                return Data::DataValue::fromRange(values, value.getType());
            }

            /**
             * \brief Returns the name of this function.
             * \return The string "Sigmoid".
             */
            virtual std::string name() const override {
                return "Sigmoid";
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
             : Function({input}, Requirement(Data::DataType::type<size_t>(), Dimensions::NumericRange<size_t>::between(0, input.getDataType().totalElements() - 1))) {}
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
                auto data = value.getData<T>();
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

        /**
        * \brief Sum reduction over specified axes, or over all elements.
        *
        * An empty axes list performs a global sum and returns a scalar.
        * Otherwise, the specified axes are removed from the output shape.
        *
        * Examples:
        * - Shape {28, 28}, axes {}    -> scalar
        * - Shape {28, 28}, axes {1}   -> {28}
        * - Shape {8, 28, 28}, axes {1, 2} -> {8}
        * - Shape {8, 28, 28}, axes {2} -> {8, 28}
        *
        * \tparam T Arithmetic element type used for input and output.
        */
        template <typename T>
        struct Sum final : Function {
            static_assert(
                std::is_arithmetic_v<T>,
                "ActivationFunctions::Sum requires a numeric type."
            );

        private:
            std::vector<size_t> axes_;

            /**
             * \brief Computes the output DataType and validates the reduction axes.
             */
            static Data::DataType makeOutputType(
                const Data::DataType& inputType,
                const std::vector<size_t>& axes)
            {
                if (!inputType.elementType ||
                    *inputType.elementType != typeid(T)) {
                    throw std::invalid_argument(
                        "ActivationFunctions::Sum: input element type must match T."
                    );
                }

                const size_t rank = inputType.rank;

                // An empty axes list means global reduction.
                if (axes.empty()) {
                    return Data::DataType::type<T>();
                }

                std::vector<bool> reduced(rank, false);

                for (size_t axis : axes) {
                    if (axis >= rank) {
                        throw std::invalid_argument("ActivationFunctions::Sum: axis " +std::to_string(axis) + " is outside the input rank " +std::to_string(rank) + ".");
                    }

                    if (reduced[axis]) {
                        throw std::invalid_argument(  "ActivationFunctions::Sum: duplicate reduction axis " + std::to_string(axis) + ".");
                    }

                    reduced[axis] = true;
                }

                auto dimensions = inputType.dimensions;
                size_t outputRank = 0;

                for (size_t axis = 0; axis < rank; ++axis) {
                    if (!reduced[axis]) {
                        dimensions[outputRank++] = dimensions[axis];
                    }
                }
                dimensions.resize(outputRank);
                return Data::DataType::type<T>(dimensions);
            }

        public:
            /**
             * \brief Constructs a sum reduction.
             *
             * \param input Input requirement.
             * \param axes Axes to reduce. Empty means global sum.
             */
            explicit Sum(
                const Requirement& input,
                std::vector<size_t> axes = {})
                : Function(
                    {input},
                    Requirement(makeOutputType(input.getDataType(), axes), NumericRange<double>::unbounded())
                ),
                axes_(std::move(axes)) { }

            /**
             * \brief Clones this reduction, preserving its axes and input requirement.
             */
            std::unique_ptr<Function> cloneUniquePtr() const override
            {
                return std::make_unique<Sum<T>>(
                    this->inputs.at(0), this->axes_
                );
            }

            /**
             * \brief Returns the function name.
             */
            std::string name() const override
            {
                return "Sum";
            }

            /**
             * \brief Executes the reduction and returns an owning DataValue.
             */
            Data::DataValue execute(
                const Data::DataValue& value) const override
            {
                const Data::DataType& inputType = value.getType();

                if (!inputType.elementType ||
                    *inputType.elementType != typeid(T)) {
                    throw std::invalid_argument(
                        "ActivationFunctions::Sum: input element type must match T."
                    );
                }

                const size_t rank = inputType.rank;
                const size_t count = inputType.totalElements();

                // Validate axes and derive the actual output shape.
                Data::DataType resultType = makeOutputType(inputType, axes_);

                const auto elements = value.getData<T>(); // Keeps the elements alive while they are read.
                const T* data = elements.get();

                // Global sum.
                if (axes_.empty()) {
                    T total{};

                    for (size_t i = 0; i < count; ++i) {
                        total += data[i];
                    }

                    return Data::DataValue::scalar<T>(total);
                }

                std::vector<bool> reduced(rank, false);

                for (size_t axis : axes_) {
                    reduced[axis] = true;
                }

                const size_t outputRank = resultType.rank;
                const size_t outputCount = resultType.totalElements();

                std::vector<T> result(outputCount, T{});

                // Map each input axis to its corresponding output axis.
                // Reduced axes map to -1.
                std::vector<size_t> outputAxisForInput(rank, static_cast<size_t>(-1));

                size_t nextOutputAxis = 0;

                for (size_t axis = 0; axis < rank; ++axis) {
                    if (!reduced[axis]) {
                        outputAxisForInput[axis] = nextOutputAxis++;
                    }
                }

                // Compute row-major output strides.
                std::vector<size_t> outputStrides(outputRank, 1);

                for (size_t axis = outputRank; axis > 1; --axis) {
                    outputStrides[axis - 2] = outputStrides[axis - 1] * resultType.dimensions[axis - 1];
                }

                // Accumulate each input element into its output element.
                for (size_t flat = 0; flat < count; ++flat) {
                    size_t remainder = flat;
                    size_t outputIndex = 0;

                    for (size_t axis = rank; axis > 0; --axis) {
                        const size_t a = axis - 1;
                        const size_t dimension = inputType.dimensions[a];

                        const size_t coordinate = remainder % dimension;
                        remainder /= dimension;

                        if (!reduced[a]) {
                            outputIndex += coordinate * outputStrides[outputAxisForInput[a]];
                        }
                    }

                    result[outputIndex] += data[flat];
                }

                return Data::DataValue::fromRange(result, resultType);
            }
        };
    };
}

#endif