
#include <cmath>
#include <algorithm>
#include "util/activationFunctions.h"


double Utils::ActivationFunctions::sigmoid(double value)
{
    return 1.0 / (1.0 + std::exp(-value));
}

double Utils::ActivationFunctions::tanh(double value)
{
    return std::tanh(value);
}

std::vector<double> Utils::ActivationFunctions::softmax(const std::vector<double>& values)
{
    std::vector<double> exps;
    double sum = 0.0;
    for (double val : values) {
        double e = std::exp(val);
        exps.push_back(e);
        sum += e;
    }
    for (double& e : exps) {
        e /= sum;
    }
    return exps;
}

std::vector<double> Utils::ActivationFunctions::scaleOutputValues(const std::vector<double>& values, const Output::OutputHandler& outputs, Utils::ActivationFunction function)
{
    std::vector<double> scaledValues;
    std::vector<std::reference_wrapper<const Output::Output>> continuousOutputs(outputs.getContinuousOutputs());

    if(values.size() != continuousOutputs.size()){
        throw std::runtime_error("Utils::ActivationFunctions::scaleOutputValues Number of output values does not match the number of continuous outputs.");
    }

    // Softmax is vector-wide, not element-wise
    if (function == ActivationFunction::SOFTMAX) {
        return softmax(values);
    }

    for(size_t idx = 0; idx < values.size(); idx++){

        const Output::Output& output = continuousOutputs.at(idx);
        double rangeMin = output.getRangeMin();
        double rangeMax = output.getRangeMax();

        // If rangeMin is -inf or rangeMax is +inf, we cannot scale
        if(rangeMin == -std::numeric_limits<double>::infinity() || rangeMax == std::numeric_limits<double>::infinity()){
            scaledValues.push_back(values[idx]);
            continue;
        }

        if(function == ActivationFunction::SIGMOID){
            double sig = sigmoid(values[idx]);
            // Scale sigmoid output [0, 1] to [rangeMin, rangeMax]
            scaledValues.push_back(sig * (rangeMax - rangeMin) + rangeMin);
            
        } else if (function == ActivationFunction::TANH) {
            double t = tanh(values[idx]);
            // Optimize for symmetric ranges (most common case)
            if (rangeMin == -rangeMax) {
                scaledValues.push_back(t * rangeMax);
            } else {
                // General scaling for asymmetric ranges
                double scaled = ((t + 1.0) / 2.0) * (rangeMax - rangeMin) + rangeMin;
                scaledValues.push_back(scaled);
            }
        } else if (function == ActivationFunction::CLAMP) {
            scaledValues.push_back(std::clamp(values[idx], rangeMin, rangeMax));
        }
        
    }
    return scaledValues;
}