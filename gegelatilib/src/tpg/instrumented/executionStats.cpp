/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2022) :
 *
 * Elinor Montmasson <elinor.montmasson@gmail.com> (2022)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

#include "tpg/instrumented/executionStats.h"

#include <algorithm>
#include <fstream>
#include <json.h>
#include <numeric>
#include <vector>

#include "program/program.h"
#include "tpg/instrumented/tpgActionInstrumented.h"
#include "tpg/instrumented/tpgEdgeInstrumented.h"
#include "tpg/instrumented/tpgTeamInstrumented.h"
#include "tpg/instrumented/tpgVertexInstrumentation.h"

void TPG::ExecutionStats::analyzeProgram(
    std::map<uint64_t, uint64_t>& instructionCounts,
    const Program::Program& program)
{

    for (int i = 0; i < program.getNbLines(); i++) {
        instructionCounts[program.getLine(i).getInstructionIndex()]++;
    }
}

void TPG::ExecutionStats::analyzeInstrumentedGraph(const TPGGraph* graph)
{
    this->avgNbExecutionPerInstruction.clear();

    const auto roots = graph->getRootVertices();
    uint64_t nbInferences = std::accumulate(
        roots.cbegin(), roots.cend(), (uint64_t)0,
        [](uint64_t accu, const TPGVertex* vertex) {
            const auto& rootTeam =
                dynamic_cast<const TPGTeamInstrumented&>(*vertex);
            // Raise std::bad_cast if not an instrumented team
            return accu + rootTeam.getNbVisits();
        });

    uint64_t nbEvaluatedTeams = 0;
    uint64_t nbEvaluatedPrograms = 0;
    uint64_t nbExecutedLines = 0;
    std::map<size_t, uint64_t> totalExecutionsPerInstruction;

    auto vertices = graph->getVertices();

    for (auto vertex : vertices) {

        // Skip non-team instrumented vertices
        if (dynamic_cast<const TPGActionInstrumented*>(vertex))
            continue;

        auto& team = dynamic_cast<const TPGTeamInstrumented&>(*vertex);
        // Raise std::bad_cast if not an instrumented team

        nbEvaluatedTeams += team.getNbVisits();

        for (const auto* edge : team.getOutgoingEdges()) {

            auto& instruEdge = dynamic_cast<const TPGEdgeInstrumented&>(*edge);
            // Raise std::bad_cast if not an instrumented edge

            uint64_t nbEdgeEval = instruEdge.getNbVisits();
            Program::Program& edgeProgram = instruEdge.getProgram();

            // Evaluated edge => edge program executed
            if (nbEdgeEval > 0) {
                nbEvaluatedPrograms += nbEdgeEval;
                nbExecutedLines += nbEdgeEval * edgeProgram.getNbLines();

                std::map<uint64_t, uint64_t> linesPerInstruction;
                analyzeProgram(linesPerInstruction, edgeProgram);
                for (const auto& pair : linesPerInstruction) {
                    totalExecutionsPerInstruction[pair.first] +=
                        nbEdgeEval * pair.second;
                }
            }
        }
    }

    this->avgEvaluatedTeams = (double)nbEvaluatedTeams / (double)nbInferences;
    this->avgEvaluatedPrograms =
        (double)nbEvaluatedPrograms / (double)nbInferences;
    this->avgExecutedLines = (double)nbExecutedLines / (double)nbInferences;

    for (const auto& p : totalExecutionsPerInstruction) {
        avgNbExecutionPerInstruction[p.first] =
            (double)p.second / (double)nbInferences;
    }
}

void TPG::ExecutionStats::analyzeInferenceTrace(
    const std::vector<const TPGVertex*>& trace)
{

    // Get only the visited Teams
    std::vector<const TPG::TPGVertex*> visitedTeams;
    std::copy_if(trace.begin(), trace.end(), std::back_inserter(visitedTeams),
                 [](const TPG::TPGVertex* vertex) {
                     return dynamic_cast<const TPG::TPGTeam*>(vertex) !=
                            nullptr;
                 });

    uint64_t nbEvaluatedTeams = visitedTeams.size();
    // Remove the action vertex at the end

    uint64_t nbEvaluatedPrograms = 0;
    uint64_t nbExecutedLines = 0;
    std::map<uint64_t, uint64_t> nbExecutionPerInstruction;

    // For of each visited teams, analysing its edges
    for (auto it = visitedTeams.begin(); it != visitedTeams.end(); it++) {
        for (auto edge : (*it)->getOutgoingEdges()) {

            // Edges leading to a previously visited teams (including the
            // current team) are not evaluated
            auto endSearchIt = it + 1;
            if (std::find(visitedTeams.begin(), endSearchIt,
                          edge->getDestination()) != endSearchIt)
                continue;

            nbEvaluatedPrograms++;
            nbExecutedLines += edge->getProgram().getNbLines();

            analyzeProgram(nbExecutionPerInstruction, edge->getProgram());
        }
    }

    this->inferenceTracesStats.push_back({trace, nbEvaluatedTeams,
                                          nbEvaluatedPrograms, nbExecutedLines,
                                          nbExecutionPerInstruction});

    // Update distributions

    this->distribEvaluatedTeams[nbEvaluatedTeams]++;
    this->distribEvaluatedPrograms[nbEvaluatedPrograms]++;
    this->distribExecutedLines[nbExecutedLines]++;
    for (const auto& p : nbExecutionPerInstruction)
        this->distribNbExecutionPerInstruction[p.first][p.second]++;
    for (auto vertex : trace)
        this->distribUsedVertices[vertex]++;
}

