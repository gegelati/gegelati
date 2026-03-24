/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Mickaël Dardaillon <mdardail@insa-rennes.fr> (2022)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
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

#include "codeGen/codeGenerationEngine.h"
#include "data/demangle.h"
#include "util/timestamp.h"

CodeGen::CodeGenerationEngine::CodeGenerationEngine(const std::string& filename,
                                                  const std::string& path)
    : AbstractEngine()
{

    this->fileMain.open(path + filename + ".c", std::ofstream::out);
    this->fileMainH.open(path + filename + ".h", std::ofstream::out);
    if (!fileMain.is_open() || !fileMainH.is_open()) {
        throw std::runtime_error("Error can't open " +
                                 std::string(path + filename) + ".c or " +
                                 std::string(path + filename) + ".h");
    }

    fileMain << "/**\n"
             << " * File generated with GEGELATI v" GEGELATI_VERSION "\n"
             << " * On the " << Util::getCurrentDate() << "\n"
             << " * With the " << DEMANGLE_TYPEID_NAME(typeid(*this).name())
             << ".\n"
             << " */\n\n";

    fileMain << "#include \"" << filename << ".h\"" << std::endl;

    fileMainH << "/**\n"
              << " * File generated with GEGELATI v" GEGELATI_VERSION "\n"
              << " * On the " << Util::getCurrentDate() << "\n"
              << " * With the " << DEMANGLE_TYPEID_NAME(typeid(*this).name())
              << ".\n"
              << " */\n\n";
    fileMainH << "#ifndef C_" << filename << "_H" << std::endl;
    fileMainH << "#define C_" << filename << "_H\n" << std::endl;
};

CodeGen::CodeGenerationEngine::~CodeGenerationEngine()
{
    fileMainH << "\n#endif" << std::endl;
    fileMain.close();
    fileMainH.close();
}


void CodeGen::CodeGenerationEngine::initFile()
{
    fileMain
        << "#include <limits.h> \n"
        << "#include <assert.h>\n"
        << "#include <stdio.h>\n"
        << "#include <stdint.h>\n"
        << "#include <stdbool.h>\n"
        << "#include <math.h>\n\n"

        << "void inferenceTPG(double* action){\n"
        << "\texecuteFromVertex(root, action);\n"
        << "}\n\n"

        << "void executeFromVertex(void*(*ptr_f)(double*), double* action){\n"
        << "\tvoid*(*f)(double*) = ptr_f;\n"
        << "\twhile (f!=NULL){\n"
        << "\t\tf = (void*(*)(double*)) (f(action));\n"
        << "\t}\n"
        << "}\n\n"

        << "void* executeTeam(Edge* e, int nbEdge){\n"
        << "\tint idxNext = execute(e, nbEdge); \n"
        << "\tif(idxNext != -1) {\n"
        << "\t\treturn e[idxNext].ptr_vertex;\n"
        << "\t}\n"
        << "\treturn NULL;\n"
        << "}\n\n"

        << "int execute(Edge* e, int nbEdge){\n"
        << "\tdouble bestResult;\n"
        << "\tint idxNext = 0;\n"
        << "\tint idx;\n"
        << "\tdouble r;\n\n"

        << "\tbestResult = e[idxNext].ptr_prog();\n"
        << "\tbestResult = (isnan(bestResult)) ? -INFINITY : bestResult;\n"
        << "\tidx = idxNext + 1;\n\n"

        << "\twhile (idx < nbEdge){\n"
        << "\t\tr = e[idx].ptr_prog();\n"
        << "\t\tr = (isnan(r)) ? -INFINITY : r;\n"
        << "\t\tif (r >= bestResult){\n"
        << "\t\t\tbestResult = r;\n"
        << "\t\t\tidxNext = idx;\n"
        << "\t\t}\n"
        << "\t\tidx++;\n"
        << "\t}\n"
        << "\treturn idxNext;\n"
        << "}\n"
        << std::endl;
}

void CodeGen::CodeGenerationEngine::initHeaderFile()
{
    fileMainH << "#include <stdlib.h>\n\n";

    fileMainH << "typedef enum Vertex {";
    for (auto vertex : this->tpg.getVertices()) {
        fileMainH << vertexName(*vertex) << "Vert" << ", ";
    }

    fileMainH << "} Vertex;\n\n"
              << "typedef struct Edge {\n"
              << "\tVertex destination;\n"
              << "\tdouble (*ptr_prog)();\n"
              << "\tvoid* (*ptr_vertex)(double* action);\n"
              << "}Edge;\n\n"

              << "void inferenceTPG(double* action);\n"
              << "void executeFromVertex(void*(*)(double*), double* action);\n"
              << "void* executeTeam(Edge* e, int nbEdge);\n"
              << "int execute(Edge* e, int nbEdge);\n"
              << std::endl;
}

