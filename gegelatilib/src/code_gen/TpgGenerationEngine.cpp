/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019)
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

#include "code_gen/TpgGenerationEngine.h"

const std::string CodeGen::TPGGenerationEngine::filenameProg = "program";

void CodeGen::TPGGenerationEngine::generateEdge(const TPG::TPGEdge& edge)
{
    const Program::Program& p = edge.getProgram();
    uint64_t progID;

    progGenerationEngine.setProgram(p);

    if (findProgramID(p, progID)){
        progGenerationEngine.generateProgram(progID);
    }
    fileMain << "\t\t\t{0,P" << progID << ",";
    if(typeid(*(edge.getDestination())) == (typeid(TPG::TPGTeam))){
        fileMain << "T" << findVertexID(*(edge.getDestination())) << "}";
    }
    else if (typeid(*(edge.getDestination())) == (typeid(TPG::TPGAction))){
        auto a = dynamic_cast<const TPG::TPGAction*>(edge.getDestination());
        fileMain << "A" << a->getActionID() << "}";
    }

}

void CodeGen::TPGGenerationEngine::generateTeam(const TPG::TPGTeam& team)
{
    uint64_t id = findVertexID(team);
    //print prototype and declaration of the function
    fileMain << "void* T" << id << "(int* action){" << std::endl;
    fileMainH << "void* T" << id << "(int* action);" << std::endl;
    //generate static array
    fileMain << "\tstatic Edge e[] = {" << std::endl;
    auto list = team.getOutgoingEdges();
    for (auto l = list.begin(); l != list.end(); l++){
        if (l != list.begin()){
            fileMain << "," << std::endl;
        }
        generateEdge(**l);
    }
    //print end static array
    fileMain << "\n\t};" << std::endl;
#ifdef DEBUG
    fileMain << "\tprintf(\"T%d\\n\", " << id << ");" << std::endl;
#endif
    // appel des fonction d'exécution
    fileMain << "\tint nbEdge = " << team.getOutgoingEdges().size() << ";" << std::endl;
    fileMain << "\treturn executeTeam(e,nbEdge);\n}\n" << std::endl;

}

void CodeGen::TPGGenerationEngine::generateAction(const TPG::TPGAction& action){
    uint64_t id = action.getActionID();
    //print prototype and declaration of the function
    fileMain << "void* A" << id << "(int* action){" << std::endl;
    fileMainH << "void* A" << id << "(int* action);" << std::endl;

    //print definition of the function
    fileMain << "\t*action = " << id << ";" << std::endl;
    fileMain << "\treturn NULL;\n}\n" << std::endl;

}

void CodeGen::TPGGenerationEngine::setRoot(const TPG::TPGVertex& team){
    fileMainH << "\nvoid* (*root)(int* action);" << std::endl;
    fileMain << "void* (*root)(int* action) = T" << findVertexID(team) << ";" << std::endl;

}

