#ifndef TPG_EDGE_INSTRUMENTED_H
#define TPG_EDGE_INSTRUMENTED_H

#include "tpg/tpgEdge.h"

namespace TPG {

    /**
     * \brief Instrumented TPGEdge class to keep track of a TPG execution
     * statistics.
     */
    class TPGEdgeInstrumented : public TPGEdge
    {
      public:
        /// Default constructor
        TPGEdgeInstrumented(const TPGVertex* src, const TPGVertex* dest,
                            const std::shared_ptr<Program::Program> prog)
            : TPGEdge(src, dest, prog), nbVisits{0}, nbTraversed{0}
        {
        }

        /**
         * \brief Get the number of time a TPGEdge was visited.
         *
         * That is the number of time it caused an execution of its program.
         */
        uint64_t getNbVisits() const;

        /**
         * \brief Add one to the number of visits for this
         * TPGEdge
         */
        void incrementNbVisits() const;

        /**
         * \brief Get the number of time a TPGEdge was traversed.
         *
         * hat is the number of time its program produced the winning bid.
         */
        uint64_t getNbTraversed() const;

        /**
         * \brief Add one to the number of visits for this
         * TPGEdge
         */
        void incrementNbTraversed() const;

        /**
         *  \brief Reset the instrumentation attributes.
         */
        void reset() const;

      protected:
        /// Number of a time a TPGEdge has been visited
        /// That is the number of time it caused an execution of its program.
        /// Attribute is mutable because all TPGEdge are seen as const outside
        /// from their TPGGraph.
        mutable std::atomic_uint64_t nbVisits;

        /// Number of a time a TPGEdge has been traversed
        /// That is the number of time its program produced the winning bid.
        /// Attribute is mutable because all TPGEdge are seen as const outside
        /// from their TPGGraph.
        mutable std::atomic_uint64_t nbTraversed;
    };
} // namespace TPG

#endif // !TPG_EDGE_INSTRUMENTED_H