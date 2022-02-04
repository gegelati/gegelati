

#include "tpg/tpgGraph.h"

#ifndef TPG_GRAPH_INSTRUMENTED_H
#define TPG_GRAPH_INSTRUMENTED_H

namespace TPG {

    /**
    * \brief Extension of the TPGGraph class integrating instrumentation 
    * attributes (eg. counters) to keep track of a TPGGraph execution.
    * 
    */
    class TPGGraphInstrumented : public TPG::TPGGraph
    {
      public:
        /**
         * \brief Main TPGGraphInstrumented constructor.
         *
         * \param[in] e the Environment for the TPGGraph.
         */
        TPGGraphInstrumented(const Environment& e) : TPG::TPGGraph(e) {};

        /**
         * \brief delete copy constructor
         */
        TPGGraphInstrumented(const TPGGraphInstrumented& model) = delete;

        /**
         * \brief TPGGraph move assignment operator
         *
         * \param[in] model the TPGGraph to copy
         */
        TPGGraphInstrumented(TPGGraphInstrumented&& model) noexcept
            : TPGGraph(model.getEnvironment())
        {
            swap(*this, model);
        }

        /**
         *	\brief Helper function for move constructor.
         *
         *	Swaps the TPGGraphInstrumented objects.
         */
        friend inline void swap(TPGGraphInstrumented& a, TPGGraphInstrumented& b)
        {
            using std::swap;
            swap(a.vertices, b.vertices);
            swap(a.edges, b.edges);
        }

        /**
         *	\brief assignement operator for class TPGGraphInstrumented
         */
        TPGGraphInstrumented& operator=(TPGGraphInstrumented model);
    };
} // namespace TPG

#endif