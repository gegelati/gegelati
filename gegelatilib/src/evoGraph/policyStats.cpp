/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2020 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2020 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
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

#include "evoGraph/policyStats.h"
#include <algorithm>
#include <numeric>

/// Helper struct to group stats for a program type
struct ProgramTypeStats
{
    /// Map from Program pointer to usage count.
    const std::map<const Program::Program*, size_t>* nbUsePerProgram;
    /// Vector of line counts per program.
    const std::vector<size_t>* nbLinesPerProgram;
    /// Vector of intron line counts per program.
    const std::vector<size_t>* nbIntronPerProgram;
    /// Map from instruction index to usage count.
    const std::map<size_t, size_t>* nbUsagePerInstruction;
    /// Map from data location to usage count.
    const std::map<std::pair<size_t, size_t>, size_t>* nbUsagePerDataLocation;
    /// Label for the program type ("Context" or "Action").
    const char* label;
};

namespace {
    // Helper to print stats for a program type
    void printProgramTypeStats(std::ostream& os, const ProgramTypeStats& stats)
    {
        auto sumVec = [](const std::vector<size_t>& vec) {
            return std::accumulate(vec.cbegin(), vec.cend(), (size_t)0);
        };
        auto averageVec = [&sumVec](const std::vector<size_t>& vec) {
            return vec.empty() ? 0.0 : (double)sumVec(vec) / (double)vec.size();
        };

        os << "Programs:\t" << stats.nbUsePerProgram->size() << std::endl;
        os << "Line/prog:\t" << averageVec(*stats.nbLinesPerProgram)
           << std::endl;
        os << "Intr/prog:\t" << averageVec(*stats.nbIntronPerProgram)
           << std::endl;
        os << "Use/prog:\t";
        if (!stats.nbUsePerProgram->empty()) {
            os << (double)std::accumulate(stats.nbUsePerProgram->cbegin(),
                                          stats.nbUsePerProgram->cend(),
                                          size_t(0),
                                          [](size_t accu, const auto& val) {
                                              return accu + val.second;
                                          }) /
                      (double)stats.nbUsePerProgram->size();
        }
        os << std::endl;

        os << "Use/instr:\t";
        if (!stats.nbUsagePerInstruction->empty()) {
            os << (double)std::accumulate(
                      stats.nbUsagePerInstruction->cbegin(),
                      stats.nbUsagePerInstruction->cend(), size_t(0),
                      [](size_t accu, const std::pair<size_t, size_t>& val) {
                          return accu + val.second;
                      }) /
                      (double)stats.nbUsagePerInstruction->size();
        }
        os << ": ";
        for (const auto& val : *stats.nbUsagePerInstruction) {
            os << "{" << val.first << "," << val.second << "}";
        }
        os << std::endl << std::endl;

        os << "## Data info";
        if (!stats.nbUsagePerDataLocation->empty()) {
            size_t currentDHandler =
                stats.nbUsagePerDataLocation->begin()->first.first - 1;
            std::for_each(
                stats.nbUsagePerDataLocation->cbegin(),
                stats.nbUsagePerDataLocation->cend(),
                [&os, &currentDHandler, &stats](const auto& entry) {
                    if (entry.first.first != currentDHandler) {
                        os << "\n\n### DataHandler " << entry.first.first
                           << std::endl;
                        currentDHandler = entry.first.first;
                        size_t nbLocation = 0;
                        auto nbAccess = std::accumulate(
                            stats.nbUsagePerDataLocation->cbegin(),
                            stats.nbUsagePerDataLocation->cend(), size_t(0),
                            [&currentDHandler, &nbLocation](size_t accu,
                                                            const auto& val) {
                                if (val.first.first == currentDHandler) {
                                    nbLocation++;
                                    return accu + val.second;
                                }
                                else {
                                    return accu;
                                }
                            });
                        os << "Accesses:\t" << nbAccess << std::endl;
                        os << "Locations:\t" << nbLocation << std::endl;
                    }
                    os << "{" << entry.first.second << "," << entry.second
                       << "} ";
                });
        }
        os << std::endl;
    }
} // namespace

