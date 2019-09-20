#ifndef LEARNING_ENVIRONMENT_H
#define LEARNING_ENVIRONMENT_H

#include <cstdint>
#include <vector>
#include "dataHandlers/dataHandler.h"

namespace Learn {
	/**
	* \brief Interface for creating a Learning Environment.
	*
	* This class defines all the method that should be implemented for a Learner
	* to interact with an learning environment and learn to interact with it.
	*
	* Interaction with a learning environment are made through a discrete set
	* of actions. As a result of these actions, the learning environment may
	* update its state, accessible through the data sources it provides. The
	* learning environment also provides a score resulting from the past
	* actions, and a termination boolean indicating that the
	* learningEnvironment has reached a final state, that no action will
	* affect.
	*/
	class LearningEnvironment {
		/// Number of actions available for interacting with this 
		/// LearningEnvironment
		const uint64_t nbActions;
	public:
		/**
		* \brief Delete the default constructor of a LearningEnvironment.
		*/
		LearningEnvironment() = delete;

		/**
		* \brief Constructor for LearningEnviroment.
		*
		* \param[in] nbAct number of actions that will be usable for
		* interacting with this LearningEnviromnent.
		*/
		LearningEnvironment(uint64_t nbAct) : nbActions{ nbAct } {};

		/**
		* \brief Get the number of actions available for this
		* LearningEnvironment.
		*
		* \return the integer value of the nbAction attribute.
		*/
		uint64_t getNbActions() const { return this->nbActions; };

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
		* - for comparing the performance of several agents with the same random
		* starting conditions.
		* - for training each agent with diverse starting conditions.
		*
		* \param[in] seed the integer value for controlling the randomness of
		* the LearningEnvironment.
		*/
		virtual void reset(size_t seed = 0) = 0;

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
		virtual std::vector<std::reference_wrapper<DataHandlers::DataHandler>> getDataSources() = 0;

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
};

#endif