#include "outputInfo.h"

void Output::OutputHandler::addOutput(const Output output)
{
    outputs.push_back(output);
}

const std::vector<Output::Output>& Output::OutputHandler::getOutputs() const
{
    return outputs;
}

std::vector<std::reference_wrapper<const Output::Output>> Output::OutputHandler::getDiscreteOutputs() const
{
    std::vector<std::reference_wrapper<const Output>> discrete;
    for (const Output& out : outputs) {
        if (out.getType() == OutputType::DISCRETE) {
            discrete.push_back(std::cref(out));
        }
    }
    return discrete;
}

std::vector<std::reference_wrapper<const Output::Output>> Output::OutputHandler::getContinuousOutputs() const
{
    std::vector<std::reference_wrapper<const Output>> continuous;
    for (const Output& out : outputs) {
        if (out.getType() == OutputType::CONTINUOUS) {
            continuous.push_back(std::cref(out));
        }
    }
    return continuous;
}

size_t Output::OutputHandler::sizeDiscrete() const
{
    size_t count = 0;
    for (const auto& out : outputs) {
        if (out.getType() == OutputType::DISCRETE) {
            ++count;
        }
    }
    return count;
}

size_t Output::OutputHandler::sizeContinuous() const
{
    size_t count = 0;
    for (const auto& out : outputs) {
        if (out.getType() == OutputType::CONTINUOUS) {
            ++count;
        }
    }
    return count;
}

const Output::Output& Output::OutputHandler::front() const {
    return this->outputs.front();
}

void Output::convertContinuousToDiscreteOutputs(std::vector<double>& continuousValues, const OutputHandler& outputs)
{
    std::vector<std::reference_wrapper<const Output>> discreteOutputs = outputs.getDiscreteOutputs();
    if(continuousValues.size() != discreteOutputs.size()){
        throw std::runtime_error("Output::convertContinuousToDiscreteOutputs: Number of continuous values does not match number of discrete outputs.");
    }

    for(size_t idx = 0; idx < continuousValues.size(); idx++){
        // clamp the value between 0 and nbValues -1, then round it to get the discrete value
        const Output& output = discreteOutputs.at(idx);
        double value = continuousValues.at(idx);
        if(std::isnan(value) || value < 0){
            value = 0;
        } else if (value > (double)(output.getNbValues() - 1)){
            value = (double)(output.getNbValues() - 1);
        }
        continuousValues.at(idx) = static_cast<size_t>(std::round(value));
    }
}