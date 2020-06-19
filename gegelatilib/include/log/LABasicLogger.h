#ifndef GEGELATI_LABASICLOGGER_H
#define GEGELATI_LABASICLOGGER_H


#include "log/LALogger.h"

/// Basic logger that will display some useful informations
class LABasicLogger : public LALogger {
public:
    /// same constructor as LaLogger
    LABasicLogger() : LALogger() {};

    /// same constructor as LaLogger
    LABasicLogger(std::ostream &stream) : LALogger(stream) {};

    /// Inherited via LaLogger
    void logAfterPopulateTPG(TPG::TPGGraph &tpg);

    /// Inherited via LaLogger
    void logAfterEvaluate(uint64_t &generationNumber,
                          std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex *> &results);

    /// Inherited via LaLogger
    void logAfterDecimate(TPG::TPGGraph &tpg);
};


#endif
