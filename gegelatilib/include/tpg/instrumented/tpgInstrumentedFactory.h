#ifndef TPG_INSTRUMENTED_FACTORY
#define TPG_INSTRUMENTED_FACTORY

#include "tpg/tpgFactory.h"
#include "tpg/tpgGraph.h"

namespace TPG {

    /// Specialization of the TPGFactory class producing TPGTeamInstrumented,
    /// TPGActionInstrumented, TPGEdgeInstrumented, and
    /// TPGExecutionEngineInstrumented
    ///
    /// Also contains a helper function for resetting the instrumentation of a
    /// TPGGraph.
    class TPGInstrumentedFactory : public TPGFactory
    {
      public:
        /// Specialization of the method returing the TPGGraph with a
        /// TPGInstrumentedFactory as an attribute.
        virtual std::shared_ptr<TPGGraph> createTPGGraph(
            const Environment& env) const override;

        ///  Specialization of the method returning a TPGTeamInstrumented
        virtual TPGTeam* createTPGTeam() const override;

        ///  Specialization of the method returning a TPGActionInstrumented
        virtual TPGAction* createTPGAction(const uint64_t id) const override;

        ///  Specialization of the method returning a TPGEdgeInstrumented
        virtual std::unique_ptr<TPGEdge> createTPGEdge(
            const TPGVertex* src, const TPGVertex* dest,
            const std::shared_ptr<Program::Program> prog) const override;

        ///  Specialization of the method returning a
        ///  TPGExecutionEngineInstrumented
        virtual std::unique_ptr<TPGExecutionEngine> createTPGExecutionEngine(
            const Environment& env, Archive* arch = NULL) const override;
        /**
         * \brief Reset all visit and traversal counters of a TPGGraph.
         *
         * \param[in] tpg the TPGGraph whose TPGTeamInstrumented,
         * TPGActionInstrumented, and TPGEdgeInstrumented will be reseted.The
         * TPGGraph is const since all instrumentation counters are mutable.
         */
        void resetTPGGraphCounters(const TPG::TPGGraph& tpg) const;
    };
} // namespace TPG

#endif // !TPG_INSTRUMENTED_FACTORY
