/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2021) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2021)
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

#ifndef LEARNING_ENVIRONMENT_H
#define LEARNING_ENVIRONMENT_H

#include "data/dataHandler.h"
#include <cstdint>
#include <vector>

namespace Learn {

    /**
     * \brief Different modes in which the LearningEnvironment can be reset.
     *
     * Each of the following mode corresponds to a classical phase of a learning
     * process. These mode usually refer to different parts of the data set used
     * throughout the learning process. Classically, the TRAINING mode is used
     * to effectively train an agent. The VALIDATION mode is used to evaluate
     * the efficiency of the learning process during the training phase, but on
     * data differring from the one used for training, in order to avoid biased
     * evaluation. TESTING mode is used at the end of all training activity to
     * evaluate the efficiency of the agent on completely new data.
     */
    enum class LearningMode
    {
        TRAINING,
        VALIDATION,
        TESTING
    };

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
    class LearningEnvironment
    {
      protected:
        /// Number of actions available for interacting with this
        /// LearningEnvironment
        const uint64_t nbActions;

        /// Make the default copy constructor protected.
        LearningEnvironment(const LearningEnvironment& other) = default;

      public:
        /**
         * \brief Delete the default constructor of a LearningEnvironment.
         */
        LearningEnvironment() = delete;

        /// Default virtual destructor
        virtual ~LearningEnvironment() = default;

        /**
         * \brief Constructor for LearningEnviroment.
         *
         * \param[in] nbAct number of actions that will be usable for
         * interacting with this LearningEnviromnent.
         */
        LearningEnvironment(uint64_t nbAct) : nbActions{nbAct} {};

        /**
         * \brief Get a copy of the LearningEnvironment.
         *
         * Default implementation returns a null pointer.
         *
         * \return a copy of the LearningEnvironment if it is copyable,
         * otherwise this method returns a NULL pointer.
         */
        virtual LearningEnvironment* clone() const;

        /**
         * \brief Can the LearningEnvironment be copy constructed to evaluate
         * several LearningAgent in parallel.
         *
         * \return true if the LearningEnvironment can be copied and run in
         * parallel. Default implementation returns false.
         */
        virtual bool isCopyable() const;

        /**
         * \brief Get the number of actions available for this
         * LearningEnvironment.
         *
         * \return the integer value of the nbAction attribute.
         */
        uint64_t getNbActions() const
        {
            return this->nbActions;
        };

        /**
         * \brief Execute an action on the LearningEnvironment.
         *
         * The purpose of this method is to execute an action, represented by
         * an actionId comprised between 0 and nbActions - 1.
         * The LearningEnvironment implementation only checks that the given
         * actionID is comprised between 0 and nbActions - 1.
         * It is the responsibility of this method to call the updateHash
         * method on dataSources whose content have been affected by the action.
         *
         * \param[in] actionID the integer number representing the action to
         * execute.
         * \throw std::runtime_error if the actionID exceeds nbActions - 1.
         */
        virtual void doAction(uint64_t actionID);

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
         */
        virtual void reset(size_t seed = 0,
                           LearningMode mode = LearningMode::TRAINING) = 0;

        /**
         * \brief Get the data sources for this LearningEnvironment.
         *
         * This method returns a vector of reference to the DataHandler that
         * will be given to the learningAgent, and to its Program to learn how
         * to interact with the LearningEnvironment. Throughout the existence
         * of the LearningEnvironment, data contained in the data will be
         * modified, but never the number, nature or size of the dataHandlers.
         * Since this methods return references to the DataHandler, the
         * learningAgent will assume that the referenced dataHandler are
         * automatically updated each time the doAction, or reset methods
         * are called on the LearningEnvironment.
         *
         * \return a vector of references to the DataHandler.
         */
        virtual std::vector<std::reference_wrapper<const Data::DataHandler>>
        getDataSources() = 0;

        /**
         * \brief Returns the current score of the Environment.
         *
         * The returned score will be used as a reward during the learning
         * phase of a LearningAgent.
         *
         * \return the current score for the LearningEnvironment.
         */
        virtual double getScore() const = 0;

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
