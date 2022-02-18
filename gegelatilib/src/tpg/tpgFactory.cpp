#include "tpg/tpgFactory.h"
#include "tpg/tpgGraph.h"
#include "tpg/tpgExecutionEngine.h"

std::shared_ptr<TPG::TPGGraph> TPG::TPGFactory::createTPGGraph(
    const Environment& env) const
{
    return std::make_shared<TPG::TPGGraph>(env, std::make_unique<TPGFactory>());
}

TPG::TPGTeam* TPG::TPGFactory::createTPGTeam() const
{
    return new TPG::TPGTeam();
}

TPG::TPGAction* TPG::TPGFactory::createTPGAction(const uint64_t id) const
{
    return new TPG::TPGAction(id);
}

std::unique_ptr<TPG::TPGEdge> TPG::TPGFactory::createTPGEdge(
    const TPGVertex* src, const TPGVertex* dest,
    const std::shared_ptr<Program::Program> prog) const
{
    return std::make_unique<TPG::TPGEdge>(src, dest, prog);
}

std::unique_ptr<TPG::TPGExecutionEngine> TPG::TPGFactory::createTPGExecutionEngine(
    const Environment& env, Archive* arch) const
{
    return std::make_unique<TPG::TPGExecutionEngine>(env, arch);
}
