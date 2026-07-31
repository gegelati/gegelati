
#include "evoGraph/element.h"


const Representation::Individual& EvoGraph::Element::getProgram() const
{
    return *this->program;
}

void EvoGraph::Element::setProgram(
    const Representation::Individual& program)
{
    this->program = program;
}

void EvoGraph::Element::removeProgram()
{
    this->program = std::nullopt;
}

bool EvoGraph::Element::hasProgram() const
{
    return this->program != std::nullopt;
}
