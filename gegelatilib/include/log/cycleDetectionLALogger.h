#include "laLogger.h"

namespace Log {

    /**
     * \brief Detector for connected cycles in TPGs
     *
     * This utility class implement a depth-first search algorithm for detecting
     * the presence of directed cyclic paths in TPGs.
     */
    class CycleDetectionLALogger : public LALogger
    {
      private:
        /// Control whether a message is printed when no loop is detected.
        bool logOnSuccess;

      public:
        /**
         * \brief Same constructor as LALogger. Default output is cerr.
         *
         * \param[in] la LearningAgent whose TPG will be analyzed by the
         * CycleDetectionLALogger.
         * \param[in] out The output stream the logger will send
         * elements to.
         * \param[in] logOnSuccess When true, the logger will log the absence of
         * cycles.
         */
        explicit CycleDetectionLALogger(Learn::LearningAgent& la,
                                        std::ostream& out = std::cerr,
                                        bool logOnSuccess = false)
            : LALogger(la, out), logOnSuccess(logOnSuccess)
        {
        }

        /**
         * Inherited via LALogger
         *
         * \brief Does nothing in this logger.
         */
        virtual void logHeader() override;

        /**
         * Inherited via LALogger.
         *
         * \brief Does nothing in this logger.
         */
        virtual void logNewGeneration(uint64_t& generationNumber) override;

        /**
         * Inherited via LALogger.
         *
         * \brief Checks the presence of directed cyclic path in the TPG. Loggs
         * an output if there is one.
         */
        virtual void logAfterPopulateTPG() override;

        /**
         * Inherited via LaLogger.
         *
         * \brief Does nothing in this logger.
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
         */
        virtual void logAfterDecimate() override;

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
         * \brief Does nothing in this logger.
         */
        virtual void logEndOfTraining() override;
    };
}; // namespace Log