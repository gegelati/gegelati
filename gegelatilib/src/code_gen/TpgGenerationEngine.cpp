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

#define CODE_GENERATION
#ifdef CODE_GENERATION

#include "code_gen/TpgGenerationEngine.h"

const std::string TPG::TpgGenerationEngine::filenameProg = "program";

bool TPG::TpgGenerationEngine::findProgramID(const Program::Program& prog,
                                             uint64_t& id)
{
    auto iter = this->programID.find(&prog);
    if (iter == this->programID.end()) {
        // The vertex is not known yet
        this->programID.insert(std::pair<const Program::Program*, uint64_t>(
            &prog, this->nbPrograms));
        this->nbPrograms++;
        id = this->nbPrograms - 1;
        return true;
    }
    else {
        id = iter->second;
        return false;
    }
}

uint64_t TPG::TpgGenerationEngine::findVertexID(const TPG::TPGVertex& vertex)
{
    auto iter = this->vertexID.find(&vertex);
    if (iter == this->vertexID.end()) {
        // The vertex is not known yet
        this->vertexID.insert(std::pair<const TPG::TPGVertex*, uint64_t>(
            &vertex, this->nbVertex));
        this->nbVertex++;
        return nbVertex - 1;
    }
    else {
        return iter->second;
    }
}

void TPG::TpgGenerationEngine::generateEdge(const TPG::TPGEdge& edge)
{
    const Program::Program& p = edge.getProgram();
    uint64_t progID;
    std::cout << "Génération d'une edge" << std::endl;
    progGenerationEngine.setProgram(p);
    if (this->findProgramID(p, progID)){
        std::cout << "on connaissais pas le programme" << std::endl;
        progGenerationEngine.generateProgram(progID);
    }
}

void TPG::TpgGenerationEngine::generateTeam(const TPG::TPGTeam& team)
{
    //print prototype and declaration of the function
    //generate static array
    for (TPG::TPGEdge* e : team.getOutgoingEdges()){
        generateEdge(*e);
        //print "," de séparation
    }
    //print }
    // appel des fonction d'exécution
    //fin de la fonction de team
}

void TPG::TpgGenerationEngine::generateFromRoot()
{
    std::map<const TPG::TPGTeam*, std::list<TPGEdge*>> graph;
    auto vertices = this->tpg.getVertices();
    for(const TPG::TPGVertex* vertex : vertices ){
        if(typeid(*vertex) == typeid(TPG::TPGTeam)){
            //findVertexId()
        }
        else if(typeid(*vertex) == typeid(TPG::TPGAction)){

        }
    }
//    generateTeam(*(const TPG::TPGTeam*)vertex);
}

void TPG::TpgGenerationEngine::initTpgFile(){
    fileMain << "#include \"" << filenameProg << ".h\"" << std::endl;
    fileMain << "#include <limits.h> \n\n"
    << "int executeFromVertex(void*(*ptr_f)(int*action)){\n"
    << "\tvoid*(*f)(int*action) = ptr_f;\n"
    << "\tint action = INT_MIN;\n"
    << "\twhile (f!=NULL){\n"
    << "\t\tf= (void*(*)(int*)) (f(&action));\n"
    << "\t}\n"
    << "\treturn action;\n}\n\n"

    << "void* executeTeam(Edge* e, int nbEdge, double* input){\n"
    << "\tif(input == NULL){\n"
    << "\t\tfor (int i = 0; i < nbEdge; ++i) {\n"
    << "\t\t\te[i].visited = 0;\n"
    << "\t\t}\n"
    << "\t} else {\n"
    << "\t\tint idxNext = execute(e, nbEdge); // on récupère l'indice de l'edge qui à le programme qui renvoie la plus grande valeur\n"
    << "\t\tif(idxNext != -1) {\n"
    << "\t\t\te[idxNext].visited = 1; // on marque l'edge visité\n"
    << "\t\t\treturn e[idxNext].ptr_vertex; // on renvoie le pointeur de la prochaine team à exécuter\n"
    << "\t\t} else{\n"
    << "\t\t\treturn NULL; // cas d'erreur\n"
    << "\t\t}\n"
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
    << "\tr = e[idx].ptr_prog();"
    << "\t\tif(e[idx].visited == 0 && bestResult < r){\n"
    << "\t\t\tbestResult =r;\n"
    << "\t\t\tidxNext = idx;\n"
    << "\t\t}\n"
    << "\t\tidx++;\n"
    << "\t}\n"
    << "\treturn idxNext;\n"
    << "}\n"
    << std::endl;

}

void TPG::TpgGenerationEngine::initHeaderFile(){
    fileMainH
    << "#include <stdlib.h>\n\n"

    << "typedef struct Edge {\n"
    << "\tint visited;\n"
    << "\tdouble (*ptr_prog)(void);\n"
    << "\tvoid* (*ptr_vertex)(int* action);\n"
    << "}Edge;\n\n"

    << "int executeFromVertex(void*(*)(int*action));\n"
    << "void* executeTeam(Edge* e, int nbEdge, double* input);\n"
    << "int execute(Edge* e, int nbEdge);\n"
    << "void reset(void* (*p[])(double*, int*), int);\n"
    << std::endl;
}

#endif // CODE_GENERATION