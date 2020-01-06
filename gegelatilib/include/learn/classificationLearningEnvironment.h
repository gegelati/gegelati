#ifndef CLASSIFICATION_LEARNING_ENVIRONMENT_H
#define CLASSIFICATION_LEARNING_ENVIRONMENT_H

#include <vector>

#include "learn/learningEnvironment.h"

namespace Learn {
	/**
	* \brief Specialization of the LearningEnvironment class for classification
	* purposes.
	*/
	class ClassificationLearningEnvironment : public LearningEnvironment {
	protected:
		/**
		* \brief 2D array storing for each class the guesses that were made by the
		* LearningAgent.
		*
		* For example classificationTable.at(x).at(y) represents the number of times
		* a LearningAgent guessed class y, for a data from class x since the last
		* reset.
		*/
		std::vector < std::vector< uint64_t >> classificationTable;

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
		ClassificationLearningEnvironment(uint64_t nbClass) :
			LearningEnvironment(nbClass),
			currentClass{ 0 },
			classificationTable(nbClass, std::vector<uint64_t>(nbClass, 0)) {};

		/**
		* \brief Get a const ref to the classification table of the learning environment.
		*/
		const std::vector<std::vector<uint64_t>>& getClassificationTable() const;

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
		virtual void reset(size_t seed = 0, LearningMode mode = TRAINING) override = 0;
	};
};

#endif 