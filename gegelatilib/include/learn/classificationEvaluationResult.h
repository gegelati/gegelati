#ifndef CLASSIFICATION_EVALUATION_RESULT_H
#define CLASSIFICATION_EVALUATION_RESULT_H

#include <vector>
#include <numeric>

#include "learn/evaluationResult.h"

namespace Learn {
	/**
	* \brief Specialization of the EvaluationResult class for classification
	* LearningEnvironment.
	*
	* The main difference with the base EvaluationResult class is that a score
	* per Action of a learning environment can be stored within this class.
	*/
	class ClassificationEvaluationResult : public EvaluationResult {
	protected:
		/**
		* \brief Vector storing a double score per class (i.e. per Action) of
		* a classification LearningEnvironment.
		*/
		std::vector<double> scorePerClass;

	public:
		/**
		* \brief Main constructor of the ClassificationEvaluationResult class.
		*
		* A ClassificationEvaluationResult storing a score for each class of a
		* classification-oriented LearningEnvironment.
		*
		* \param[in] scores a vector of double storing per-class scores.
		*/
		ClassificationEvaluationResult(const std::vector<double>& scores) :
			EvaluationResult(std::accumulate(scores.cbegin(), scores.cend(), 0.0) / scores.size()),
			scorePerClass(scores) {};
	};
};

#endif
