#ifndef GEGELATI_LALOGGER_H
#define GEGELATI_LALOGGER_H

#include <chrono>
#include <map>

#include "tpg/tpgGraph.h"
#include "learn/evaluationResult.h"
#include "log/Logger.h"


/// Learning Agent logger class that will be called during LA executions
class LALogger : public Logger {
protected:
    /// keeps the time of logger declaration to be able to show total durations
    std::shared_ptr<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> const start;
    /// keeps a given time to be able to show durations from that moment
    std::shared_ptr<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> checkpoint;

    /// \brief returns the duration from a given time to now
    /// \param[in] begin time from which we want to compute duration
    /// \return the duration from begin to now
    double getDurationFrom(std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> &begin);

    /// \brief gets the current time value, for exemple to set checkpoint
    /// \return the current time value
    std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> getTime();

public:
    /// \brief basic constructor setting cout as output, start and checkpoint as now
    LALogger()
            : Logger(),
              start(std::make_shared<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>>(
                      getTime())) {
        chronoFromNow();
    };

    /// \brief constructor defining a given output and setting start and checkpoint as now
    /// \param[in] out the output stream we want to log things to
    LALogger(std::ostream &out) : Logger(out),
                                     start(std::make_shared<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>>(
                                             getTime())) {
        chronoFromNow();
    };

    /// \brief updates checkpoint to now
    void chronoFromNow();

    /// \brief method called by the Learning Agent right after PopulateTPG is done
    /// \param[in] generationNumber the number of the current generation
    /// \param[in] tpg the tpg we currently have
    virtual void logAfterPopulateTPG(uint64_t &generationNumber, TPG::TPGGraph &tpg) = 0;

    /// \brief method called by the Learning Agent right after the evaluation is done
    /// \param[in] results scores of the evaluation
    virtual void
    logAfterEvaluate(std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex *> &results) = 0;

    /// \brief method called by the Learning Agent right after the decimation is done
    /// \param[in] tpg the tpg we currently have
    virtual void logAfterDecimate(TPG::TPGGraph &tpg) = 0;
};

#endif
