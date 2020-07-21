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

#include "learn/learningAgent.h"
#include "log/laLogger.h"

double Log::LALogger::getDurationFrom(
    const std::chrono::time_point<std::chrono::system_clock,
                                  std::chrono::nanoseconds>& begin) const
{
    return ((std::chrono::duration<double>)(getTime() - begin)).count();
}

std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>
Log::LALogger::getTime() const
{
    return std::chrono::system_clock::now();
}

Log::LALogger::LALogger(Learn::LearningAgent& la, std::ostream& out):
    Log::Logger(out),
    learningAgent(la),
    start(std::make_shared<std::chrono::time_point<
              std::chrono::system_clock, std::chrono::nanoseconds>>(getTime()))
{
    this->learningAgent.addLogger(*this);
    chronoFromNow();
};

void Log::LALogger::chronoFromNow()
{
    checkpoint = std::make_shared<std::chrono::time_point<
        std::chrono::system_clock, std::chrono::nanoseconds>>(getTime());
}