void EvoGraph::PolicyStats::clear()
{
    this->maxPolicyDepth = 0;
    this->nbDistinctTeams = 0;
    this->nbVertexPerDepthLevel.clear();
    this->nbLinesPerProgram.clear();
    this->nbIntronPerProgram.clear();
    this->nbOutgoingEdgesPerTeam.clear();
    this->nbUsagePerActionID.clear();
    this->nbUsagePerInstruction.clear();
    this->nbUsagePerDataLocation.clear();
    this->nbUsePerProgram.clear();
    this->nbUsePerTPGTeam.clear();
    this->nbUsePerAction.clear();
    this->nbUsePerActionProgram.clear();
    this->nbLinesPerActionProgram.clear();
    this->nbIntronPerActionProgram.clear();
    this->nbUsagePerInstructionActionProg.clear();
    this->nbUsagePerDataLocationActionProg.clear();
}

void EvoGraph::PolicyStats::setEnvironment(const Environment& env)
{
    this->environment = &env;
    this->dataSourcesAndRegisters.insert(
        dataSourcesAndRegisters.begin(),
        environment->getFakeDataSources().begin(),
        environment->getFakeDataSources().end());
}

void EvoGraph::PolicyStats::analyzeLine(const Program::Line* line,
                                   bool actionProgram)
{
    auto instructionIdx = line->getInstructionIndex();

    auto& nbUsagePerInstruction = actionProgram
                                      ? this->nbUsagePerInstructionActionProg
                                      : this->nbUsagePerInstruction;
    nbUsagePerInstruction[instructionIdx]++;

    const Instructions::Instruction& instruction =
        this->environment->getInstructionSet().getInstruction(instructionIdx);

    for (size_t operandIdx = 0; operandIdx < instruction.getNbOperands();
         operandIdx++) {
        const std::pair<size_t, size_t>& rawOperand =
            line->getOperand(operandIdx);
        const std::type_info& operandType =
            instruction.getOperandTypes().at(operandIdx).get();
        const Data::DataHandler& dHandler =
            dataSourcesAndRegisters.at(rawOperand.first).get();
        size_t scaledLocation =
            dHandler.scaleLocation(rawOperand.second, operandType);
        std::vector<size_t> accessedLocations =
            dHandler.getAddressesAccessed(operandType, scaledLocation);

        auto& nbUsagePerDataLocation =
            actionProgram ? this->nbUsagePerDataLocationActionProg
                          : this->nbUsagePerDataLocation;
        for (size_t accessedLocation : accessedLocations) {
            nbUsagePerDataLocation[{rawOperand.first, accessedLocation}]++;
        }
    }
}

void EvoGraph::PolicyStats::analyzeProgram(const Program::Program* prog)
{
    // Check if the Program was already analyzed
    auto& nbUsePerProgram = prog->isActionProgram()
                                ? this->nbUsePerActionProgram
                                : this->nbUsePerProgram;
    auto& nbLinesPerProgram = prog->isActionProgram()
                                  ? this->nbLinesPerActionProgram
                                  : this->nbLinesPerProgram;
    auto& nbIntronPerProgram = prog->isActionProgram()
                                   ? this->nbIntronPerActionProgram
                                   : this->nbIntronPerProgram;

    auto programIterator = nbUsePerProgram.find(prog);
    if (programIterator != nbUsePerProgram.end()) {
        programIterator->second++;
        return;
    }

    nbUsePerProgram.emplace(prog, 1);
    nbLinesPerProgram.push_back(prog->getNbLines());

    size_t nbIntronLines = 0;
    for (auto lineIdx = 0; lineIdx < prog->getNbLines(); lineIdx++) {
        if (!prog->isIntron(lineIdx)) {
            const Program::Line& line = prog->getLine(lineIdx);
            this->analyzeLine(&line, prog->isActionProgram());
        }
        else {
            nbIntronLines++;
        }
    }
    nbIntronPerProgram.push_back(nbIntronLines);
}

void EvoGraph::PolicyStats::analyzeTPGTeam(const EvoGraph::TPGTeam* team)
{
    size_t nbUse = ++this->nbUsePerTPGTeam[team];
    if (nbUse == 1) {
        this->nbDistinctTeams++;
        this->nbOutgoingEdgesPerTeam.push_back(team->getOutgoingEdges().size());
    }
}

