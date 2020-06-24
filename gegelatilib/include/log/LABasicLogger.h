#ifndef GEGELATI_LABASICLOGGER_H
#define GEGELATI_LABASICLOGGER_H

#include "log/LALogger.h"

namespace Log {

/**
* \brief Basic logger that will display some useful informations
*
* The aim of this logger is really basic : showing generation number, nb of
* vertices, min, mean, avg score of this generation and to finish some timing.
* Everything is logged like a tab with regularly spaces columns.
*/
    class LABasicLogger : public LALogger {
    private:
        /**
        * width of columns when logging values.
        */
        int colWidth = 17;

    public:
        /**
        * \brief same constructor as LaLogger.
        */
        LABasicLogger() : LALogger() {
            logHeader();
        };

        /**
        * \brief same constructor as LaLogger.
        * \param[in] out the output stream we want to log things to.
        */
        explicit LABasicLogger(std::ostream &out) : LALogger(out) {
            logHeader();
        };

        /**
        * \brief Logs the header (column names) of the tab we will log.
        */
        void logHeader();

        /**
         * Inherited via LaLogger.
         * \brief Logs the generation and vertices nb of the tpg.
         * \param[in] generationNumber the number of the current generation.
         * \param[in] tpg the tpg we currently have.
         */
        void
        logAfterPopulateTPG(uint64_t &generationNumber, TPG::TPGGraph &tpg);

        /**
         * Inherited via LaLogger.
         * \brief Logs the min, avg and max score of the generation,
         * and then logs eval time.
         * \param[in] results scores of the evaluation.
         */
        void logAfterEvaluate(
                std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex *> &results);

        /**
         * Inherited via LaLogger.
         * \brief Logs the decimate time and the total running time.
         * \param[in] tpg the tpg we currently have.
         */
        void logAfterDecimate(TPG::TPGGraph &tpg);
    };
}

#endif
