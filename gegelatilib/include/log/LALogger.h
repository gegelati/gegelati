#ifndef LA_LOGGER_H
#define LA_LOGGER_H

#include <chrono>
#include <map>

#include "tpg/tpgGraph.h"
#include "learn/evaluationResult.h"
#include "log/Logger.h"

namespace Log {

/**
* \brief Learning Agent logger class that will be called during LearningAgent
* executions.
*
* The idea of this class' methods is close to the idea of a callback.
* Each time the learning process does something, a specific method
* could be called by the LearningAgent. The many possible implementations
* of these methods allows the user to log whatever he wants.
*/
    class LALogger : public Logger {
    protected:
        /**
        * Keeps the time of logger declaration to be able to show
        * durations from the beginning of the learning.
        */
        std::shared_ptr<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> const start;

        /**
        * Keeps a given time to be able to show durations from that moment,
        * e.g. to know the duration of the eval.
        */
        std::shared_ptr<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> checkpoint;

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

        /**
        * \brief Computes the duration from a given time to now.
        *
        * \param[in] begin Time from which the durations will be computed
        * \return The duration from begin to now in seconds.
        */
        double getDurationFrom(const
                               std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> &begin) const;

        /**
        * \brief Gets the current time value, for example to set checkpoint.
        *
        * \return The current time value.
        */
        std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>
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
        * \param[in] out The output stream the logger will send elements to.
        */
        explicit LALogger(std::ostream &out = std::cout) : Logger(out),
                                                           start(std::make_shared<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>>(
                                                                   getTime())) {
            chronoFromNow();
        };


        /**
        * \brief Updates checkpoint to now.
        */
        void chronoFromNow();

        /**
        * \brief Logs the header (e.g. column names) of this logger.
        */
        virtual void logHeader() = 0;

        /**
        * \brief Method called by the Learning Agent right after
        * PopulateTPG is done.
        *
        * \param[in] generationNumber The number of the current generation.
        * \param[in] tpg The current tpg of the learning agent.
        */
        virtual void
        logAfterPopulateTPG(uint64_t &generationNumber, TPG::TPGGraph &tpg) = 0;

        /**
        * \brief Method called by the Learning Agent right after the evaluation
        * is done.
        *
        * \param[in] results scores of the evaluation.
        */

        virtual void
        logAfterEvaluate(
                std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex *> &results) = 0;

        /**
        * \brief Method called by the Learning Agent right after the decimation
        * is done.
        *
        * \param[in] tpg The current tpg of the learning agent.
        */
        virtual void logAfterDecimate(TPG::TPGGraph &tpg) = 0;

        /**
        * \brief Method called by the Learning Agent right after the validation
        * is done.
        *
        * \param[in] results scores of the validation.
        */
        virtual void logAfterValidate(
                std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex *> &results) = 0;

        /**
         * \brief Method called by the Learning Agent when the training is done
         */
        virtual void logEndOfTraining() = 0;
    };
}

#endif
