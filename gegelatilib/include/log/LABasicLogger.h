#ifndef GEGELATI_LABASICLOGGER_H
#define GEGELATI_LABASICLOGGER_H

#include "log/LALogger.h"

/// Basic logger that will display some useful informations
class LABasicLogger : public LALogger {
protected:
    /// width of columns when logging values
    int colWidth = 17;

public:
    /// \brief same constructor as LaLogger
    LABasicLogger() : LALogger() {
        logHeader();
    };

    /// \brief same constructor as LaLogger
    /// \param[in] out the output stream we want to log things to
    LABasicLogger(std::ostream &out) : LALogger(out) {
        logHeader();
    };

    /// \brief Logs the header (column names) of the tab we will log
    void logHeader();

    /// Inherited via LaLogger
    /// Logs the generation and vertices nb of the tpg
    /// \param[in] generationNumber the number of the current generation
    /// \param[in] tpg the tpg we currently have
    void logAfterPopulateTPG(uint64_t &generationNumber, TPG::TPGGraph &tpg);

    /// Inherited via LaLogger
    /// Logs the min, avg and max score of the generation, and then logs eval time
    /// \param[in] results scores of the evaluation
    void logAfterEvaluate(std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex *> &results);

    /// Inherited via LaLogger
    /// Logs the decimate time and the total running time at this point
    /// \param[in] tpg the tpg we currently have
    void logAfterDecimate(TPG::TPGGraph &tpg);
};


#endif
