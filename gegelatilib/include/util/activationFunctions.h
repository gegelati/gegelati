
#ifndef ACTIVATION_FUNCTIONS_H
#define ACTIVATION_FUNCTIONS_H

#include "outputInfo.h"

namespace Utils
{

    /// \brief Enumeration of supported activation functions.
    enum class ActivationFunction
    {
        SIGMOID,
        TANH,
        CLAMP,
        SOFTMAX
    };
    
    /// \brief Namespace containing activation functions used for scaling outputs.
    namespace ActivationFunctions {

        /**
         * \brief Sigmoid activation function.
         * 
         * \param value Input value.
         * \return Sigmoid of the input value.
         */
        double sigmoid(double value);

        /**
         * \brief Hyperbolic tangent activation function.
         * 
         * \param value Input value.
         * \return Hyperbolic tangent of the input value.
         */
        double tanh(double value);

        /**
         * \brief Softmax activation function.
         * 
         * \param values Vector of input values.
         * \return Vector of softmax values.
         */
        std::vector<double> softmax(const std::vector<double>& values);

        /**
         * \brief Scales output values based on the specified activation function and output ranges.
         * 
         * \param values Vector of input values.
         * \param outputs Output handler containing output definitions.
         * \param function Activation function to use for scaling.
         * \return Vector of scaled output values.
         */
        std::vector<double> scaleOutputValues(const std::vector<double>& values, const Output::OutputHandler& outputs, Utils::ActivationFunction function);
    }
};


#endif