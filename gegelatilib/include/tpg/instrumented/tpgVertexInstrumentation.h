
#ifndef TPG_VERTEX_INSTRUMENTATION_H
#define TPG_VERTEX_INSTRUMENTATION_H

namespace TPG {
    /**
     * \brief Instrumentation code for TPGVertex class for instrumented execution.
     */
    class TPGVertexInstrumentation 
    {
        /**
         * \brief Get the number of time a TPGVertexInstrumentation was visited.
         */
        size_t getNbVisits() const;

        /**
         * \brief Add one to the number of visits for this
         * TPGVertexInstrumented.
         */
        void incrementNbVisits();

        /**
         *  \brief Reset the instrumentation attributes.
         */
        void reset();

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
        size_t nbVisits;
    };
} // namespace TPG

#endif // !TPG_VERTEX_INSTRUMENTATION_H
