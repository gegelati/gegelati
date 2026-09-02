/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2025)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2024 - 2025)
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

#ifndef REINFORCEMENT_ENVIRONMENT_H
#define REINFORCEMENT_ENVIRONMENT_H

#include "evaluation/learningEnvironment.h"

namespace Evaluation {

    /**
     * \brief Interface for creating a Learning Environment.
     *
     * This class defines all the method that should be implemented for a
     * Learner to interact with an learning environment and learn to interact
     * with it.
     *
     * Interaction with a learning environment are made through a discrete set
     * of actions. As a result of these actions, the learning environment may
     * update its state, accessible through the data sources it provides. The
     * learning environment also provides a score resulting from the past
     * actions, and a termination boolean indicating that the
     * learningEnvironment has reached a final state, that no action will
     * affect.
     */
    class ReinforcementEnvironment : public LearningEnvironment
    {
      protected:

      public:
        /**
         * \brief Constructor for LearningEnviroment.
         *
         * \param[in] inputDimensions the dimensions of the input sources.
         * \param[in] outputDimension the dimensions of the output source.
         */
        ReinforcementEnvironment(const std::vector<Data::DataType>& inputDimensions, const Data::DataType& outputDimension)
            : LearningEnvironment(inputDimensions, outputDimension) {};

        /**
         * \brief Execute an action on the LearningEnvironment.
         *
         * \param[in] action the view representing the action to
         * execute.
         * \throw std::runtime_error if the action does not correspond to the output dimension.
         */
        virtual void doAction(const Data::DataView& action);

        /**
         * \brief Reset the LearningEnvironment.
         *
         * Resetting a learning environment is needed to train an agent.
         * Optionally seed can be given to this function to control the
         * randomness of a LearningEnvironment (if any). When available, this
         * feature will be used:
         * - for comparing the performance of several agents with the same
         * random starting conditions.
         * - for training each agent with diverse starting conditions.
         *
         * \param[in] seed the integer value for controlling the randomness of
         * the LearningEnvironment.
         * \param[in] mode LearningMode in which the Environment should be
         * reset for the next set of actions.
         * \param[in] iterationNumber the integer value to indicate the current
         * iteration number when parameter nbIterationsPerPolicyEvaluation > 1
         * \param[in] generationNumber the integer value to indicate the
         * current generation number
         */
        virtual void reset(size_t seed = 0,
                           LearningMode mode = LearningMode::TRAINING,
                           uint16_t iterationNumber = 0,
                           uint64_t generationNumber = 0) = 0;

        /**
         * \brief Method for checking if the LearningEnvironment has reached a
         * terminal state.
         *
         * The boolean value returned by this method, when equal to true,
         * indicates that the LearningEnvironment has reached a terminal state.
         * A terminal state is a state in which further calls to the doAction
         * method will have no effects on the dataSources of the
         * LearningEnvironment, or on its score. For example, this terminal
         * state may be reached for a Game Over state within a game, or in case
         * the objective of the learning agent has been successfuly reached.
         *
         * \return a boolean indicating termination.
         */
        virtual bool isTerminal() const = 0;
    };
}; // namespace Learn

#endif
