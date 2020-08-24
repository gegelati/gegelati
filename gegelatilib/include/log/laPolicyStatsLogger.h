/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2020)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

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
