#ifndef TPG_KEYED_FACTORY
#define TPG_KEYED_FACTORY

#include "tpg/tpgFactory.h"
#include "tpg/tpgGraph.h"

namespace TPG {

    /// Specialization of the TPGFactory class producing TPGTeamKeyed,
    /// TPGEdgeKeyed, and TPGExecutionEngineKeyed
    class TPGKeyedFactory : public TPGFactory
    {
      public:
        /// Specialization of the method returing the TPGGraphKeyed with a
        /// TPGKeyedFactory as an attribute.
        virtual std::shared_ptr<TPGGraph> createTPGGraph(
            const Environment& env) const override;

        /// Specialization of the method returning a TPGTeamInstrumented
        virtual std::unique_ptr<TPGTeam> createTPGTeam() const override;

        /// Specialization of the method returning a TPGEdgeInstrumented
        virtual std::unique_ptr<TPGEdge> createTPGEdge(
            const TPGVertex* src, const TPGVertex* dest,
            const std::shared_ptr<Program::Program> prog) const override;

        ///  Specialization of the method returning a
        ///  TPGExecutionEngineKeyed
        virtual std::unique_ptr<TPGExecutionEngine> createTPGExecutionEngine(
            const Environment& env, Archive* arch = NULL) const override;
    };
} // namespace TPG

#endif // !TPG_KEYED_FACTORY
