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


#include "data/dataValue.h"
#include "dimensions/requirement.h"

namespace Evaluation {

    /**
     * \brief Interface of a reinforcement learning environment, in which the individuals act (see ReinforcementProblem).
     *
     * This class defines all the methods that should be implemented for an
     * individual to interact with the environment.
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

        /// @brief DataSources used as input space of the environment
        std::vector<Data::DataValue> dataSources;

        /// Make the default copy constructor protected.
        ReinforcementEnvironment(const ReinforcementEnvironment& other) = default;


      public:
        /**
         * \brief Constructor for ReinforcementEnvironment.
         *
         * \param[in] inputDimensions the dimensions of the input sources.
         * \param[in] outputDimension the dimensions of the output source.
         * \param[in] maxSteps Maximum number of steps doable in a single episode.
         */
        ReinforcementEnvironment(const std::vector<Dimensions::Requirement>& inputDimensions, const Dimensions::Requirement& outputDimension, uint64_t maxSteps = UINT64_MAX)
            : inputDimensions(inputDimensions), outputDimension(outputDimension), maxSteps{maxSteps} {};

        /**
         * \brief Get a copy of the environment.
         *
         * Default implementation returns a null pointer.
         *
         * \return a copy of the environment if it is copyable,
         * otherwise this method returns a NULL pointer.
         */
        virtual std::shared_ptr<Evaluation::ReinforcementEnvironment> cloneSharedPtr() const;

        /**
         * \brief Can the environment be copy constructed to evaluate
         * several individuals in parallel.
         *
         * \return true if the environment can be copied and run in
         * parallel. Default implementation returns false.
         */
        virtual bool isCopyable() const;


        /**
         * \brief get the input dimensions of the environment.
         */
        virtual const std::vector<Dimensions::Requirement>& getInputDimensions() const;

        /**
         * \brief get the output dimension of the environment.
         */
        virtual const Dimensions::Requirement& getOutputDimension() const;

        /**
         * \brief get the maximum number of steps doable in a single episode.
         */
        virtual uint64_t getMaxSteps() const;

        /**
         * \brief Get the data sources of the environment.
         *
         * The dataSources should be updated during step or reset method.
         */
        virtual const std::vector<Data::DataValue>& getDataSources() const;

        /**
         * \brief Execute an action in the environment.
         *
         * \param[in] action the view representing the action to
         * execute.
         * \throw std::runtime_error if the action does not correspond to the output dimension.
         */
        virtual void doAction(const Data::DataValue& action);

        /**
         * \brief Reset the environment, to start a new episode.
         *
         * Optionally seed can be given to this function to control the
         * randomness of the environment (if any). When available, this
         * feature will be used:
         * - for comparing the performance of several individuals with the same
         * random starting conditions.
         * - for training each individual with diverse starting conditions.
         *
         * \param[in] seed the integer value for controlling the randomness of
         * the environment.
         */
        virtual void reset(size_t seed = 0) = 0;

        /**
         * \brief Method for checking if the environment has reached a
         * terminal state.
         *
         * The boolean value returned by this method, when equal to true,
         * indicates that the environment has reached a terminal state.
         * A terminal state is a state in which further calls to the doAction
         * method will have no effects on the dataSources of the
         * environment, or on its rewards. For example, this terminal
         * state may be reached for a Game Over state within a game, or in case
         * the objective of the individual has been successfully reached.
         *
         * \return a boolean indicating termination.
         */
        virtual bool isTerminal() const = 0;

        /**
         * \brief Returns the reward of the last action.
         *
         * The rewards are summed during an evaluation (see Metrics::RewardsMetric).
         *
         * \return the reward of the last action.
         */
        virtual double getLastReward() const = 0;
    };
}; // namespace Evaluation

#endif
