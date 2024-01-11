/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
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

#include <algorithm>
#include <numeric>

#include "learn/classificationLearningEnvironment.h"

void Learn::ClassificationLearningEnvironment::doAction(uint64_t actionID)
{
    // Base method
    LearningEnvironment::doAction(actionID);

    // Classification table update
    this->classificationTable.at(this->currentClass).at(actionID)++;
}

const std::vector<std::vector<uint64_t>>& Learn::
    ClassificationLearningEnvironment::getClassificationTable() const
{
    return this->classificationTable;
}

double Learn::ClassificationLearningEnvironment::getScore() const
{
    // Compute the average f1 score over all classes
    // (chosen instead of the global f1 score as it gives an equal weight to
    // the f1 score of each class, no matter its ratio within the observed
    // population)
    double averageF1Score = 0.0;

    // for each class
    for (uint64_t classIdx = 0; classIdx < classificationTable.size();
         classIdx++) {
        uint64_t truePositive = classificationTable.at(classIdx).at(classIdx);
        uint64_t falseNegative =
            std::accumulate(classificationTable.at(classIdx).begin(),
                            classificationTable.at(classIdx).end(),
                            (uint64_t)0) -
            truePositive;
        uint64_t falsePositive = 0;
        std::for_each(classificationTable.begin(), classificationTable.end(),
                      [&classIdx, &falsePositive](
                          const std::vector<uint64_t>& classifForClass) {
                          falsePositive += classifForClass.at(classIdx);
                      });
        falsePositive -= truePositive;

        double recall =
            (double)truePositive / (double)(truePositive + falseNegative);
        double precision =
            (double)truePositive / (double)(truePositive + falsePositive);
        // If true positive is 0, set score to 0.
        double fScore = (truePositive != 0)
                            ? 2 * (precision * recall) / (precision + recall)
                            : 0.0;
        averageF1Score += fScore;
    }

    averageF1Score /= (double)this->classificationTable.size();

    return averageF1Score;
}

void Learn::ClassificationLearningEnvironment::reset(size_t seed,
                                                     LearningMode mode,
                                                     uint16_t iterationNumber,
                                                     uint64_t generationNumber)
{
    // reset scores to 0 in classification table
    for (std::vector<uint64_t>& perClass : this->classificationTable) {
        for (uint64_t& score : perClass) {
            score = 0;
        }
    }
}
