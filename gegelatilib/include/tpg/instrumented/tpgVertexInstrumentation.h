#ifndef TPG_VERTEX_INSTRUMENTATION_H
#define TPG_VERTEX_INSTRUMENTATION_H

#include <atomic>
#include <cstddef>

namespace TPG {
    /**
     * \brief Instrumentation code for TPGVertex class for instrumented
     * execution.
     */
    class TPGVertexInstrumentation
    {
      public:
        /**
         * \brief Get the number of time a TPGVertexInstrumentation was visited.
         */
        uint64_t getNbVisits() const;

        /**
         * \brief Add one to the number of visits for this
         * TPGVertexInstrumented.
         */
        void incrementNbVisits() const;

        /**
         *  \brief Reset the instrumentation attributes.
         */
        void reset() const;

      protected:
        /**
         * \brief Protected default constructor to forbid instanciation.
         *
         * This constructor initializes the instrumentation attributes.
         */
        TPGVertexInstrumentation() : nbVisits{0}
        {
        }

        /// Number of a time a TPGVertex has been visited
        /// Attribute is mutable because all TPGVertex are seen as const outside
        /// from their TPGGraph.
        mutable std::atomic_uint64_t nbVisits;
    };
} // namespace TPG

#endif // !TPG_VERTEX_INSTRUMENTATION_H
