#include <numeric>
#include <iomanip>

#include <log/LABasicLogger.h>

void LABasicLogger::logHeader() {
    *this << "Gen\t\tNbVert\t\tMin\t\t\tAvg\t\t\tMax\t\t\tDuration(eval)\t\tDuration(decimation)\tTotal_time\n";
}

void LABasicLogger::logAfterPopulateTPG(uint64_t &generationNumber, TPG::TPGGraph &tpg) {
    *this << generationNumber << "\t\t";
    *this << tpg.getNbVertices() << "\t\t\t";
    // resets checkpoint to be able to show evaluation time
    chronoFromNow();
}

void LABasicLogger::logAfterEvaluate(
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
    *this << std::setprecision(2) << std::fixed << min << "\t\t" << avg << "\t\t" << max << "\t\t"
          << getDurationFrom(*checkpoint)
          << "\t\t\t\t";
    // resets checkpoint to be able to show decimation time
    chronoFromNow();
}

void LABasicLogger::logAfterDecimate(TPG::TPGGraph &tpg) {
    *this << getDurationFrom(*checkpoint) << "\t\t\t\t\t" << getDurationFrom(*start) << "\n";
}