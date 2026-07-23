#include "tpg/keyed/tpgKeyedFactory.h"
#include "tpg/keyed/tpgEdgeKeyed.h"
#include "tpg/keyed/tpgTeamKeyed.h"
#include <tpg/keyed/tpgExecutionEngineKeyed.h>

std::shared_ptr<TPG::TPGGraph> TPG::TPGKeyedFactory::createTPGGraph(
    const Environment& env) const
{
    return std::make_shared<TPG::TPGGraph>(env,
                                           std::make_unique<TPGKeyedFactory>());
}

std::unique_ptr<TPG::TPGTeam> TPG::TPGKeyedFactory::createTPGTeam() const
{
    return std::make_unique<TPG::TPGTeamKeyed>();
}

std::unique_ptr<TPG::TPGEdge> TPG::TPGKeyedFactory::createTPGEdge(
    const TPGVertex* src, const TPGVertex* dest,
    const std::shared_ptr<Program::Program> prog) const
{
    auto ptr = std::make_unique<TPG::TPGEdgeKeyed>(src, dest, prog);
    return ptr;
}

std::unique_ptr<TPG::TPGExecutionEngine> TPG::TPGKeyedFactory::
    createTPGExecutionEngine(const Environment& env, Archive* arch) const
{
    return std::make_unique<TPGExecutionEngineKeyed>(env, arch);
}
