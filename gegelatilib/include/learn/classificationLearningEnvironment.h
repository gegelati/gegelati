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

#ifndef CLASSIFICATION_LEARNING_ENVIRONMENT_H
#define CLASSIFICATION_LEARNING_ENVIRONMENT_H

#include <vector>

#include "learn/learningEnvironment.h"

namespace Learn {
    /**
     * \brief Specialization of the LearningEnvironment class for classification
     * purposes.
     */
    class ClassificationLearningEnvironment : public LearningEnvironment
    {
      protected:
        /**
         * \brief 2D array storing for each class the guesses that were made by
         * the LearningAgent.
         *
         * For example classificationTable.at(x).at(y) represents the number of
         * times a LearningAgent guessed class y, for a data from class x since
         * the last reset.
         */
        std::vector<std::vector<uint64_t>> classificationTable;

        /**
         * \brief Class of the current data.
         *
         * This attribute should be updated alongside the data made avalaible
         * to the LearningAgent.
         */
        uint64_t currentClass;

      public:
        /**
         * Main constructor of the ClassificationLearningEnvironment.
         *
         * \param[in] nbClass number of classes of the
         * classificationLearningEnvironment, and thus number of action of the
         * underlying LearningEnvironment.
         */
        ClassificationLearningEnvironment(uint64_t nbClass)
            : LearningEnvironment(nbClass), currentClass{0},
              classificationTable(nbClass, std::vector<uint64_t>(nbClass, 0)){};

        /**
         * \brief Get a const ref to the classification table of the learning
         * environment.
         */
        const std::vector<std::vector<uint64_t>>& getClassificationTable()
            const;

        /**
         * \brief Default implementation for the doAction method.
         *
         * This implementation only increments the classificationTable based on
         * the currentClass attribute. Refresh of the data should be implemented
         * by the child class, hence the pure virtual method.
         */
        virtual void doAction(uint64_t actionID) override = 0;

        /**
         * \brief Default scoring for classificationLearningEnvironment.
         *
         * The default scoring for the classificationLearningEnvironment is
         * computed based on the classificationTable attribute.
         *
         * The score represents the percentage of correct classification.
         */
        virtual double getScore() const override;

        /**
         * \brief Default implementation of the reset.
         *
         * Resets to zero the classificationTable.
         */
        virtual void reset(size_t seed = 0,
            LearningMode mode = LearningMode::TRAINING) override = 0;
    };
}; // namespace Learn

#endif
