#include "learn/classificationLearningEnvironment.h"

void Learn::ClassificationLearningEnvironment::doAction(uint64_t actionID) {
	// Base method
	LearningEnvironment::doAction(actionID);

	// Classification table update
	this->classificationTable.at(this->currentClass).at(actionID)++;
}

const std::vector<std::vector<uint64_t>>& Learn::ClassificationLearningEnvironment::getClassificationTable() const
{
	return this->classificationTable;
}

double Learn::ClassificationLearningEnvironment::getScore() const
{

	uint64_t nbTotalGuess = 0;
	uint64_t nbCorrectGuess = 0;

	// Scan the classification table to compute the double score
	for (auto classIdx = 0; classIdx < this->classificationTable.size(); classIdx++) {
		for (auto guessedClass = 0; guessedClass < this->classificationTable.size(); guessedClass++) {
			nbTotalGuess += this->classificationTable.at(classIdx).at(guessedClass);
			nbCorrectGuess += (classIdx == guessedClass) ? this->classificationTable.at(classIdx).at(guessedClass) : 0;
		}
	}

	return (double)nbCorrectGuess / (double)nbTotalGuess;
}

void Learn::ClassificationLearningEnvironment::reset(size_t seed, LearningMode mode)
{
	// reset scores to 0 in classification table
	for (std::vector<uint64_t>& perClass : this->classificationTable) {
		for (uint64_t score : perClass) {
			score = 0;
		}
	}
}
