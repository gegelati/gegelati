#include "learn/classificationEvaluationResult.h"

const std::vector<double>& Learn::ClassificationEvaluationResult::getScorePerClass() const {
	return this->scorePerClass;
}