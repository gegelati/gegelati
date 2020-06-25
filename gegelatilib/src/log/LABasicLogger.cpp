#include <numeric>
#include <iomanip>

#include "log/LABasicLogger.h"

void Log::LABasicLogger::logHeader() {
    // fixing float precision
    *this << std::setprecision(2) << std::fixed << std::left;
    *this << std::setw(colWidth) << "Gen" << std::setw(colWidth) << "NbVert" << std::setw(colWidth)
          << "Min" << std::setw(colWidth) << "Avg" << std::setw(colWidth)
          << "Max" << std::setw(colWidth) << "Duration(eval)" << std::setw(colWidth)
          << "Duration(decim)" << std::setw(colWidth)
          << "Total_time" << std::endl;
}

void Log::LABasicLogger::logAfterPopulateTPG(uint64_t &generationNumber, TPG::TPGGraph &tpg) {
    *this << std::setw(colWidth) << generationNumber << std::setw(colWidth) << tpg.getNbVertices();
    // resets checkpoint to be able to show evaluation time
    chronoFromNow();
}

void Log::LABasicLogger::logAfterEvaluate(
        std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex *> &results) {
    auto iter = results.begin();
    double min = iter->first->getResult();
    std::advance(iter, results.size() - 1);
    double max = iter->first->getResult();
    double avg = std::accumulate(results.begin(), results.end(), 0.0,
                                 [](double acc,
                                    std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex *> pair) -> double {
                                     return acc + pair.first->getResult();
                                 });
    avg /= (double)results.size();
    *this << std::setw(colWidth) << min << std::setw(colWidth) << avg << std::setw(colWidth) << max
          << std::setw(colWidth)
          << getDurationFrom(*checkpoint);
    // resets checkpoint to be able to show decimation time
    chronoFromNow();
}

void Log::LABasicLogger::logAfterDecimate(TPG::TPGGraph &tpg) {
    *this << std::setw(colWidth) << getDurationFrom(*checkpoint) << std::setw(colWidth) << getDurationFrom(*start)
          << std::endl;
}