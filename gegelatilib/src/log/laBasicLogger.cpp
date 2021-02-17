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

#include <iomanip>
#include <numeric>

#include "learn/learningAgent.h"

#include "log/laBasicLogger.h"

void Log::LABasicLogger::logResults(
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const TPG::TPGVertex*>& results)
{
    auto iter = results.begin();
    double min = iter->first->getResult();
    std::advance(iter, results.size() - 1);
    double max = iter->first->getResult();
    double avg = std::accumulate(
        results.begin(), results.end(), 0.0,
        [](double acc,
           std::pair<std::shared_ptr<Learn::EvaluationResult>,
                     const TPG::TPGVertex*>
               pair) -> double { return acc + pair.first->getResult(); });
    avg /= (double)results.size();
    *this << std::setw(colWidth) << min << std::setw(colWidth) << avg
          << std::setw(colWidth) << max;
}

void Log::LABasicLogger::logHeader()
{
    // First line of header
    //*this << std::left;
    *this << std::setw(2 * colWidth) << " " << std::setw(colWidth) << "Train";
    if (doValidation) {
        *this << std::setw(2 * colWidth) << " " << std::setw(1 * colWidth)
              << "Valid";
    }
    *this << std::endl;

    // Second line of header
    //*this << std::right;
    *this << std::setw(colWidth) << "Gen" << std::setw(colWidth) << "NbVert"
          << std::setw(colWidth) << "Min" << std::setw(colWidth) << "Avg"
          << std::setw(colWidth) << "Max";
    if (doValidation) {
        *this << std::setw(colWidth) << "Min" << std::setw(colWidth) << "Avg"
              << std::setw(colWidth) << "Max";
    }
    *this << std::setw(colWidth) << "T_mutat" << std::setw(colWidth)
          << "T_eval";
    if (doValidation) {
        *this << std::setw(colWidth) << "T_valid";
    }
    *this << std::setw(colWidth) << "T_total" << std::endl;
}

void Log::LABasicLogger::logNewGeneration(uint64_t& generationNumber)
{
    *this << std::setw(colWidth) << generationNumber;
    // resets checkpoint to be able to show evaluation time
    chronoFromNow();
}

void Log::LABasicLogger::logAfterPopulateTPG()
{
    this->mutationTime = getDurationFrom(*checkpoint);

    *this << std::setw(colWidth)
          << this->learningAgent.getTPGGraph().getNbVertices();

    chronoFromNow();
}

void Log::LABasicLogger::logAfterEvaluate(
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const TPG::TPGVertex*>& results)
{
    evalTime = getDurationFrom(*checkpoint);

    logResults(results);

    // resets checkpoint to be able to show validation time if there is some
    chronoFromNow();
}

void Log::LABasicLogger::logAfterValidate(
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const TPG::TPGVertex*>& results)
{
    validTime = getDurationFrom(*checkpoint);

    // being in this method means validation is active, and so we are sure we
    // can log results
    logResults(results);
}

void Log::LABasicLogger::logEndOfTraining()
{
    *this << std::setw(colWidth) << mutationTime;
    *this << std::setw(colWidth) << evalTime;
    if (doValidation) {
        *this << std::setw(colWidth) << validTime;
    }
    *this << std::setw(colWidth) << getDurationFrom(*start) << std::endl;
}
