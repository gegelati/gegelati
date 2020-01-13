#include "learn/evaluationResult.h"

double Learn::EvaluationResult::getResult() const {
	return this->result;
}

bool Learn::operator<(const EvaluationResult& a, const EvaluationResult& b)
{
	return a.getResult() < b.getResult();
}
