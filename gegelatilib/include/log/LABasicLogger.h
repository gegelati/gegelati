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

        /**
         * \brief Logs the min, avg and max score of the generation.
         *
         * This method is used by the eval and valid callback as
         * they both have the same input and want to log the same elements
         * (min, avg max).
         */
        void logResults(std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                                      const TPG::TPGVertex*>& results);

      public:
        /**
         * \brief Same constructor as LaLogger. Default output is cout.
         *
         * \param[in] out The output stream the logger will send elements to.
         */
        explicit LABasicLogger(std::ostream& out = std::cout) : LALogger(out)
        {
        }

        /**
         * Inherited via LaLogger
         *
         * \brief Logs the header (column names) of the tab that will be logged.
         */
        virtual void logHeader() override;

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
         * \brief Logs the min, avg and max score of the generation.
         *
         * If doValidation is true, it only updates eval time.
         * The method logResults will be called in order to log
         * statistics about results (method shared with logAfterValidate).
         *
         * \param[in] results scores of the evaluation.
         */
        virtual void logAfterEvaluate(
            std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                          const TPG::TPGVertex*>& results) override;

        /**
         * Inherited via LaLogger.
         *
         * \brief Does nothing in this logger.
         *
         * \param[in] tpg The current tpg of the learning agent.
         */
        virtual void logAfterDecimate(TPG::TPGGraph& tpg) override{
            // nothing to log
        };

        /**
         * Inherited via LaLogger.
         *
         * \brief Logs the min, avg and max score of the generation.
         *
         * If doValidation is true, no eval results are logged so that
         * the logger can only show validation results.
         *
         * \param[in] results scores of the validation.
         */
        virtual void logAfterValidate(
            std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                          const TPG::TPGVertex*>& results) override;

        /**
         * Inherited via LaLogger
         *
         * \brief Logs the eval, valid (if doValidation is true)
         * and total running time.
         */
        virtual void logEndOfTraining() override;
    };

} // namespace Log

#endif
