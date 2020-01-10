#include <numeric>

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
	// Compute the average f1 score over all classes
	// (chosen instead of the global f1 score as it gives an equal weight to 
	// the f1 score of each class, no matter its ratio within the observed
	// population)
	double averageF1Score = 0.0;

	// for each class
	for (uint64_t classIdx = 0; classIdx < classificationTable.size(); classIdx++) {
		uint64_t truePositive = classificationTable.at(classIdx).at(classIdx);
		uint64_t falseNegative = std::accumulate(classificationTable.at(classIdx).begin(), classificationTable.at(classIdx).end(), (uint64_t)0) - truePositive;
		uint64_t falsePositive = std::transform_reduce(classificationTable.begin(), classificationTable.end(), (uint64_t)0, std::plus<>(),
			[&classIdx](const std::vector<uint64_t>& classifForClass) {return classifForClass.at(classIdx); }) - truePositive;

		double recall = (double)truePositive / (double)(truePositive + falseNegative);
		double precision = (double)truePositive / (double)(truePositive + falsePositive);
		// If true positive is 0, set score to 0.
		double fScore = (truePositive != 0) ? 2 * (precision * recall) / (precision + recall) : 0.0;
		averageF1Score += fScore;
	}

	averageF1Score /= this->classificationTable.size();

	return averageF1Score;
}

void Learn::ClassificationLearningEnvironment::reset(size_t seed, LearningMode mode)
{
	// reset scores to 0 in classification table
	for (std::vector<uint64_t>& perClass : this->classificationTable) {
		for (uint64_t& score : perClass) {
			score = 0;
		}
	}
}
