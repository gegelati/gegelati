#ifndef EXECUTION_STATS_H
#define EXECUTION_STATS_H

#include <map>

#include "tpg/tpgGraph.h"
#include "tpg/instrumented/tpgExecutionEngineInstrumented.h"

namespace TPG{

    /**
     * Utility class for extracting execution statistics
     * from a TPGExecutionEngineInstrumented.
     *
     * TODO how to use the class
     */
    class ExecutionStats{

      private:

        /* Average results */

        /// Average number of visited teams per inference.
        double avgVisitedTeams;

        /// Average number of programs evaluated per inference.
        double avgEvaluatedPrograms;

        /// Average number of executed lines per inference.
        double avgExecutedLines;

        /**
         * This map associate an Instruction identifier from an
         * instruction set with the average number of execution
         * of the instruction per inference.
         */
        std::map<size_t, double> avgNbExecutionPerInstruction;

      public:

        /// Default constructor.
        ExecutionStats() = default;

        /// Default destructor.
        virtual ~ExecutionStats() = default;

        /// Analyze the average results of a TPGGraph execution.
        void analyzeExecution(const TPGGraph* graph);

        /// Analyze the execution of one inference trace.
        void analyzeInferenceTrace(const std::vector<const TPGVertex*> trace);


    };

}


#endif // EXECUTION_STATS_H
