#include "tpg/instrumented/executionStats.h"

#include <numeric>

#include "tpg/instrumented/tpgVertexInstrumentation.h"
#include "tpg/instrumented/tpgTeamInstrumented.h"
#include "tpg/instrumented/tpgActionInstrumented.h"
#include "tpg/instrumented/tpgEdgeInstrumented.h"
#include "program/program.h"
#include "program/line.h"

void TPG::ExecutionStats::analyzeExecution(const TPGGraph* graph)
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

    auto vertices = graph->getVertices();

    // TODO make them class attributes ?
    uint64_t nbVisitedTeams = 0;
    uint64_t nbEvaluatedPrograms = 0;
    uint64_t nbExecutedLines = 0;
    std::map<size_t, uint64_t> totalExecutionsPerInstruction;

    for(auto vertex : vertices){

        // TODO in a function analyseTeam
        // Skip non-team instrumented vertices
        if(dynamic_cast<const TPGActionInstrumented*>(vertex))
            continue;

        auto& team = dynamic_cast<const TPGTeamInstrumented&>(*vertex);
            // Raise std::bad_cast if not an instrumented team

        nbVisitedTeams += team.getNbVisits();

        for(const auto * edge : team.getOutgoingEdges()){

            // TODO in a function analyzeEdge
            auto& instruEdge = dynamic_cast<const TPGEdgeInstrumented&>(*edge);
                // Raise std::bad_cast if not an instrumented edge

            uint64_t edgeVisits = instruEdge.getNbVisits();
            Program::Program& edgeProgram = instruEdge.getProgram();

            // Visited edge => edge program executed
            if(edgeVisits > 0){
                nbEvaluatedPrograms += edgeVisits;
                nbExecutedLines +=
                    edgeVisits * edgeProgram.getNbLines();

                // TODO in a function analyzeProgram
                for(int i = 0; i < edgeProgram.getNbLines(); i++){
                    Program::Line& line = edgeProgram.getLine(i);

                    totalExecutionsPerInstruction[line.getDestinationIndex()]
                        += edgeVisits;
                }

            }

        }
    }

    this->avgVisitedTeams = (double)nbVisitedTeams / (double)nbInferences;
    this->avgEvaluatedPrograms = (double)nbEvaluatedPrograms / (double)nbInferences;
    this->avgExecutedLines = (double)nbExecutedLines / (double)nbInferences;

    for(const auto& p : totalExecutionsPerInstruction){
        avgNbExecutionPerInstruction[p.first] = (double)p.second / (double)nbInferences;
    }

}

void TPG::ExecutionStats::analyzeInferenceTrace(
    const std::vector<const TPGVertex*> trace)
{
    // TODO
}
