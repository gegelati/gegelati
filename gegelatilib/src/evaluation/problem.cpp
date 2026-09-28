
#include "evaluation/problem.h"

const std::vector<Dimensions::Requirement>& Evaluation::Problem::getInputDimensions() const
{
    return this->inputDimensions;
}

const Dimensions::Requirement& Evaluation::Problem::getOutputDimension() const
{
    return this->outputDimension;
}

std::string Evaluation::Problem::summary() const
{
    std::ostringstream result;            
    result << "Inputs (" << this->inputDimensions.size() << "):\n";
    for (const auto& input : this->inputDimensions) {
        result << "  * " << input.summary() << "\n";
    }
    result << "\nOutput: " << this->outputDimension << "\n";
    return result.str();
}

uint64_t Evaluation::Problem::getProblemSeed() const 
{
    return this->problemSeed;
}

uint64_t Evaluation::Problem::maxHash() const
{
    return UINT64_MAX;
}