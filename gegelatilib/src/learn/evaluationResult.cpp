#include "learn/evaluationResult.h"

double Learn::EvaluationResult::getResult() const {
	return this->result;
}

bool Learn::operator<(const EvaluationResult& a, const EvaluationResult& b)
{
	return a.getResult() < b.getResult();
}

bool Learn::operator<(const std::shared_ptr<EvaluationResult>& a, const std::shared_ptr<EvaluationResult>& b)
{
	return *a < *b;
}

bool Learn::operator==(const EvaluationResult& a, const EvaluationResult& b)
{
	return a.getResult() == b.getResult();
}
