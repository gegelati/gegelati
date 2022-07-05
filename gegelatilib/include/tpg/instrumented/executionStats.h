#ifndef EXECUTION_STATS_H
#define EXECUTION_STATS_H

#include <map>

#include "tpg/tpgGraph.h"
#include "tpg/instrumented/tpgExecutionEngineInstrumented.h"

namespace TPG{

    /**
     * \brief Struct used to store execution statistics about one inference trace.
     *
     * It contains for one trace :
     * - the inference trace in a std::vector<const TPG::TPGVertex*>
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
     * The main method of this class is analyzeExecution(), which will :
     *  - retrieve from a TPGGraph the instrumented values and compute
     *  average execution statistics.
     *  - compute execution statistics for every inference done with a TPGExecutionEngineInstrumented.
     *
     * Before analyzing or even starting any inference, you must :
     *  - use a TPGGraph associated to a TPGInstrumentedFactory.
     *  - use a TPGExecutionEngineInstrumented that will execute the TPGGraph.
     *  - clear any previous instrumented data :
     *      --> for the TPGGraph, use TPGInstrumentedFactory::resetTPGGraphCounters().
     *      --> for the TPGExecutionEngineInstrumented, use its method clearTraceHistory().
     * Otherwise, the results won't have any meaning.
     * If you have never execute the TPGGraph or the TPGExecutionEngineInstrumented,
     * resetting them isn't required.
     *
     * You can then execute the TPG for as many inferences as you like.
     *
     * Then, use analyzeExecution() with the related TPGGraph and
     * TPGExecutionEngineInstrumented, and use the getters and setters to
     * read the statistics.
     *
     * Warning : the class deduce the number of inferences based on the sum
     * of evaluation each root vertices had. If you executed your TPGGraph
     * starting from multiple roots, then remember that the average statistics
     * are based on ALL inference, regardless of the root vertices used.
     *
     * // TODO explanation on JSon exporter
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
        std::vector<TraceStats> inferenceTracesStats;


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
         * \brief Analyze the average statistics of an instrumented TPGGraph execution.
         *
         * Results are stored in the average results class attributes.
         *
         * \param graph the analyzed TPGGraph*.
         * \throws std::bad_cast if graph contains at least one non instrumented vertex or edge.
         */
        void analyzeInstrumentedGraph(const TPGGraph* graph);

        /**
         * \brief Analyze the execution statistics of one inference trace.
         *
         * The vector trace contains all visited vertices for one inference
         * in order : trace[0] is the root, and trace[trace.size()-1] the action.
         *
         * Results are stored in a new TraceStats struct which is pushed back
         * in inferenceTracesStats of the object.
         *
         * \param trace a const vector of the analyzed inference trace.
         */
        void analyzeInferenceTrace(const std::vector<const TPGVertex*>& trace);

        /**
         * \brief Analyze the execution statistics of multiple inferences
         * done with a TPGExecutionEngineInstrumented.
         *
         * Previous results will be erased.
         *
         * \param tee the TPGExecutionEngineInstrumented.
         * \param graph the TPGGraph executed with tee.
         * \throws std::bad_cast if the graph contains a non instrumented vertex or edge.
         */
        void analyzeExecution(const TPG::TPGExecutionEngineInstrumented& tee, const TPGGraph* graph);


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
        const std::vector<TraceStats>& getInferenceTracesStats() const;

        /// Clear results of previously analyzed inference trace.
        void clearInferenceTracesStats();

    };

}


#endif // EXECUTION_STATS_H
