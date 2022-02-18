#include "tpg/instrumented/tpgInstrumentedFactory.h"
#include "tpg/instrumented/tpgActionInstrumented.h"
#include "tpg/instrumented/tpgEdgeInstrumented.h"
#include "tpg/instrumented/tpgExecutionEngineInstrumented.h"
#include "tpg/instrumented/tpgTeamInstrumented.h"

std::shared_ptr<TPG::TPGGraph> TPG::TPGInstrumentedFactory::createTPGGraph(
    const Environment& env) const
{
    return std::make_shared<TPG::TPGGraph>(
        env, std::make_unique<TPGInstrumentedFactory>());
}

TPG::TPGTeam* TPG::TPGInstrumentedFactory::createTPGTeam() const
{
    return new TPGTeamInstrumented();
}

TPG::TPGAction* TPG::TPGInstrumentedFactory::createTPGAction(
    const uint64_t id) const
{
    return new TPGActionInstrumented(id);
}

std::unique_ptr<TPG::TPGEdge> TPG::TPGInstrumentedFactory::createTPGEdge(
    const TPGVertex* src, const TPGVertex* dest,
    const std::shared_ptr<Program::Program> prog) const
{
    auto ptr = std::make_unique<TPG::TPGEdgeInstrumented>(src, dest, prog);
    return ptr;
}

std::unique_ptr<TPG::TPGExecutionEngine> TPG::TPGInstrumentedFactory::
    createTPGExecutionEngine(const Environment& env, Archive* arch) const
{
    return std::make_unique<TPGExecutionEngineInstrumented>(env, arch);
}
