
#include "evoGraph/element.h"


const Algorithm::Agent& EvoGraph::Element::getProgram() const
{
    return *this->program;
}

void EvoGraph::Element::setProgram(
    const Algorithm::Agent& program)
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
