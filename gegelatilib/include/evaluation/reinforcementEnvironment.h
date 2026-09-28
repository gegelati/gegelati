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

#ifndef RREINFORCEMENT_ENVIRONMENT_H
#define RREINFORCEMENT_ENVIRONMENT_H


#include "data/dataValue.h"
#include "dimensions/requirement.h"

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
     * problem has reached a final state, that no action will
     * affect.
     */
    class ReinforcementEnvironment
    {
      protected:

        /// Input dimensions
        std::vector<Dimensions::Requirement> inputDimensions;

        /// Output dimension
        Dimensions::Requirement outputDimension;

        /// @brief Maximum number of steps doable in a single episode.
        uint64_t maxSteps;

        /// Make the default copy constructor protected.
        ReinforcementEnvironment(const ReinforcementEnvironment& other) = default;


      public:
        /**
         * \brief Constructor for LearningEnviroment.
         *
         * \param[in] inputDimensions the dimensions of the input sources.
         * \param[in] outputDimension the dimensions of the output source.
         * \param[in] maxSteps Maximum number of steps doable in a single episode.
         */
        ReinforcementEnvironment(const std::vector<Dimensions::Requirement>& inputDimensions, const Dimensions::Requirement& outputDimension, uint64_t maxSteps = UINT64_MAX)
            : inputDimensions(inputDimensions), outputDimension(outputDimension), maxSteps{maxSteps} {};

        /**
         * \brief Get a copy of the Problem.
         *
         * Default implementation returns a null pointer.
         *
         * \return a copy of the Problem if it is copyable,
         * otherwise this method returns a NULL pointer.
         */
        virtual std::unique_ptr<Evaluation::ReinforcementEnvironment> cloneUniquePtr() const;

        /**
         * \brief Can the Problem be copy constructed to evaluate
         * several LearningAgent in parallel.
         *
         * \return true if the Problem can be copied and run in
         * parallel. Default implementation returns false.
         */
        virtual bool isCopyable() const;


        /**
         * \brief get the input dimensions of the Problem.
         */
        virtual const std::vector<Dimensions::Requirement>& getInputDimensions() const;

        /**
         * \brief get the output dimension of the Problem.
         */
        virtual const Dimensions::Requirement& getOutputDimension() const;

        /**
         * \brief get the maximum number of steps doable in a single episode.
         */
        virtual uint64_t getMaxSteps() const;

        /**
         * \brief Get the data sources for this Problem.
         *
         * This method returns a vector of reference to the DataHandler that
         * will be given to the LearningAgent, and to its Program to learn how
         * to interact with the Problem. Throughout the existence
         * of the Problem, data contained in the data will be
         * modified, but never the number, nature or size of the dataHandlers.
         * Since this methods return references to the DataHandler, the
         * LearningAgent will assume that the referenced dataHandler are
         * automatically updated each time the doAction, or reset methods
         * are called on the Problem.
         *
         * \return a vector of references to the DataHandler.
         */
        virtual std::vector<Data::DataView>  getDataSources() const = 0;

        /**
         * \brief Execute an action on the Problem.
         *
         * \param[in] action the view representing the action to
         * execute.
         * \throw std::runtime_error if the action does not correspond to the output dimension.
         */
        virtual void doAction(const Data::DataValue& action);

        /**
         * \brief Reset the Problem.
         *
         * Resetting a learning environment is needed to train an agent.
         * Optionally seed can be given to this function to control the
         * randomness of a Problem (if any). When available, this
         * feature will be used:
         * - for comparing the performance of several agents with the same
         * random starting conditions.
         * - for training each agent with diverse starting conditions.
         *
         * \param[in] seed the integer value for controlling the randomness of
         * the Problem.
         */
        virtual void reset(size_t seed = 0) = 0;

        /**
         * \brief Method for checking if the Problem has reached a
         * terminal state.
         *
         * The boolean value returned by this method, when equal to true,
         * indicates that the Problem has reached a terminal state.
         * A terminal state is a state in which further calls to the doAction
         * method will have no effects on the dataSources of the
         * Problem, or on its score. For example, this terminal
         * state may be reached for a Game Over state within a game, or in case
         * the objective of the learning agent has been successfuly reached.
         *
         * \return a boolean indicating termination.
         */
        virtual bool isTerminal() const = 0;

        /**
         * \brief Returns the current score of the Environment.
         *
         * The returned score will be used as a reward during the learning
         * phase of a LearningAgent.
         *
         * \return the current score for the Problem.
         */
        virtual double getLastReward() const = 0;
    };
}; // namespace Learn

#endif
