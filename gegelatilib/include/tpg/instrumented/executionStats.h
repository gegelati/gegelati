#ifndef EXECUTION_STATS_H
#define EXECUTION_STATS_H

#include <map>

#include "tpg/tpgGraph.h"
#include "tpg/instrumented/tpgExecutionEngineInstrumented.h"

namespace TPG{

    /**
     * \brief Struct used to store execution statistics about one inference execution trace.
     *
     * It contains for one trace :
     * - the execution trace in a std::vector<const TPG::TPGVertex*>
     * - the number of evaluated teams
     * - the number of evaluated programs
     * - the number of executed lines
     * - the number of execution for each instructions (indexed by instruction index)
     */
    struct TraceStats{
        const std::vector<const TPG::TPGVertex*> trace;

        const uint64_t nbEvaluatedTeams;
        const uint64_t nbEvaluatedPrograms;
        const uint64_t nbExecutedLines;
        const std::map<uint64_t, uint64_t> nbExecutionPerInstruction;
    };

    /**
     * \brief Utility class for extracting execution statistics
     * from a TPGExecutionEngineInstrumented.
     *
     * TODO how to use the class
     */
    class ExecutionStats{

      private:

        /* Average results */

        /// Average number of evaluated teams per inference.
        double avgEvaluatedTeams = 0.0;

        /// Average number of programs evaluated per inference.
        double avgEvaluatedPrograms = 0.0;

        /// Average number of executed lines per inference.
        double avgExecutedLines = 0.0;

        /**
         * This map associate an Instruction identifier from an
         * instruction set with the average number of execution
         * of the instruction per inference.
         */
        std::map<size_t, double> avgNbExecutionPerInstruction;

        /// Statistics of last analyzed traces.
        std::vector<TraceStats> executionTracesStats;


      protected:

        /**
         * \brief Analyze a program to get the number of each instructions used.
         *
         * \param[out] instructionCounts the std::map<uint64_t, uint64_t>& that
         * will be incremented for each instruction used by the program.
         * \param[in] program the analyzed program.
         */
        static void analyzeProgram(std::map<uint64_t, uint64_t>& instructionCounts,
                             const Program::Program& program);

      public:

        /// Default constructor.
        ExecutionStats() = default;

        /// Default destructor.
        virtual ~ExecutionStats() = default;

        /**
         * \brief Analyze the average results of a TPGGraph execution.
         *
         * Results are stored in the average results class attributes.
         *
         * \param graph the analyzed TPGGraph*.
         * \throws std::bad_cast if the TPG contains a non instrumented vertex or edge.
         */
        void analyzeExecution(const TPGGraph* graph);

        /// Analyze the execution of one inference trace.
        void analyzeInferenceTrace(const std::vector<const TPGVertex*> trace);


        /// Get the average number of evaluated teams per inference.
        double getAvgEvaluatedTeams() const;

        /// Get the average number of programs evaluated per inference.
        double getAvgEvaluatedPrograms() const;

        /// Get the average number of executed lines per inference.
        double getAvgExecutedLines() const;

        /// Get a reference to the map that associate each instruction to
        /// its average number of execution per inference.
        const std::map<size_t, double>& getAvgNbExecutionPerInstruction() const;

        /// Get a vector of the trace statistics of last analyzed traces.
        const std::vector<TraceStats>& getExecutionTracesStats() const;

    };

}


#endif // EXECUTION_STATS_H
