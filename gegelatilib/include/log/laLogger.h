/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2020)
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
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

#ifndef LA_LOGGER_H
#define LA_LOGGER_H

#include <chrono>
#include <map>
#include <ostream>

#include "learn/evaluationResult.h"
#include "log/logger.h"
#include "tpg/tpgGraph.h"

namespace Learn {
    class LearningAgent;
}

namespace Log {

    /**
     * \brief Learning Agent logger class that will be called during
     * LearningAgent executions.
     *
     * The idea of this class' methods is close to the idea of a callback.
     * Each time the learning process does something, a specific method
     * could be called by the LearningAgent. The many possible implementations
     * of these methods allows the user to log whatever he wants.
     */
    class LALogger : public Logger
    {
      protected:
        /**
         * Keeps the time of logger declaration to be able to show
         * durations from the beginning of the learning.
         */
        std::shared_ptr<std::chrono::time_point<
            std::chrono::system_clock, std::chrono::nanoseconds>> const start;

        /**
         * Keeps a given time to be able to show durations from that moment,
         * e.g. to know the duration of the eval.
         */
        std::shared_ptr<std::chrono::time_point<std::chrono::system_clock,
                                                std::chrono::nanoseconds>>
            checkpoint;

        /**
         * Keeps the duration of the mutation to be able to log it
         * some time after it is computed.
         */
        double mutationTime = 0;

        /**
         * Keeps the duration of the evaluation to be able to log it
         * some time after it is computed.
         */
        double evalTime = 0;

        /**
         * Keeps the duration of the validation to be able to log it
         * some time after it is computed.
         */
        double validTime = 0;

        /// LearningAgent logged by the LALogger
        Learn::LearningAgent& learningAgent;

        /**
         * \brief Computes the duration from a given time to now.
         *
         * \param[in] begin Time from which the durations will be computed
         * \return The duration from begin to now in seconds.
         */
        double getDurationFrom(
            const std::chrono::time_point<std::chrono::system_clock,
                                          std::chrono::nanoseconds>& begin)
            const;

        /**
         * \brief Gets the current time value, for example to set checkpoint.
         *
         * \return The current time value.
         */
        std::chrono::time_point<std::chrono::system_clock,
                                std::chrono::nanoseconds>
        getTime() const;

      public:
        /**
         * Boolean telling the logger if the training will make a validation
         */
        bool doValidation = false;

        /**
         * \brief Constructor defining a given output and setting start and
         * checkpoint as now. Default output is cout.
         *
         * The constructed LALogger will add itself automatically to the Loggers
         * of the given LearningAgent.
         *
         * \param[in] la The LearningAgent which will be logged by this
         * LALogger.
         * \param[in] out The output stream the logger will send
         * elements to.
         */
        explicit LALogger(Learn::LearningAgent& la,
                          std::ostream& out = std::cout);

        /**
         * \brief Updates checkpoint to now.
         */
        void chronoFromNow();

        /**
         * \brief Logs the header (e.g. column names) of this logger.
         */
        virtual void logHeader() = 0;

        /**
         * \brief Method called by the LearningAgent at the start of a
         * generation.
         *
         * \param[in] generationNumber The number of the current
         * generation.
         */
        virtual void logNewGeneration(uint64_t& generationNumber) = 0;

        /**
         * \brief Method called by the Learning Agent right after
         * PopulateTPG is done.
         *
         */
        virtual void logAfterPopulateTPG() = 0;

        /**
         * \brief Method called by the Learning Agent right after the evaluation
         * is done.
         *
         * \param[in] results scores of the evaluation.
         */

        virtual void logAfterEvaluate(
            std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                          const TPG::TPGVertex*>& results) = 0;

        /**
         * \brief Method called by the Learning Agent right after the decimation
         * is done.
         */
        virtual void logAfterDecimate() = 0;

        /**
         * \brief Method called by the Learning Agent right after the validation
         * is done.
         *
         * \param[in] results scores of the validation.
         */
        virtual void logAfterValidate(
            std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                          const TPG::TPGVertex*>& results) = 0;

        /**
         * \brief Method called by the Learning Agent when the training is done
         */
        virtual void logEndOfTraining() = 0;
    };
} // namespace Log

#endif
