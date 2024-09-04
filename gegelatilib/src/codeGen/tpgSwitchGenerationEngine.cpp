/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2022) :
 *
 * Elinor Montmasson <elinor.montmasson@gmail.com> (2022)
 * Karol Desnos <kdesnos@insa-rennes.fr> (2022)
 * Mickaël Dardaillon <mdardail@insa-rennes.fr> (2022)
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

#ifdef CODE_GENERATION

#include "codeGen/tpgSwitchGenerationEngine.h"

void CodeGen::TPGSwitchGenerationEngine::generateEdge(const TPG::TPGEdge& edge)
{
    const Program::Program& p = edge.getProgram();
    uint64_t progID;

    progGenerationEngine.setProgram(p);

    if (findProgramID(p, progID)) {
        progGenerationEngine.generateProgram(progID);
    }
    fileMain << "P" << progID << "()";
}

void CodeGen::TPGSwitchGenerationEngine::generateTeam(const TPG::TPGTeam& team)
{
    auto edges = team.getOutgoingEdges();
    auto teamName = vertexName(team);

    auto nextVertices = std::vector<const TPG::TPGVertex*>();
    for (const auto* edge : edges)
        nextVertices.push_back(edge->getDestination());

    // Destinations
    fileMain << "\t\t\tconst enum vertices next[" << edges.size() << "] = { ";
    for (auto nextVertex : nextVertices) {
        fileMain << vertexName(*nextVertex) << ", ";
    }
    fileMain << " };" << std::endl << std::endl;

    // score array
    fileMain << "\t\t\tdouble " << vertexName(team) << "Scores["
             << team.getOutgoingEdges().size() << "];" << std::endl;

    fileMain << std::endl;

    int i = 0;
    for (const auto* edge : edges) {
        fileMain << "\t\t\t" << teamName << "Scores[" << i << "] = ";
        ++i;
        generateEdge(*edge);
        fileMain << ";" << std::endl;
    }
    fileMain << std::endl;

    fileMain << "\t\t\tint best = bestProgram(" << teamName << "Scores, "
             << edges.size() << ");" << std::endl;
    fileMain << "\t\t\tcurrentVertex = next[best];" << std::endl;
}

void CodeGen::TPGSwitchGenerationEngine::generateAction(
    const TPG::TPGAction& action)
{
    uint64_t id = action.getActionID();
    fileMain << "\t\t\t*action = " << id << ";" << std::endl;
}

void CodeGen::TPGSwitchGenerationEngine::generateTPGGraph()
{

    if (this->tpg.getNbEdgesActivable() != 1) {
        throw std::runtime_error(
            "The number of Activable edges should not be different to 1 for "
            "the switch codeGen."
            "It has not been implemented yet on switch, please use the stack "
            "codeGen for multiAction TPGs.");
    }

    initTpgFile();
    initHeaderFile();

    std::map<const TPG::TPGTeam*, std::list<TPG::TPGEdge*>> graph;
    auto vertices = this->tpg.getVertices();

    // generate enum of teams and actions for readability
    fileMain << "enum vertices {";
    for (auto vertex : vertices) {
        fileMain << vertexName(*vertex) << ", ";
    }
    fileMain << "};" << std::endl << std::endl;

    // generate inference function
    fileMain << "void inferenceTPG(int* action) {" << std::endl;

    // start graph on root
    fileMain << "\tenum vertices currentVertex = "
             << vertexName(*tpg.getRootVertices().at(0)) << ";" << std::endl;

    // Init the action to INT_MIN
    fileMain << "\t*action = INT_MIN;" << std::endl;

    // generate switch case to navigate the graph
    fileMain << "\twhile(*action == INT_MIN) {" << std::endl;
    fileMain << "\t\tswitch (currentVertex) {" << std::endl;
    for (auto vertex : vertices) {
        fileMain << "\t\tcase " << vertexName(*vertex) << ": {" << std::endl;
        if (dynamic_cast<const TPG::TPGTeam*>(vertex) != nullptr) {
            generateTeam(*(const TPG::TPGTeam*)vertex);
        }
        else if (dynamic_cast<const TPG::TPGAction*>(vertex) != nullptr) {
            generateAction(*(const TPG::TPGAction*)vertex);
        }
        fileMain << "\t\t\tbreak;" << std::endl;
        fileMain << "\t\t}" << std::endl;
    }
    fileMain << "\t\t}" << std::endl;
    fileMain << "\t}" << std::endl;
    fileMain << "}" << std::endl;
}

void CodeGen::TPGSwitchGenerationEngine::initTpgFile()
{
    fileMain
        << "#include <limits.h>\n"
        << "#include <assert.h>\n"
        << "#include <float.h>\n"
        << "#include <stdbool.h>\n"
        << "#include <stdio.h>\n"
        << "#include <stdint.h>\n"
        << "#include <math.h>\n"
        << "\n"

        << "int bestProgram(double *results, int nb) {\n"
        << "\tint bestProgram = 0;\n"
        << "\tdouble bestScore = (isnan(results[0]))? -INFINITY : results[0];\n"
        << "\tfor (int i = 1; i < nb; i++) {\n"
        << "\t\tdouble challengerScore = (isnan(results[i]))? -INFINITY : "
           "results[i];\n"
        << "\t\tif (challengerScore >= bestScore) {\n"
        << "\t\t\tbestProgram = i;\n"
        << "\t\t\tbestScore = challengerScore;\n"
        << "\t\t}\n"
        << "\t}\n"
        << "\treturn bestProgram;\n"
        << "}\n"

        << std::endl;
}
void CodeGen::TPGSwitchGenerationEngine::initHeaderFile()
{
    fileMainH << "#include <stdlib.h>\n\n"
              << "void inferenceTPG(int* action);\n";
}

std::string CodeGen::TPGSwitchGenerationEngine::vertexName(
    const TPG::TPGVertex& v)
{
    std::ostringstream vertexName;
    if (dynamic_cast<const TPG::TPGTeam*>(&v) != nullptr) {
        vertexName << "T";
    }
    else {
        vertexName << "A";
    }
    vertexName << findVertexID(v);
    return vertexName.str();
}

#endif // CODE_GENERATION
