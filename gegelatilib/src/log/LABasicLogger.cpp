#include <log/LABasicLogger.h>

void LABasicLogger::logAfterPopulateTPG(TPG::TPGGraph& tpg){
    *this<<"elapsed time : "<<getDurationFrom(*start);
    // TODO
}

void LABasicLogger::logAfterEvaluate(uint64_t& generationNumber,
                 std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex *> &results){
    // TODO
}

void LABasicLogger::logAfterDecimate(TPG::TPGGraph& tpg){
    // TODO

}