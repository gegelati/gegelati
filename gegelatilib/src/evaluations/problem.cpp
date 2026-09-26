
#include "evaluations/problem.h"

const std::vector<Dimensions::Requirement>& Evaluations::Problem::getInputDimensions() const
{
    return this->inputDimensions;
}

const Dimensions::Requirement& Evaluations::Problem::getOutputDimension() const
{
    return this->outputDimension;
}

std::string Evaluations::Problem::summary() const
{
    std::ostringstream result;            
    result << "Inputs (" << this->inputDimensions.size() << "):\n";
    for (const auto& input : this->inputDimensions) {
        result << "  * " << input.summary() << "\n";
    }
    result << "\nOutput: " << this->outputDimension << "\n";
    return result.str();
}

uint64_t Evaluations::Problem::getProblemSeed() const 
{
    return this->problemSeed;
}

uint64_t Evaluations::Problem::maxHash() const
{
    return UINT64_MAX;
}