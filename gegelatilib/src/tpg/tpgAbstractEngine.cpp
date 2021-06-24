#include "tpg/tpgAbstractEngine.h"

bool TPG::TPGAbstractEngine::findProgramID(const Program::Program& prog,
                                             uint64_t& id){
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

uint64_t TPG::TPGAbstractEngine::findVertexID(const TPG::TPGVertex& vertex){
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


#include "tpg/tpgAbstractEngine.h"
