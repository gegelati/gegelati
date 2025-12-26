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