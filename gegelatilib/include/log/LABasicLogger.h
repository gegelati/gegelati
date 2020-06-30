#ifndef LA_BASIC_LOGGER_H
#define LA_BASIC_LOGGER_H

#include "log/LALogger.h"

namespace Log {

    /**
     * \brief Basic logger that will display some useful information
     *
     * The information logged by this LALogger are generation number, nb of
     * vertices, min, mean, avg score of this generation and to finish some
     * timing. Everything is logged like a tab with regularly spaced columns.
     */
    class LABasicLogger : public LALogger
    {
      private:
        /**
         * Width of columns when logging values.
         */
        int colWidth = 17;

      public:
        /**
         * \brief Same constructor as LaLogger. Default output is cout.
         *
         * \param[in] out The output stream the logger will send elements to.
         */
        explicit LABasicLogger(std::ostream& out = std::cout) : LALogger(out)
        {
            logHeader();
        };

        /**
         * \brief Logs the header (column names) of the tab that will be logged.
         */
        void logHeader();

        /**
         * Inherited via LaLogger.
         *
         * \brief Logs the generation and vertices nb of the tpg.
         *
         * \param[in] generationNumber The number of the current generation.
         * \param[in] tpg The current tpg of the learning agent.
         */
        virtual void logAfterPopulateTPG(uint64_t& generationNumber,
                                         TPG::TPGGraph& tpg) override;

        /**
         * Inherited via LaLogger.
         *
         * \brief Logs the min, avg and max score of the generation,
         * and then logs eval time.
         *
         * \param[in] results Scores of the evaluation.
         */
        virtual void logAfterEvaluate(
            std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                          const TPG::TPGVertex*>& results) override;

        /**
         * Inherited via LaLogger.
         *
         * \brief Logs the decimate time and the total running time.
         *
         * \param[in] tpg The current tpg of the learning agent.
         */
        virtual void logAfterDecimate(TPG::TPGGraph& tpg) override;
    };
} // namespace Log

#endif
