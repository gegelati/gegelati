/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2022) :
 *
 * Elinor Montmasson <elinor.montmasson@gmail.com> (2022)
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Mickaël Dardaillon <mdardail@insa-rennes.fr> (2022)
 * Thomas Bourgoin <tbourgoi@insa-rennes.fr> (2021)
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

#include "codeGen/tpgStackGenerationEngine.h"

CodeGen::TPGStackGenerationEngine::TPGStackGenerationEngine(
    const std::string& filename, const TPG::TPGGraph& tpg,
    const std::string& path)
    : TPGGenerationEngine(filename, tpg, path)
{
}

CodeGen::TPGStackGenerationEngine::~TPGStackGenerationEngine()
{
    fileMainH << "\n#endif" << std::endl;
    fileMain.close();
    fileMainH.close();
}

void CodeGen::TPGStackGenerationEngine::generateEdge(const TPG::TPGEdge& edge)
{
    const Program::Program& p = edge.getProgram();
    uint64_t progID;

    progGenerationEngine.setProgram(p);

    if (findProgramID(p, progID)) {
        progGenerationEngine.generateProgram(progID);
    }

    std::string destinationName;
    const TPG::TPGVertex* destination = edge.getDestination();

    if (dynamic_cast<const TPG::TPGTeam*>(destination) != nullptr) {
        destinationName = 'T' + std::to_string(findVertexID(*destination));
    }
    else if (dynamic_cast<const TPG::TPGAction*>(destination) != nullptr) {
        auto a = dynamic_cast<const TPG::TPGAction*>(destination);
        destinationName = 'A' + std::to_string(a->getActionClass()) + "_" +
                          std::to_string(a->getActionID());
    }

    fileMain << "\t\t\t{" << destinationName << "Vert, P" << progID << ", "
             << destinationName << "}";
}

void CodeGen::TPGStackGenerationEngine::generateTeam(const TPG::TPGTeam& team)
{
    uint64_t id = findVertexID(team);
    // print prototype and declaration of the function
    fileMain << "void T" << id << "(int* action){" << std::endl;
    fileMainH << "void T" << id << "(int* action);" << std::endl;
    // generate static array
    fileMain << "\tstatic Edge e[] = {" << std::endl;
    auto list = team.getOutgoingEdges();
    for (auto l = list.begin(); l != list.end(); l++) {
        if (l != list.begin()) {
            fileMain << "," << std::endl;
        }
        generateEdge(**l);
    }
    // print end static array
    fileMain << "\n\t};\n";
#ifdef DEBUG
    fileMain << "\tprintf(\"T%d\\n\", " << id << ");" << std::endl;
#endif
    // appel des fonction d'exécution
    fileMain << "\tint nbEdge = " << team.getOutgoingEdges().size() << ";"
             << std::endl;
    fileMain << "\texecuteTeam(e, nbEdge, action);\n}\n" << std::endl;
}

void CodeGen::TPGStackGenerationEngine::generateAction(
    const TPG::TPGAction& action)
{
    uint64_t id = action.getActionID();
    uint64_t actClass = action.getActionClass();
    // print prototype and declaration of the function
    fileMain << "void A" << actClass << "_" << id << "(int* action){"
             << std::endl;
    fileMainH << "void A" << actClass << "_" << id << "(int* action);"
              << std::endl;

    // print definition of the function
    fileMain << "\tif(action[" << actClass << "] == INT_MIN){" << std::endl;

    fileMain << "\t\taction[" << actClass << "] = " << id << ";" << std::endl;
    fileMain << "\t}\n" << std::endl;
    fileMain << "}\n" << std::endl;
}

void CodeGen::TPGStackGenerationEngine::setRoot(const TPG::TPGVertex& team)
{
    fileMainH << "\nextern void (*root)(int* action);" << std::endl;
    fileMain << "void (*root)(int*) = T" << findVertexID(team) << ";"
             << std::endl;
}

void CodeGen::TPGStackGenerationEngine::generateTPGGraph()
{
    initTpgFile();
    initHeaderFile();

    std::map<const TPG::TPGTeam*, std::list<TPG::TPGEdge*>> graph;
    auto vertices = this->tpg.getVertices();
    // give an id for each team of the graph
    for (auto vertex : vertices) {
        if (dynamic_cast<const TPG::TPGTeam*>(vertex) != nullptr) {
            generateTeam(*(const TPG::TPGTeam*)vertex);
        }
        else if (dynamic_cast<const TPG::TPGAction*>(vertex) != nullptr) {
            generateAction(*(const TPG::TPGAction*)vertex);
        }
    }
    setRoot(*tpg.getRootVertices().at(0));
}

