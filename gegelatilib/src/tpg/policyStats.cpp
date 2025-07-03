#include <algorithm>
#include <numeric>
#include "tpg/policyStats.h"

// Helper struct to group stats for a program type
struct ProgramTypeStats {
    const std::map<const Program::Program*, size_t>* nbUsePerProgram;
    const std::vector<size_t>* nbLinesPerProgram;
    const std::vector<size_t>* nbIntronPerProgram;
    const std::map<size_t, size_t>* nbUsagePerInstruction;
    const std::map<std::pair<size_t, size_t>, size_t>* nbUsagePerDataLocation;
    const char* label;
};

namespace {
    // Helper to print stats for a program type
    void printProgramTypeStats(std::ostream& os, const ProgramTypeStats& stats) {
        auto sumVec = [](const std::vector<size_t>& vec) {
            return std::accumulate(vec.cbegin(), vec.cend(), (size_t)0);
        };
        auto averageVec = [&sumVec](const std::vector<size_t>& vec) {
            return vec.empty() ? 0.0 : (double)sumVec(vec) / (double)vec.size();
        };

        os << "Programs:\t" << stats.nbUsePerProgram->size() << std::endl;
        os << "Line/prog:\t" << averageVec(*stats.nbLinesPerProgram) << std::endl;
        os << "Intr/prog:\t" << averageVec(*stats.nbIntronPerProgram) << std::endl;
        os << "Use/prog:\t";
        if (!stats.nbUsePerProgram->empty()) {
            os << (double)std::accumulate(
                stats.nbUsePerProgram->cbegin(),
                stats.nbUsePerProgram->cend(), size_t(0),
                [](size_t accu, const auto& val) { return accu + val.second; }
            ) / (double)stats.nbUsePerProgram->size();
        } else {
            os << "0";
        }
        os << std::endl;

        os << "Use/instr:\t";
        if (!stats.nbUsagePerInstruction->empty()) {
            os << (double)std::accumulate(
                stats.nbUsagePerInstruction->cbegin(),
                stats.nbUsagePerInstruction->cend(), size_t(0),
                [](size_t accu, const std::pair<size_t, size_t>& val) {
                    return accu + val.second;
                }
            ) / (double)stats.nbUsagePerInstruction->size();
        } else {
            os << "0";
        }
        os << ": ";
        for (const auto& val : *stats.nbUsagePerInstruction) {
            os << "{" << val.first << "," << val.second << "}";
        }
        os << std::endl << std::endl;

        os << "## Data info";
        if (!stats.nbUsagePerDataLocation->empty()) {
            size_t currentDHandler = stats.nbUsagePerDataLocation->begin()->first.first - 1;
            std::for_each(
                stats.nbUsagePerDataLocation->cbegin(),
                stats.nbUsagePerDataLocation->cend(),
                [&os, &currentDHandler, &stats](const auto& entry) {
                    if (entry.first.first != currentDHandler) {
                        os << "\n\n### DataHandler " << entry.first.first << std::endl;
                        currentDHandler = entry.first.first;
                        size_t nbLocation = 0;
                        auto nbAccess = std::accumulate(
                            stats.nbUsagePerDataLocation->cbegin(),
                            stats.nbUsagePerDataLocation->cend(), size_t(0),
                            [&currentDHandler, &nbLocation](size_t accu, const auto& val) {
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
                    os << "{" << entry.first.second << "," << entry.second << "} ";
                }
            );
        }
        os << std::endl;
    }
}

void TPG::PolicyStats::clear()
{
    this->maxPolicyDepth = 0;
    this->nbDistinctTeams = 0;
    this->nbTPGVertexPerDepthLevel.clear();
    this->nbLinesPerProgram.clear();
    this->nbIntronPerProgram.clear();
    this->nbOutgoingEdgesPerTeam.clear();
    this->nbUsagePerActionID.clear();
    this->nbUsagePerInstruction.clear();
    this->nbUsagePerDataLocation.clear();
    this->nbUsePerProgram.clear();
    this->nbUsePerTPGTeam.clear();
    this->nbUsePerTPGAction.clear();
    this->nbUsePerActionProgram.clear();
    this->nbLinesPerActionProgram.clear();
    this->nbIntronPerActionProgram.clear();
    this->nbUsagePerInstructionActionProg.clear();
    this->nbUsagePerDataLocationActionProg.clear();
}

void TPG::PolicyStats::setEnvironment(const Environment& env)
{
    this->environment = &env;
    this->dataSourcesAndRegisters.insert(
        dataSourcesAndRegisters.begin(),
        environment->getFakeDataSources().begin(),
        environment->getFakeDataSources().end());
}

void TPG::PolicyStats::analyzeLine(const Program::Line* line, bool actionProgram)
{
    auto instructionIdx = line->getInstructionIndex();

    auto& nbUsagePerInstruction = actionProgram ? this->nbUsagePerInstructionActionProg : this->nbUsagePerInstruction;
    nbUsagePerInstruction[instructionIdx]++;

    const Instructions::Instruction& instruction =
        this->environment->getInstructionSet().getInstruction(instructionIdx);

    for (size_t operandIdx = 0; operandIdx < instruction.getNbOperands(); operandIdx++) {
        const std::pair<size_t, size_t>& rawOperand = line->getOperand(operandIdx);
        const std::type_info& operandType = instruction.getOperandTypes().at(operandIdx).get();
        const Data::DataHandler& dHandler = dataSourcesAndRegisters.at(rawOperand.first).get();
        size_t scaledLocation = dHandler.scaleLocation(rawOperand.second, operandType);
        std::vector<size_t> accessedLocations = dHandler.getAddressesAccessed(operandType, scaledLocation);

        auto& nbUsagePerDataLocation = actionProgram ? this->nbUsagePerDataLocationActionProg : this->nbUsagePerDataLocation;
        for (size_t accessedLocation : accessedLocations) {
            nbUsagePerDataLocation[{rawOperand.first, accessedLocation}]++;
        }
    }
}

void TPG::PolicyStats::analyzeProgram(const Program::Program* prog)
{
    // Check if the Program was already analyzed
    auto& nbUsePerProgram = prog->isActionProgram() ? this->nbUsePerActionProgram : this->nbUsePerProgram;
    auto& nbLinesPerProgram = prog->isActionProgram() ? this->nbLinesPerActionProgram : this->nbLinesPerProgram;
    auto& nbIntronPerProgram = prog->isActionProgram() ? this->nbIntronPerActionProgram : this->nbIntronPerProgram;

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

void TPG::PolicyStats::analyzeTPGTeam(const TPG::TPGTeam* team)
{
    size_t nbUse = ++this->nbUsePerTPGTeam[team];
    if (nbUse == 1) {
        this->nbDistinctTeams++;
        this->nbOutgoingEdgesPerTeam.push_back(team->getOutgoingEdges().size());
    }
}

void TPG::PolicyStats::analyzeTPGAction(const TPG::TPGAction* action)
{
    this->nbUsePerTPGAction[action]++;
    this->nbUsagePerActionID[action->getActionID()]++;
}

void TPG::PolicyStats::analyzePolicy(const TPG::TPGVertex* root)
{
    size_t depth = 0;
    std::vector<const TPG::TPGVertex*> stage[2];
    stage[0].push_back(root);
    while (!stage[depth % 2].empty()) {
        this->nbTPGVertexPerDepthLevel[depth] = stage[depth % 2].size();
        auto& nextStage = stage[(depth + 1) % 2];
        nextStage.clear();

        for (const TPG::TPGVertex* vertex : stage[depth % 2]) {
            if (auto team = dynamic_cast<const TPG::TPGTeam*>(vertex)) {
                this->analyzeTPGTeam(team);
                if (this->nbUsePerTPGTeam[team] == 1) {
                    for (const TPG::TPGEdge* edge : vertex->getOutgoingEdges()) {
                        this->analyzeProgram(&edge->getProgram());
                        nextStage.push_back(edge->getDestination());
                    }
                }
            }
            if (auto action = dynamic_cast<const TPG::TPGAction*>(vertex)) {
                this->analyzeTPGAction(action);
                for (auto edge : action->getOutgoingEdges()) {
                    this->analyzeProgram(&edge->getProgram());
                }
            }
        }
        depth++;
    }
    this->maxPolicyDepth = depth - 1;
}

std::ostream& TPG::operator<<(std::ostream& os, const TPG::PolicyStats& policyStats)
{
    auto sumVec = [](const std::vector<size_t>& vec) {
        return std::accumulate(vec.cbegin(), vec.cend(), (size_t)0);
    };

    os << "# PolicyStats" << std::endl;
    os << "## Topology info" << std::endl;
    os << "Teams:\t\t" << policyStats.nbDistinctTeams << std::endl;
    os << "Edges:\t\t" << sumVec(policyStats.nbOutgoingEdgesPerTeam) << std::endl;
    os << "Actions:\t" << policyStats.nbUsePerTPGAction.size() << std::endl;

    os << "Stages\t\t" << policyStats.maxPolicyDepth << std::endl;
    os << "Vertex/stage:\t";
    for (auto& nbVertexPerStage : policyStats.nbTPGVertexPerDepthLevel) {
        os << "{" << nbVertexPerStage.first << "," << nbVertexPerStage.second << "} ";
    }
    os << std::endl;

    os << "Use/action:\t";
    if (!policyStats.nbUsagePerActionID.empty()) {
        os << (double)std::accumulate(
            policyStats.nbUsagePerActionID.begin(),
            policyStats.nbUsagePerActionID.end(), (size_t)0,
            [](size_t accu, std::pair<size_t, size_t> val) {
                return accu + val.second;
            }) / (double)policyStats.nbUsagePerActionID.size();
    } else {
        os << "0";
    }
    os << ": ";
    for (const auto& val : policyStats.nbUsagePerActionID) {
        os << "{" << val.first << "," << val.second << "} ";
    }
    os << std::endl;

    // Context program stats
    os << std::endl << "## Context Program info" << std::endl;
    printProgramTypeStats(os, ProgramTypeStats{
        &policyStats.nbUsePerProgram,
        &policyStats.nbLinesPerProgram,
        &policyStats.nbIntronPerProgram,
        &policyStats.nbUsagePerInstruction,
        &policyStats.nbUsagePerDataLocation,
        "Context"
    });

    // Action program stats (if any)
    if (!policyStats.nbUsePerActionProgram.empty()) {
        os << std::endl << std::endl << std::endl << "## Action Program info" << std::endl;
        printProgramTypeStats(os, ProgramTypeStats{
            &policyStats.nbUsePerActionProgram,
            &policyStats.nbLinesPerActionProgram,
            &policyStats.nbIntronPerActionProgram,
            &policyStats.nbUsagePerInstructionActionProg,
            &policyStats.nbUsagePerDataLocationActionProg,
            "Action"
        });
    }

    return os;
}