
#include "evoGraph/element.h"


std::shared_ptr<const Algorithm::Agent> EvoGraph::Element::getProgram() const
{
    return this->program;
}

void EvoGraph::Element::setProgram(
    std::shared_ptr<const Algorithm::Agent> program)
{
    this->program = program;
}

bool EvoGraph::Element::hasProgram()
{
    return this->program != nullptr;
}