void CodeGen::TPGStackGenerationEngine::initTpgFile()
{
    int nbActionClass = 0;
    for (auto vertex : this->tpg.getVertices()) {
        if (dynamic_cast<const TPG::TPGAction*>(vertex) != nullptr) {
            if ((*(const TPG::TPGAction*)vertex).getActionClass() + 1 >
                nbActionClass) {
                nbActionClass =
                    (int)(*(const TPG::TPGAction*)vertex).getActionClass() + 1;
            }
        }
    }

    std::string initAction = "";
    for (int i = 0; i < nbActionClass; ++i) {
        initAction += "\taction[" + std::to_string(i) + "] = INT_MIN;\n";
    }

    fileMain << "#include <limits.h> \n"
             << "#include <assert.h>\n"
             << "#include <stdio.h>\n"
             << "#include <stdint.h>\n"
             << "#include <stdbool.h>\n"
             << "#include <math.h>\n\n"

             << "void inferenceTPG(int *action){\n"
             << "\texecuteFromVertex(root, action);\n"
             << "}\n\n"

             << "void executeFromVertex(void (*ptr_f)(int*), int *action){\n"
             << initAction << "\tptr_f(action);\n"
             << "\n}\n\n"
             << std::endl;

    if (this->tpg.getNbEdgesActivable() == 1) {

        fileMain << "void executeTeam(Edge* e, int nbEdge, int* action){\n"
                 << "\tint idxBest = -1;\n"
                 << "\tdouble bestResult = -INFINITY;\n"
                 << "\tfor (int idx = 0; idx < nbEdge; ++idx) {\n"
                 << "\t\tdouble r = e[idx].ptr_prog();\n"
                 << "\t\tr = (isnan(r)) ? -INFINITY : r;\n"
                 << "\t\tif(r >= bestResult) {\n"
                 << "\t\t\t// If the result is better than best result, put "
                    "the result on best and the former best on second best\n"
                 << "\t\t\tbestResult = r;\n"
                 << "\t\t\tidxBest = idx;\n"
                 << "\t\t}\n"
                 << "\t}\n"
                 << "\te[idxBest].ptr_vertex(action);\n"
                 << "}\n"
                 << std::endl;
    }
    else if (this->tpg.getNbEdgesActivable() == 2) {

        fileMain << "void executeTeam(Edge* e, int nbEdge, int* action){\n"
                 << "\tint idxBest = -1, idxSecondBest = -1;\n"
                 << "\tdouble bestResult1 = -INFINITY;\n"
                 << "\tdouble bestResult2 = -INFINITY;\n"

                 << "\tfor (int idx = 0; idx < nbEdge; ++idx) {\n"
                 << "\t\tdouble r = e[idx].ptr_prog();\n"
                 << "\t\tr = (isnan(r)) ? -INFINITY : r;\n"

                 << "\t\tif(r >= bestResult2 && idx != idxBest) {\n"
                 << "\t\t\tif(r >= bestResult1) {\n"
                 << "\t\t\t\t// If the result is better than best result, put "
                    "the result on best and the former best on second best\n"
                 << "\t\t\t\tbestResult2 = bestResult1;\n"
                 << "\t\t\t\tidxSecondBest = idxBest;\n"

                 << "\t\t\t\tbestResult1 = r;\n"
                 << "\t\t\t\tidxBest = idx;\n"
                 << "\t\t\t} else {\n"
                 << "\t\t\t\t// If the result is better than second best "
                    "result, put the result on second best\n"
                 << "\t\t\t\tbestResult2 = r;\n"
                 << "\t\t\t\tidxSecondBest = idx;\n"
                 << "\t\t\t}\n"
                 << "\t\t}\n"
                 << "\t}\n"
                 << "\te[idxBest].ptr_vertex(action);\n"
                 << "\te[idxSecondBest].ptr_vertex(action);\n"

                 << "}\n"
                 << std::endl;
    }
    else {
        throw std::runtime_error("The number of Activable edges should not be "
                                 "higher than 2 for the codeGen "
                                 "It has not been implemented yet.");
    }
}

void CodeGen::TPGStackGenerationEngine::initHeaderFile()
{
    fileMainH << "#include <stdlib.h>\n\n";

    fileMainH << "typedef enum Vertex {";
    for (auto vertex : this->tpg.getVertices()) {
        fileMainH << vertexName(*vertex) << "Vert"
                  << ", ";
    }

    fileMainH << "} Vertex;\n\n"

              << "typedef struct Edge {\n"
              << "\tVertex destination;\n"
              << "\tdouble (*ptr_prog)();\n"
              << "\tvoid (*ptr_vertex)(int* action);\n"
              << "}Edge;\n\n"

              << "void inferenceTPG(int *action);\n"
              << "void executeFromVertex(void (*)(int*), int *action);\n"
              << "void executeTeam(Edge* e, int nbEdge, int *action);\n"
              << std::endl;
}

std::string CodeGen::TPGStackGenerationEngine::vertexName(
    const TPG::TPGVertex& v)
{
    std::ostringstream vertexName;
    if (dynamic_cast<const TPG::TPGTeam*>(&v) != nullptr) {
        vertexName << "T" << findVertexID(v);
    }
    else {
        vertexName << "A"
                   << dynamic_cast<const TPG::TPGAction*>(&v)->getActionClass()
                   << "_"
                   << dynamic_cast<const TPG::TPGAction*>(&v)->getActionID();
    }
    return vertexName.str();
}

#endif // CODE_GENERATION
