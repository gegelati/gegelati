#include "tpg/tpgFactory.h"

TPG::TPGTeam* TPG::TPGFactory::createTPGTeam() const
{
    return new TPG::TPGTeam();
}

TPG::TPGAction* TPG::TPGFactory::createTPGAction(
    const uint64_t id) const
{
    return new TPG::TPGAction(id);
}

std::unique_ptr<TPG::TPGEdge> TPG::TPGFactory::createTPGEdge(
    const TPGVertex* src, const TPGVertex* dest,
    const std::shared_ptr<Program::Program> prog) const
{
    return std::make_unique<TPG::TPGEdge>(src, dest, prog);
}
