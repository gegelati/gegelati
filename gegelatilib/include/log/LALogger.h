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
    /// saves a given reference time, used to calculate some durations
    std::shared_ptr<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> start;

    /// returns the duration from a given begining
    double getDurationFrom(std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> &begin);

    /// returns the current time value
    std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> getTime();

public:
    /// basic constructor setting cout as output and now as start
    LALogger() : Logger() {
        chronoFromNow();
    };

    /// constructor defining a given output and setting  now as start
    LALogger(std::ostream &stream) : Logger(stream) {
        chronoFromNow();
    };

    /// updates start to now
    void chronoFromNow();

    /// called by the Learning Agent right after PopulateTPG is done
    virtual void logAfterPopulateTPG(TPG::TPGGraph &tpg) = 0;

    /// called by the Learning Agent right after the evaluation is done
    virtual void logAfterEvaluate(uint64_t &generationNumber,
                                  std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex *> &results) = 0;

    /// called by the Learning Agent right after the decimation is done
    virtual void logAfterDecimate(TPG::TPGGraph &tpg) = 0;
};

#endif
