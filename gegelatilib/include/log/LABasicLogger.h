#ifndef GEGELATI_LABASICLOGGER_H
#define GEGELATI_LABASICLOGGER_H

#include "log/LALogger.h"

/// Basic logger that will display some useful informations
class LABasicLogger : public LALogger {
public:
    /// same constructor as LaLogger
    LABasicLogger() : LALogger() {
        logHeader();
    };

    /// same constructor as LaLogger
    LABasicLogger(std::ostream &stream) : LALogger(stream) {
        logHeader();
    };

    /// Logs "Gen	NbVert	Min	Avg	Max	Duration(eval)	Duration(decimation)	Total_time"
    void logHeader();

    /// Inherited via LaLogger
    /// Logs the generation and vertices nb of the tpg
    void logAfterPopulateTPG(uint64_t &generationNumber, TPG::TPGGraph &tpg);

    /// Inherited via LaLogger
    /// Logs the min, avg and max score of the generation, and then logs eval time
    void logAfterEvaluate(std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex *> &results);

    /// Inherited via LaLogger
    /// Logs the decimate time and the total running time at this point
    void logAfterDecimate(TPG::TPGGraph &tpg);
};


#endif
