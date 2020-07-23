#ifndef LA_POLICY_STATS_LOGGER_H
#define LA_POLICY_STATS_LOGGER_H

#include "log/laLogger.h"
#include "tpg/policyStats.h"
#include "tpg/tpgVertex.h"

namespace Log {
    /**
     * \brief LALogger specialization using logging the PolicyStats information
     * on the best root.
     *
     * After each evaluation of the TPG root vertices by the LearningAgent, this
     * LALogger logs the PolicyStats of the bestRoot into its output stream.
     */
    class LAPolicyStatsLogger : public LALogger
    {
      private:
        /**
         * \brief Last best root TPG::TPGVertex whose policyStats was printed in
         * the log.
         *
         * After each evaluation, the policyStats of a root is printed in the
         * stream only if a new root is marked as the bestRoot.
         */
        const TPG::TPGVertex* lastBestRoot = nullptr;

        /// Number of the current generation.
        uint64_t generationNumber;

      public:
        /**
         * \brief Main constructor for the LAPolicyStatsLogger.
         *
         * \param[in] la LearningAgent whose information will be logged by the
         * LAPolicyStatsLogger.
         * \param[in] out ostream where the logger will write its output.
         */
        LAPolicyStatsLogger(Learn::LearningAgent& la,
                            std::ostream& out = std::cout)
            : LALogger(la, out){};

        /// Inherited from LALogger
        void logNewGeneration(uint64_t& generationNumber) override;

        /// Inherited from LALogger
        void logHeader() override{
            // nothing to log
        };

        /// Inherited from LALogger
        void logAfterPopulateTPG() override{
            // nothing to log
        };

        /// Inherited from LALogger
        void logAfterDecimate() override;

        /// Inherited from LALogger
        void logAfterValidate(
            std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                          const TPG::TPGVertex*>& results) override{
            // nothing to log
        };

        /// Inherited from LALogger
        void logEndOfTraining() override{
            // nothing to log
        };

        /// Inherited from LALogger
        void logAfterEvaluate(
            std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                          const TPG::TPGVertex*>& results) override{
            // nothing to log
        };
    };
}; // namespace Log

#endif // !LA_POLICY_STATS_LOGGER_H
