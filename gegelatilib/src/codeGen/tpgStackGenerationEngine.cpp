/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2022) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Thomas Bourgoin <tbourgoi@insa-rennes.fr> (2021)
 * Mickael Dardaillon <mdardail@insa-rennes.fr> (2022)
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
    const std::string& path, const uint64_t& stackSize)
    : TPGGenerationEngine(filename, tpg, path), stackSize{stackSize}
{
    if (stackSize == 0) {
        throw std::runtime_error(
            "error the size of the call stack is equal to 0");
    }
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
    fileMain << "\t\t\t{0,P" << progID << ",";
    if (dynamic_cast<const TPG::TPGTeam*>((edge.getDestination())) != nullptr) {
        fileMain << "T" << findVertexID(*(edge.getDestination())) << "}";
    }
    else if (dynamic_cast<const TPG::TPGAction*>((edge.getDestination())) !=
             nullptr) {
        auto a = dynamic_cast<const TPG::TPGAction*>(edge.getDestination());
        fileMain << "A" << a->getActionID() << "}";
    }
}

void CodeGen::TPGStackGenerationEngine::generateTeam(const TPG::TPGTeam& team)
{
    uint64_t id = findVertexID(team);
    // print prototype and declaration of the function
    fileMain << "void* T" << id << "(int* action){" << std::endl;
    fileMainH << "void* T" << id << "(int* action);" << std::endl;
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
    fileMain << "\n\t};" << std::endl;
#ifdef DEBUG
    fileMain << "\tprintf(\"T%d\\n\", " << id << ");" << std::endl;
#endif
    // appel des fonction d'exécution
    fileMain << "\tint nbEdge = " << team.getOutgoingEdges().size() << ";"
             << std::endl;
    fileMain << "\treturn executeTeam(e,nbEdge);\n}\n" << std::endl;
}

void CodeGen::TPGStackGenerationEngine::generateAction(
    const TPG::TPGAction& action)
{
    uint64_t id = action.getActionID();
    // print prototype and declaration of the function
    fileMain << "void* A" << id << "(int* action){" << std::endl;
    fileMainH << "void* A" << id << "(int* action);" << std::endl;

    // print definition of the function
    fileMain << "\t*action = " << id << ";" << std::endl;
    fileMain << "\treturn NULL;\n}\n" << std::endl;
}

void CodeGen::TPGStackGenerationEngine::setRoot(const TPG::TPGVertex& team)
{
    fileMainH << "\nextern void* (*root)(int* action);" << std::endl;
    fileMain << "void* (*root)(int* action) = T" << findVertexID(team) << ";"
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
    fileMain
        << "#include <limits.h> \n"
        << "#include <assert.h>\n"
        << "#include <stdio.h>\n"
        << "#include <stdint.h>\n"
        << "#include <math.h>\n"

        << "#define stackSize  " << stackSize << "\n\n"

        << "Edge* callStack[stackSize];\n"
        << "uint32_t top = 0;\n\n"

        << "int inferenceTPG(){\n"
        << "\treset();\n"
        << "\treturn executeFromVertex(root);\n"
        << "}\n\n"

        << "int executeFromVertex(void*(*ptr_f)(int*action)){\n"
        << "\tvoid*(*f)(int*action) = ptr_f;\n"
        << "\tint action = INT_MIN;\n"
        << "\twhile (f!=NULL){\n"
        << "\t\tf= (void*(*)(int*)) (f(&action));\n"
        << "\t}\n"
        << "\treturn action;\n}\n\n"

        << "void* executeTeam(Edge* e, int nbEdge){\n"
        << "\tint idxNext = execute(e, nbEdge); \n"
        << "\tif(idxNext != -1) {\n"
        << "\t\te[idxNext].visited = 1;\n"
        << "\t\tpush(&e[idxNext]);\n"
        << "\t\treturn e[idxNext].ptr_vertex;\n"
        << "\t}\n"
        << "\treturn NULL;\n"
        << "}\n\n"

        << "int execute(Edge* e, int nbEdge){\n"
        << "\tdouble bestResult = e[0].ptr_prog();\n"
        << "\tbestResult = (isnan(bestResult)) ? -INFINITY : bestResult;\n "
        << "\tint idxNext = 0;\n"
        << "\tint idx;\n"
        << "\tdouble r;\n"
        << "\twhile (e[idxNext].visited == 1){\n"
        << "\t\tidxNext++;\n"
        << "\t\tif(idxNext>= nbEdge){\n"
        << "\t\t\tprintf(\"Error all the edges of the team are already "
           "visited\\n\");\n"
        << "\t\t\treturn -1;\n"
        << "\t\t}\n"
        << "\t\tbestResult = e[idxNext].ptr_prog();\n"
        << "\t\tbestResult = (isnan(bestResult))? -INFINITY : bestResult;\n"
        << "\t}\n"
        << "\tidx = idxNext+1;\n"
        << "\t//check if there exist another none visited edge with a better "
           "result\n"
        << "\twhile(idx < nbEdge){\n"
        << "\t\tr = e[idx].ptr_prog();\n"
        << "\t\tr = (isnan(r))? -INFINITY : r;"
        << "\t\tif(e[idx].visited == 0 &&  r >= bestResult){\n"
        << "\t\t\tbestResult =r;\n"
        << "\t\t\tidxNext = idx;\n"
        << "\t\t}\n"
        << "\t\tidx++;\n"
        << "\t}\n"
        << "\treturn idxNext;\n"
        << "}\n\n"

        << "void push( Edge* e){\n"
        << "\tif(top == stackSize) {\n"
        << "\t\tfprintf(stderr, \"Call stack of size %d is too small for the "
           "iteration of "
           "this TPG\", stackSize);\n"
        << "\t}\n"
        << "\tcallStack[top] = e;\n"
        << "\ttop++;"
        << "}\n\n"

        << "Edge* pop(){\n"
        << "\tEdge* edge = NULL;\n"
        << "if(top > 0){\n"
        << "\t\ttop--;\n"
        << "\t\tedge = callStack[top];\n"
        << "\t}\n"
        << "\treturn edge;\n"
        << "}\n\n"

        << "void reset(){\n"
        << "\twhile (top > 0) {\n"
        << "\t\tpop()->visited = 0;\n"
        << "\t}\n"
        << "}\n"
        << std::endl;
}

void CodeGen::TPGStackGenerationEngine::initHeaderFile()
{
    fileMainH << "#include <stdlib.h>\n\n"

              << "typedef struct Edge {\n"
              << "\tint visited;\n"
              << "\tdouble (*ptr_prog)();\n"
              << "\tvoid* (*ptr_vertex)(int* action);\n"
              << "}Edge;\n\n"

              << "int inferenceTPG();\n"
              << "int executeFromVertex(void*(*)(int*action));\n"
              << "void* executeTeam(Edge* e, int nbEdge);\n"
              << "int execute(Edge* e, int nbEdge);\n"
              << "void push(Edge* e);\n"
              << "Edge* pop();\n"
              << "void reset();\n"
              << std::endl;
}

#endif // CODE_GENERATION
