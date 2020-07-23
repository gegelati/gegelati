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

#ifndef ADVERSARIAL_LEARNING_ENVIRONMENT_H
#define ADVERSARIAL_LEARNING_ENVIRONMENT_H

#include "learn/adversarialEvaluationResult.h"
#include "learn/learningEnvironment.h"
#include <vector>

namespace Learn {
    /**
     * \brief Interface for creating a Learning Environment in adversarial mode.
     *
     * The main difference with the base Learning Environment is that several
     * roots will participate in a single simulation.
     * It is the user responsibility to correctly adapt this class to the
     * problem. In fact, there should be 2 overrides that will be different from
     * the basic Learning Environment :
     * - doAction will be called by the Learning Agent with participating roots
     * in turn. If there are 3 roots in this simulation, the first will call
     * doAction, then the second one, and the third one. Then, it will be the
     * first root again. Note that each root will use the same dataSource.
     * It means the user has to update the dataSource so that each root will
     * see the input from its point of view (doing that, a root that was first
     * to act will not be lost of it becomes third).
     * - getScores will be called by the Learning Agent once, at the end of the
     * game. It shall return an evaluation result containing the scores of the
     * roots, in the order in which they participated.
     */
    class AdversarialLearningEnvironment : public LearningEnvironment
    {
        using LearningEnvironment::LearningEnvironment;

      public:
        /**
         * \brief Computes scores of each root and returns them.
         *
         * @return A shared pointer of an evaluation result containing a score
         * per participant.
         */
        virtual std::shared_ptr<Learn::AdversarialEvaluationResult> getScores()
            const = 0;

        /**
         * Inherited from LearningEnvironment
         *
         * \brief Simply returns the first score of the results, allowing
         * compatibility with non adversarial learning agents.
         *
         * @return The first score of the evaluation result from getScores().
         */
        double getScore() const override
        {
            return getScores()->getScoreOf(0);
        }
    };

} // namespace Learn
#endif
