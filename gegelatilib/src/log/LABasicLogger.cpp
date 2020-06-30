#include <numeric>
#include <iomanip>

#include "log/LABasicLogger.h"

void Log::LABasicLogger::logResults(std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex *> &results){
    auto iter = results.begin();
    double min = iter->first->getResult();
    std::advance(iter, results.size() - 1);
    double max = iter->first->getResult();
    double avg = std::accumulate(results.begin(), results.end(), 0.0,
                                 [](double acc,
                                    std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex *> pair) -> double {
                                     return acc + pair.first->getResult();
                                 });
    avg /= (double) results.size();
    *this << std::setw(colWidth) << min << std::setw(colWidth) << avg
          << std::setw(colWidth) << max;
}

void Log::LABasicLogger::logHeader() {
    // fixing float precision
    *this << std::setprecision(2) << std::fixed << std::left;
    *this << std::setw(colWidth) << "Gen" << std::setw(colWidth) << "NbVert"
          << std::setw(colWidth)
          << "Min" << std::setw(colWidth) << "Avg" << std::setw(colWidth)
          << "Max" << std::setw(colWidth) << "Duration(eval)";
    if (doValidation) {
        *this << std::setw(colWidth) << "Duration(valid)";
    }
    *this << std::setw(colWidth) << "Total_time" << std::endl;
}

void Log::LABasicLogger::logAfterPopulateTPG(uint64_t &generationNumber,
                                             TPG::TPGGraph &tpg) {
    *this << std::setw(colWidth) << generationNumber << std::setw(colWidth)
          << tpg.getNbVertices();
    // resets checkpoint to be able to show evaluation time
    chronoFromNow();
}

void Log::LABasicLogger::logAfterEvaluate(
        std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex *> &results) {
    evalTime = getDurationFrom(*checkpoint);

    // we only log results statistics if there is no validation
    if(!doValidation){
        logResults(results);
    }

    // resets checkpoint to be able to show validation time if there is some
    chronoFromNow();
}

void Log::LABasicLogger::logAfterValidate(
        std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex *> &results) {
    validTime = getDurationFrom(*checkpoint);

    // being in this method means validation is active, and so we are sure we
    // can log results
    logResults(results);
}

void Log::LABasicLogger::logEndOfTraining() {
    *this << std::setw(colWidth) << evalTime;
    if(doValidation) {
        *this << std::setw(colWidth) << validTime;
    }
    *this << std::setw(colWidth) << getDurationFrom(*start)
          << std::endl;
}
