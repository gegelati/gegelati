//
// Created by asimonu on 19/06/2020.
//

#include <tpg/tpgGraph.h>
#include <learn/evaluationResult.h>
#include <chrono>
#include "Log.hpp"

#ifndef GEGELATI_LALOGGER_H
#define GEGELATI_LALOGGER_H


class LALogger : Logger {
protected:
    std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> start;

    double getDurationFrom(std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> begin);

    std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> getTime();

public:
    LALogger() : start(std::chrono::system_clock::now()) {};

    virtual void logAfterPopulateTPG(TPG::TPGGraph tpg) = 0;

    virtual void logEvaluate(uint64_t generationNumber,
                             std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex *> results) = 0;

    virtual void logAfterDecimate(TPG::TPGGraph tpg) = 0;
};

#endif //GEGELATI_LALOGGER_H