void CodeGen::CodeGenerationEngine::generateAgent(const Algorithm::Agent& agent, const Algorithm::Algorithm& algorithm) 
{

    std::map<size_t, std::reference_wrapper<const Algorithm::Algorithm>> potentialAlgorithms;
    std::vector<std::reference_wrapper<const Algorithm::Algorithm>> algorithmsToRegister = {algorithm};
    while(algorithmsToRegister.size() != 0) {
        const Algorithm::Algorithm& currentAlgo = algorithmsToRegister.front();
        algorithmsToRegister.erase(algorithmsToRegister.begin());
        potentialAlgorithms.insert({currentAlgo.getAlgorithmID(), currentAlgo});

        auto subAlgorithms = algorithm.cGetSubAlgorithms();
        algorithmsToRegister.insert(algorithmsToRegister.begin(), subAlgorithms.begin(), subAlgorithms.end());
    }

    std::set<std::reference_wrapper<const Algorithm::Algorithm>> initializedAlgorithm;
    algorithm.initCodeGeneratedFile(fileMain, fileMainH);
    initializedAlgorithm.insert(algorithm);

    std::set<std::reference_wrapper<const Algorithm::Agent>> agentsToGenerate;
    agentsToGenerate.insert(agent);
    std::set<std::reference_wrapper<const Algorithm::Agent>> generatedAgents;
    while(agentsToGenerate.size() > 0) {
        if(initializedAlgorithm.find(potentialAlgorithms.at(agent.getAlgorithmID())) == initializedAlgorithm.end()) {
            potentialAlgorithms.at(agent.getAlgorithmID()).get().initCodeGeneratedFile(fileMain, fileMainH);
        }
        potentialAlgorithms.at(agent.getAlgorithmID()).get().codeGenerateAgent(agent, fileMain, fileMainH, agentsToGenerate);
        generatedAgents.insert(agent);

        for(const Algorithm::Agent& agentToGenerate: agentsToGenerate) {
            if(generatedAgents.find(agentToGenerate) != generatedAgents.end()){
                agentsToGenerate.insert(agentToGenerate);
            }
        }
    }
}


void CodeGen::CodeGenerationEngine::generateAgents(const Algorithm::Agent& agent, const Algorithm::Algorithm& algorithm) 
{

    std::map<size_t, std::reference_wrapper<const Algorithm::Algorithm>> potentialAlgorithms;
    std::vector<std::reference_wrapper<const Algorithm::Algorithm>> algorithmsToRegister = {algorithm};
    while(algorithmsToRegister.size() != 0) {
        const Algorithm::Algorithm& currentAlgo = algorithmsToRegister.front();
        algorithmsToRegister.erase(algorithmsToRegister.begin());
        potentialAlgorithms.insert({currentAlgo.getAlgorithmID(), currentAlgo});

        auto subAlgorithms = algorithm.cGetSubAlgorithms();
        algorithmsToRegister.insert(algorithmsToRegister.begin(), subAlgorithms.begin(), subAlgorithms.end());
    }

    std::set<std::reference_wrapper<const Algorithm::Algorithm>> initializedAlgorithm;
    algorithm.initCodeGeneratedFile(fileMain, fileMainH);
    initializedAlgorithm.insert(algorithm);

    std::set<std::reference_wrapper<const Algorithm::Agent>> agentsToGenerate;
    agentsToGenerate.insert(agent);
    std::set<std::reference_wrapper<const Algorithm::Agent>> generatedAgents;
    while(agentsToGenerate.size() > 0) {
        if(initializedAlgorithm.find(potentialAlgorithms.at(agent.getAlgorithmID())) == initializedAlgorithm.end()) {
            potentialAlgorithms.at(agent.getAlgorithmID()).get().initCodeGeneratedFile(fileMain, fileMainH);
        }
        potentialAlgorithms.at(agent.getAlgorithmID()).get().codeGenerateAgent(agent, fileMain, fileMainH, agentsToGenerate);
        generatedAgents.insert(agent);

        for(const Algorithm::Agent& agentToGenerate: agentsToGenerate) {
            if(generatedAgents.find(agentToGenerate) != generatedAgents.end()){
                agentsToGenerate.insert(agentToGenerate);
            }
        }
    }
}

#endif // CODE_GENERATION
