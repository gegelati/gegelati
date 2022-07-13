#ifndef EXECUTION_STATS_H
#define EXECUTION_STATS_H

#include <map>

#include "tpg/tpgGraph.h"
#include "tpg/instrumented/tpgExecutionEngineInstrumented.h"

namespace TPG{

    /**
     * \brief Struct used to store execution statistics about one inference trace.
     *
     * It contains :
     * - the inference trace in a std::vector<const TPG::TPGVertex*>
     * - the number of evaluated teams
     * - the number of evaluated programs
     * - the number of executed lines
     * - the number of execution for each instructions (indexed by instruction index)
     */
    struct TraceStats{
        /// The inference trace.
        const std::vector<const TPG::TPGVertex*> trace;

        /// Number of team evaluated.
        const uint64_t nbEvaluatedTeams;
        /// Number of programs evaluated.
        const uint64_t nbEvaluatedPrograms;
        /// Number of program lines executed.
        const uint64_t nbExecutedLines;
        /// Map that associate the instruction indexes with the number of
        /// execution of the corresponding Instruction.
        const std::map<uint64_t, uint64_t> nbExecutionPerInstruction;
    };

    /**
     * \brief Utility class for extracting execution statistics
     * from a TPGExecutionEngineInstrumented and an instrumented TPGGraph.
     *
     * The main method of this class is analyzeExecution(), which will :
     *  - retrieve from a TPGGraph the instrumented values and compute
     *  average execution statistics.
     *  - compute execution statistics for every inference done with a TPGExecutionEngineInstrumented.
     *  - create distributions from the statistics of the traces.
     *
     * Before analyzing or even starting any inference, you must :
     *  - use a TPGGraph associated to a TPGInstrumentedFactory.
     *  - use a TPGExecutionEngineInstrumented that will execute the TPGGraph.
     *  - clear any previous instrumented data :
     *      --> for the TPGGraph, use TPGInstrumentedFactory::resetTPGGraphCounters().
     *      --> for the TPGExecutionEngineInstrumented, use its method clearTraceHistory().
     * Otherwise, the results won't have any meaning.
     * If you have never executed the TPGGraph or the TPGExecutionEngineInstrumented,
     * resetting them isn't required.
     *
     * You can then execute the TPG for as many inferences as you like.
     *
     * Then, use analyzeExecution() with the related TPGGraph and
     * TPGExecutionEngineInstrumented, and access the statistics using the
     * provided getters and setters.
     *
     * Warning : the class deduces the number of inferences based on the sum
     * of evaluation each root vertices had. If you executed your TPGGraph
     * starting from multiple roots, then remember that the average statistics
     * are based on ALL inferences, regardless of the root vertices used.
     *
     * The Json exporter is designed to be used after a call to analyzeExecution().
     * Just call writeStatsToJson() to export statistics in a file with json format.
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


        /* Analysed Traces */

        /// Statistics of last analyzed traces.
        std::vector<TraceStats> inferenceTracesStats;


        /* Distributions */

        /**
         * Distribution of the number of evaluated team per inference for all analysed traces.
         *
         * distribEvaluatedTeams[x] = y --> y inferences evaluated x teams.
         */
        std::map<size_t, size_t> distribEvaluatedTeams;

        /**
         * Distribution of the number of evaluated programs per inference for all analysed traces.
         *
         * distribEvaluatedPrograms[x] = y --> y inferences evaluated x programs.
         */
        std::map<size_t, size_t> distribEvaluatedPrograms;

        /**
         * Distribution of the number of executed lines per inference for all analysed traces.
         *
         * distribExecutedLines[x] = y --> y inferences executed x lines.
         */
        std::map<size_t, size_t> distribExecutedLines;

        /**
         * Distributions of the number of executions of each instruction per
         * inference for all analysed traces.
         *
         * distribNbExecutionPerInstruction[i][x] = y --> for instruction at index i,
         * y inferences executed this instruction x times.
         */
        std::map<size_t, std::map<size_t, size_t>> distribNbExecutionPerInstruction;

        /**
         * Distribution of the number of visit each vertex had for all analysed traces.
         *
         * distribUsedVertices[v] = y --> y inferences visited vertex pointed by v.
         */
        std::map<const TPG::TPGVertex*, size_t> distribUsedVertices;



        /// Graph used during last call to analyzeExecution
        const TPGGraph* lastAnalyzedGraph = nullptr;


      protected:

        /**
         * \brief Analyze a program to get how many times each instruction is used.
         *
         * \param[out] instructionCounts the std::map<uint64_t, uint64_t>& that
         * will be incremented for each instruction use.
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
         * \param[in] graph the analyzed TPGGraph*.
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
         * in attribute inferenceTracesStats. Previous results will be erased.
         *
         * \param[in] trace a vector<const TPGVertex*> of the analyzed inference trace.
         */
        void analyzeInferenceTrace(const std::vector<const TPGVertex*>& trace);

        /**
         * \brief Analyze the execution statistics of multiple inferences
         * done with a TPGExecutionEngineInstrumented.
         *
         * Previous results will be erased.
         *
         * \param[in] tee the TPGExecutionEngineInstrumented.
         * \param[in] graph the TPGGraph executed with tee.
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

        /// Get stored trace statistics.
        const std::vector<TraceStats>& getInferenceTracesStats() const;

        /// Clear stored trace statistics and distributions.
        void clearInferenceTracesStats();


        /**
         * \brief Export the execution statistics of the last analyzeExecution()
         * call to a file using Json format.
         *
         * This method will use the statistics currently stored in the object
         * because it is intended to be used after a call to analyzeExecution().
         * Using it after separate calls to analyzeInstrumentedGraph() or
         * analyzeInferenceTrace() might lead to uncorrelated data.
         *
         * Data is organised as follows :
         * { "AverageStats" :
         *  {
         *      "avgEvaluatedTeams" : value,
         *      "avgEvaluatedPrograms" : value,
         *      "avgExecutedLines" : value,
         *      "avgNbExecutionPerInstruction"
         *      {
         *          "InstructionIndex" : nbExecution,
         *          ...
         *      }
         *  },
         *
         *  "TracesStats" :
         *  {
         *      "TraceNumber" :
         *      {
         *          "nbEvaluatedPrograms" : value,
         *          "nbEvaluatedTeams" : value,
         *          "nbExecutedLines" : value,
         *          "nbExecutionPerInstruction" :
         *          {
         *              "InstructionIndex" : nbExecution,
         *              ...
         *          },
         *          "trace" : [Array of vertex indexes in the TPGGraph]
         *      },
         *      ...
         *  }
         *
         * }
         *
         * \param[in] filePath the path to the output file.
         * \param[in] noIndent true if the json format must not be indented. Files can
         * become large quickly with a lot of traces, so if the file purpose
         * is to be analysed by another program, set this to true to save some
         * space on your disk. Set noIndent to false if you want to keep
         * the file readable.
         */
        void writeStatsToJson(const char * filePath, bool noIndent = false) const;

    };

}


#endif // EXECUTION_STATS_H