void TPG::ExecutionStats::analyzeExecution(
    const TPG::TPGExecutionEngineInstrumented& tee, const TPG::TPGGraph* graph)
{
    clearInferenceTracesStats();
    this->lastAnalyzedGraph = graph; // Will be used by writeStatsToJson()

    analyzeInstrumentedGraph(graph);

    for (const auto& trace : tee.getTraceHistory())
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
    getInferenceTracesStats() const
{
    return this->inferenceTracesStats;
}

const std::map<size_t, size_t>& TPG::ExecutionStats::getDistribEvaluatedTeams()
    const
{
    return this->distribEvaluatedTeams;
}
const std::map<size_t, size_t>& TPG::ExecutionStats::
    getDistribEvaluatedPrograms() const
{
    return this->distribEvaluatedPrograms;
}
const std::map<size_t, size_t>& TPG::ExecutionStats::getDistribExecutedLines()
    const
{
    return this->distribExecutedLines;
}
const std::map<size_t, std::map<size_t, size_t>>& TPG::ExecutionStats::
    getDistribNbExecutionPerInstruction() const
{
    return this->distribNbExecutionPerInstruction;
}
const std::map<const TPG::TPGVertex*, size_t>& TPG::ExecutionStats::
    getDistribUsedVertices() const
{
    return this->distribUsedVertices;
}

void TPG::ExecutionStats::clearInferenceTracesStats()
{
    this->inferenceTracesStats.clear();

    this->distribEvaluatedTeams.clear();
    this->distribEvaluatedPrograms.clear();
    this->distribExecutedLines.clear();
    this->distribNbExecutionPerInstruction.clear();
    this->distribUsedVertices.clear();
}

void TPG::ExecutionStats::writeStatsToJson(const char* filePath,
                                           bool noIndent) const
{
    std::map<const TPGVertex*, unsigned int> vertexIndexes;
    if (this->lastAnalyzedGraph != nullptr) {
        // Store the index of each vertex in the TPGGraph in a lookup table
        // to print the execution traces.
        auto graphVertices = this->lastAnalyzedGraph->getVertices();
        for (int i = 0; i < graphVertices.size(); i++) {
            vertexIndexes[graphVertices[i]] = i;
        }
    }

    std::ofstream outputFile(filePath);

    Json::Value root;

    // Average statistics
    root["ExecutionStats"]["avgEvaluatedTeams"] = this->avgEvaluatedTeams;
    root["ExecutionStats"]["avgEvaluatedPrograms"] = this->avgEvaluatedPrograms;
    root["ExecutionStats"]["avgExecutedLines"] = this->avgExecutedLines;

    for (const auto& p : this->avgNbExecutionPerInstruction)
        root["ExecutionStats"]["avgNbExecutionPerInstruction"]
            [std::to_string(p.first)] = p.second;

    // Distributions
    for (const auto& p : this->distribEvaluatedTeams)
        root["ExecutionStats"]["distributionEvaluatedTeams"]
            [std::to_string(p.first)] = p.second;

    for (const auto& p : this->distribEvaluatedPrograms)
        root["ExecutionStats"]["distributionEvaluatedPrograms"]
            [std::to_string(p.first)] = p.second;

    for (const auto& p : this->distribExecutedLines)
        root["ExecutionStats"]["distributionExecutedLines"]
            [std::to_string(p.first)] = p.second;

    for (const auto& p1 : this->distribNbExecutionPerInstruction) {
        for (const auto& p2 : p1.second)
            root["ExecutionStats"]["distributionNbExecutionPerInstruction"]
                [std::to_string(p1.first)][std::to_string(p2.first)] =
                    p2.second;
    }

    for (const auto& p : this->distribUsedVertices) {
        size_t idxVertex = vertexIndexes[p.first];
        root["ExecutionStats"]["distributionUsedVertices"]
            [std::to_string(idxVertex)] = p.second;
    }

    // Trace statistics
    int i = 0;
    for (auto& stats : this->getInferenceTracesStats()) {
        std::string nbTrace = std::to_string(i);

        if (this->lastAnalyzedGraph != nullptr) {
            for (int j = 0; j < stats.trace.size(); j++) {
                root["TracesStats"][nbTrace]["trace"][j] =
                    vertexIndexes[stats.trace[j]];
            }
        }

        root["TracesStats"][nbTrace]["nbEvaluatedTeams"] =
            stats.nbEvaluatedTeams;
        root["TracesStats"][nbTrace]["nbEvaluatedPrograms"] =
            stats.nbEvaluatedPrograms;
        root["TracesStats"][nbTrace]["nbExecutedLines"] = stats.nbExecutedLines;
        for (const auto& p : stats.nbExecutionPerInstruction)
            root["TracesStats"][nbTrace]["nbExecutionPerInstruction"]
                [std::to_string(p.first)] = p.second;

        i++;
    }

    Json::StreamWriterBuilder writerFactory;
    // Set a precision to 6 digits after the point.
    writerFactory.settings_["precision"] = 6U;
    if (noIndent)
        writerFactory.settings_["indentation"] = "";
    Json::StreamWriter* writer = writerFactory.newStreamWriter();
    writer->write(root, &outputFile);
    delete writer;

    outputFile.close();
}
