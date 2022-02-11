#include "tpg/tpgGraphElementFactory.h"

TPG::TPGTeam* TPG::TPGGraphElementFactory::createTPGTeam() const
{
    return new TPG::TPGTeam();
}

TPG::TPGAction* TPG::TPGGraphElementFactory::createTPGAction(
    const uint64_t id) const
{
    return new TPG::TPGAction(id);
}

std::unique_ptr<TPG::TPGEdge> TPG::TPGGraphElementFactory::createTPGEdge(
    const TPGVertex* src, const TPGVertex* dest,
    const std::shared_ptr<Program::Program> prog) const
{
    return std::make_unique<TPG::TPGEdge>(src, dest, prog);
}