void EvoGraph::PolicyStats::analyzeAction(const EvoGraph::Action* action)
{
    this->nbUsePerAction[action]++;
    this->nbUsagePerActionID[action->getActionID()]++;
}

void EvoGraph::PolicyStats::analyzePolicy(const EvoGraph::Vertex* root)
{
    size_t depth = 0;
    std::vector<const EvoGraph::Vertex*> stage[2];
    stage[0].push_back(root);
    while (!stage[depth % 2].empty()) {
        this->nbVertexPerDepthLevel[depth] = stage[depth % 2].size();
        auto& nextStage = stage[(depth + 1) % 2];
        nextStage.clear();

        for (const EvoGraph::Vertex* vertex : stage[depth % 2]) {
            if (auto team = dynamic_cast<const EvoGraph::TPGTeam*>(vertex)) {
                this->analyzeTPGTeam(team);
                if (this->nbUsePerTPGTeam[team] == 1) {
                    for (const EvoGraph::Edge* edge :
                         vertex->getOutgoingEdges()) {
                        this->analyzeProgram(&edge->getProgram());
                        nextStage.push_back(edge->getDestination());
                    }
                }
            }
            if (auto action = dynamic_cast<const EvoGraph::Action*>(vertex)) {
                this->analyzeAction(action);
                if (this->nbUsePerAction[action] == 1) {
                    for (auto edge : action->getOutgoingEdges()) {
                        this->analyzeProgram(&edge->getProgram());
                    }
                }
            }
        }
        depth++;
    }
    this->maxPolicyDepth = depth - 1;
}

std::ostream& EvoGraph::operator<<(std::ostream& os,
                              const EvoGraph::PolicyStats& policyStats)
{
    auto sumVec = [](const std::vector<size_t>& vec) {
        return std::accumulate(vec.cbegin(), vec.cend(), (size_t)0);
    };

    os << "# PolicyStats" << std::endl;
    os << "## Topology info" << std::endl;
    os << "Teams:\t\t" << policyStats.nbDistinctTeams << std::endl;
    os << "Edges:\t\t" << sumVec(policyStats.nbOutgoingEdgesPerTeam)
       << std::endl;
    os << "Actions:\t" << policyStats.nbUsePerAction.size() << std::endl;

    os << "Stages\t\t" << policyStats.maxPolicyDepth << std::endl;
    os << "Vertex/stage:\t";
    for (auto& nbVertexPerStage : policyStats.nbVertexPerDepthLevel) {
        os << "{" << nbVertexPerStage.first << "," << nbVertexPerStage.second
           << "} ";
    }
    os << std::endl;

    os << "Use/action:\t";
    if (!policyStats.nbUsagePerActionID.empty()) {
        os << (double)std::accumulate(
                  policyStats.nbUsagePerActionID.begin(),
                  policyStats.nbUsagePerActionID.end(), (size_t)0,
                  [](size_t accu, std::pair<size_t, size_t> val) {
                      return accu + val.second;
                  }) /
                  (double)policyStats.nbUsagePerActionID.size();
    }
    os << ": ";
    for (const auto& val : policyStats.nbUsagePerActionID) {
        os << "{" << val.first << "," << val.second << "} ";
    }
    os << std::endl;

    // Context program stats
    os << std::endl << "## Context Program info" << std::endl;
    printProgramTypeStats(
        os, ProgramTypeStats{&policyStats.nbUsePerProgram,
                             &policyStats.nbLinesPerProgram,
                             &policyStats.nbIntronPerProgram,
                             &policyStats.nbUsagePerInstruction,
                             &policyStats.nbUsagePerDataLocation, "Context"});

    // Action program stats (if any)
    if (!policyStats.nbUsePerActionProgram.empty()) {
        os << std::endl
           << std::endl
           << std::endl
           << "## Action Program info" << std::endl;
        printProgramTypeStats(
            os, ProgramTypeStats{&policyStats.nbUsePerActionProgram,
                                 &policyStats.nbLinesPerActionProgram,
                                 &policyStats.nbIntronPerActionProgram,
                                 &policyStats.nbUsagePerInstructionActionProg,
                                 &policyStats.nbUsagePerDataLocationActionProg,
                                 "Action"});
    }

    return os;
}