void CodeGen::TPGGenerationEngine::generateTPGGraph(){
    std::map<const TPG::TPGTeam*, std::list<TPG::TPGEdge*>> graph;
    auto vertices = this->tpg.getVertices();
    //give an id for each team of the graph
    for(const TPG::TPGVertex* vertex : vertices ){
        if(typeid(*vertex) == typeid(TPG::TPGTeam)){
            findVertexID(*vertex);
        }
    }
    for (auto vertex : vertices) {
        if(typeid(*vertex) == typeid(TPG::TPGTeam)){
            generateTeam(*(const TPG::TPGTeam*)vertex);
        }
        else if(typeid(*vertex) == typeid(TPG::TPGAction)){
            generateAction(*(const TPG::TPGAction*)vertex);
        }
    }
    setRoot(*tpg.getRootVertices().at(0));
}
void CodeGen::TPGGenerationEngine::initTpgFile(){
    fileMain << "#include <limits.h> \n"
             << "#include <assert.h>\n"
             << "#include <stdio.h>\n"

             << "Stack s;\n\n"

             << "int executeFromVertex(void*(*ptr_f)(int*action)){\n"
             << "\tvoid*(*f)(int*action) = ptr_f;\n"
             << "\tint action = INT_MIN;\n"
             << "\twhile (f!=NULL){\n"
             << "\t\tf= (void*(*)(int*)) (f(&action));\n"
             << "\t}\n"
             << "\treturn action;\n}\n\n"

             << "void* executeTeam(Edge* e, int nbEdge){\n"
             << "\tint idxNext = execute(e, nbEdge); // on récupère l'indice de l'edge qui à le programme qui renvoie la plus grande valeur\n"
             << "\tif(idxNext != -1) {\n"
             << "\t\te[idxNext].visited = 1; // on marque l'edge visité\n"
             << "\t\tpush(&e[idxNext]);\n"
             << "\t\treturn e[idxNext].ptr_vertex; // on renvoie le pointeur de la prochaine team à exécuter\n"
             << "\t}\n"
             << "\treturn NULL;\n"
             << "}\n\n"

             << "int execute(Edge* e, int nbEdge){\n"
             << "\tdouble bestResult = e[0].ptr_prog();\n"
             << "\tint idxNext = 0;\n"
             << "\tint idx;\n"
             << "\tdouble r;\n"
             << "\twhile (e[idxNext].visited == 1){\n"
             << "\t\tidxNext++;\n"
             << "\t\tif(idxNext>= nbEdge){\n"
             << "\t\t\tprintf(\"Error all the edges of the team are already visited\\n\");\n"
             << "\t\t\treturn -1;\n"
             << "\t\t}\n"
             << "\t\tbestResult = e[idxNext].ptr_prog();\n"
             << "\t}\n"
             << "\tidx = idxNext+1;\n"
             << "\t//check if there exist another none visited edge with a better result\n"
             << "\twhile(idx < nbEdge){\n"
             << "\t\tr = e[idx].ptr_prog();\n"
             << "\t\tif(e[idx].visited == 0 &&  r >= bestResult){\n"
             << "\t\t\tbestResult =r;\n"
             << "\t\t\tidxNext = idx;\n"
             << "\t\t}\n"
             << "\t\tidx++;\n"
             << "\t}\n"
             << "\treturn idxNext;\n"
             << "}\n\n"

             << "void push( Edge* e){\n"
             << "\tif(s.top == s.max) {\n"
             << "\t\tif(s.max == 0){\n"
             << "\t\t\ts.e = (Edge **)(malloc((s.max + 1) * sizeof(Edge *)));\n"
             << "\t\t}\n"
             << "\t\telse{\n"
             << "\t\t\ts.e = (Edge **) (realloc(s.e, (s.max + 1) * sizeof(Edge *)));\n"
             << "\t\t}\n"
             << "\t\tassert(s.e != NULL);\n"
             << "\t\ts.max++;\n"
             << "\t}\n"
             << "\ts.e[s.top] = e;\n"
             << "\ts.top++;\n"
             << "}\n\n"

             << "Edge* pop(){\n"
             << "\tEdge* edge = NULL;\n"
             << "\tif(s.top > 0){\n"
             << "\t\ts.top--;\n"
             << "\t\tedge = s.e[s.top];\n"
             << "\t}\n"
             << "\tif(s.top == 0 && s.e != NULL){\n"
             << "\t\tfree(s.e);\n"
             << "\t\ts.max = 0;\n"
             << "\t}\n"
             << "\treturn edge;\n"
             << "}\n\n"

             << "void reset(){\n"
             << "\twhile (s.top > 0) {\n"
             << "\t\tpop()->visited = 0;\n"
             << "\t}\n"
             << "}\n"
             << std::endl;

}
void CodeGen::TPGGenerationEngine::initHeaderFile(){
    fileMainH
    << "#include <stdlib.h>\n\n"

    << "typedef struct Edge {\n"
    << "\tint visited;\n"
    << "\tdouble (*ptr_prog)(void);\n"
    << "\tvoid* (*ptr_vertex)(int* action);\n"
    << "}Edge;\n\n"

    << "typedef struct Stack {\n"
    << "\tEdge** e;\n"
    << "\tint max;\n"
    << "\tint top;\n"
    << "}Stack;"

    << "int executeFromVertex(void*(*)(int*action));\n"
    << "void* executeTeam(Edge* e, int nbEdge);\n"
    << "int execute(Edge* e, int nbEdge);\n"
    << "void push(Edge* e);\n"
    << "Edge* pop();\n"
    << "void reset();\n"
    << std::endl;
}

#endif // CODE_GENERATION