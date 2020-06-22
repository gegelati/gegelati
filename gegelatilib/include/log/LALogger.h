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

    /// returns the duration from a given begining
    double getDurationFrom(std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> &begin);

    /// returns the current time value
    std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> getTime();

public:
    /// basic constructor setting cout as output, start and checkpoint as now
    LALogger()
            : Logger(),
              start(std::make_shared<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>>(
                      getTime())) {
        chronoFromNow();
    };

    /// constructor defining a given output and setting start and checkpoint as now
    LALogger(std::ostream &stream) : Logger(stream),
                                     start(std::make_shared<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>>(
                                             getTime())) {
        chronoFromNow();
    };

    /// updates checkpoint to now
    void chronoFromNow();

    /// called by the Learning Agent right after PopulateTPG is done
    virtual void logAfterPopulateTPG(uint64_t &generationNumber, TPG::TPGGraph &tpg) = 0;

    /// called by the Learning Agent right after the evaluation is done
    virtual void
    logAfterEvaluate(std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex *> &results) = 0;

    /// called by the Learning Agent right after the decimation is done
    virtual void logAfterDecimate(TPG::TPGGraph &tpg) = 0;
};

#endif
