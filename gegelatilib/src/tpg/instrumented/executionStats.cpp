#include "tpg/instrumented/executionStats.h"

#include <numeric>
#include <algorithm>
#include <vector>

#include "tpg/instrumented/tpgVertexInstrumentation.h"
#include "tpg/instrumented/tpgTeamInstrumented.h"
#include "tpg/instrumented/tpgActionInstrumented.h"
#include "tpg/instrumented/tpgEdgeInstrumented.h"
#include "program/program.h"
#include "program/line.h"

void TPG::ExecutionStats::analyzeProgram(
    std::map<uint64_t, uint64_t>& instructionCounts, const Program::Program& program)
{

    for(int i = 0; i < program.getNbLines(); i++){
        instructionCounts[program.getLine(i).getInstructionIndex()]++;
    }

}

void TPG::ExecutionStats::analyzeInstrumentedGraph(const TPGGraph* graph)
{
    this->avgNbExecutionPerInstruction.clear();

    const auto roots = graph->getRootVertices();
    uint64_t nbInferences = std::accumulate(
        roots.cbegin(),
        roots.cend(),
        (uint64_t)0,
        [](uint64_t accu, const TPGVertex* vertex){
            const auto& rootTeam = dynamic_cast<const TPGTeamInstrumented&>(*vertex);
                // Raise std::bad_cast if not an instrumented team
            return accu + rootTeam.getNbVisits();
        });

    uint64_t nbEvaluatedTeams = 0;
    uint64_t nbEvaluatedPrograms = 0;
    uint64_t nbExecutedLines = 0;
    std::map<size_t, uint64_t> totalExecutionsPerInstruction;

    auto vertices = graph->getVertices();

    for(auto vertex : vertices){

        // Skip non-team instrumented vertices
        if(dynamic_cast<const TPGActionInstrumented*>(vertex))
            continue;

        auto& team = dynamic_cast<const TPGTeamInstrumented&>(*vertex);
            // Raise std::bad_cast if not an instrumented team

        nbEvaluatedTeams += team.getNbVisits();

        for(const auto * edge : team.getOutgoingEdges()){

            auto& instruEdge = dynamic_cast<const TPGEdgeInstrumented&>(*edge);
                // Raise std::bad_cast if not an instrumented edge

            uint64_t nbEdgeEval = instruEdge.getNbVisits();
            Program::Program& edgeProgram = instruEdge.getProgram();

            // Evaluated edge => edge program executed
            if(nbEdgeEval > 0){
                nbEvaluatedPrograms += nbEdgeEval;
                nbExecutedLines += nbEdgeEval * edgeProgram.getNbLines();

                std::map<uint64_t, uint64_t> linesPerInstruction;
                analyzeProgram(linesPerInstruction, edgeProgram);
                for(const auto& pair : linesPerInstruction){
                    totalExecutionsPerInstruction[pair.first]
                        += nbEdgeEval * pair.second;
                }
            }

        }

    }

    this->avgEvaluatedTeams = (double)nbEvaluatedTeams / (double)nbInferences;
    this->avgEvaluatedPrograms = (double)nbEvaluatedPrograms / (double)nbInferences;
    this->avgExecutedLines = (double)nbExecutedLines / (double)nbInferences;

    for(const auto& p : totalExecutionsPerInstruction){
        avgNbExecutionPerInstruction[p.first] = (double)p.second / (double)nbInferences;
    }

}

void TPG::ExecutionStats::analyzeInferenceTrace(
    const std::vector<const TPGVertex*>& trace)
{
    uint64_t nbEvaluatedTeams = trace.size() - 1;
        // Remove the action vertex at the end

    uint64_t nbEvaluatedPrograms = 0;
    uint64_t nbExecutedLines = 0;
    std::map<uint64_t, uint64_t> nbExecutionPerInstruction;

    // For of each visited teams, analysing its edges
    for(auto it = trace.begin(); it != trace.end()-1; it++){
        for(auto edge : (*it)->getOutgoingEdges()){

            // Edges leading to a previously visited teams (including the current team) are not evaluated
            auto endSearchIt = it+1;
            if( std::find(trace.begin(), endSearchIt, edge->getDestination()) != endSearchIt )
                continue;

            nbEvaluatedPrograms++;
            nbExecutedLines += edge->getProgram().getNbLines();

            analyzeProgram(nbExecutionPerInstruction, edge->getProgram());

        }
    }

    this->inferenceTracesStats.push_back({trace, nbEvaluatedTeams, nbEvaluatedPrograms, nbExecutedLines, nbExecutionPerInstruction});

}

void TPG::ExecutionStats::analyzeExecution(
    const TPG::TPGExecutionEngineInstrumented& tee, const TPG::TPGGraph* graph)
{
    clearInferenceTracesStats();

    analyzeInstrumentedGraph(graph);

    for(const auto& trace : tee.getTraceHistory())
        analyzeInferenceTrace(trace);

}

double TPG::ExecutionStats::getAvgEvaluatedTeams() const
{
    return this->avgEvaluatedTeams;
}
double TPG::ExecutionStats::getAvgEvaluatedPrograms() const
{
    return this->avgEvaluatedPrograms;
}
double TPG::ExecutionStats::getAvgExecutedLines() const
{
    return this->avgExecutedLines;
}
const std::map<size_t, double>& TPG::ExecutionStats::
    getAvgNbExecutionPerInstruction() const
{
    return this->avgNbExecutionPerInstruction;
}

const std::vector<TPG::TraceStats>& TPG::ExecutionStats::
    getInferenceTracesStats()
    const
{
    return this->inferenceTracesStats;
}

void TPG::ExecutionStats::clearInferenceTracesStats()
{
    this->inferenceTracesStats.clear();
}
