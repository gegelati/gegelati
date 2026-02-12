
#include "evoGraph/element.h"


std::weak_ptr<const Algorithm::Agent> EvoGraph::Element::getProgram() const
{
    return this->program;
}

void EvoGraph::Element::setProgram(
    std::weak_ptr<const Algorithm::Agent> program)
{
    this->program = program;
}

bool EvoGraph::Element::hasProgram()
{
    return !this->program.expired();
}
