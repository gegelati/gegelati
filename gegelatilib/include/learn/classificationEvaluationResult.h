/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019)
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

		/**
		* \brief Vector storing the number of evaluation for each class
		* of the classification LearningEnvironment.
		*/
		std::vector<size_t> nbEvaluationPerClass;

	public:
		/**
		* \brief Main constructor of the ClassificationEvaluationResult class.
		*
		* A ClassificationEvaluationResult storing a score for each class of a
		* classification-oriented LearningEnvironment.
		*
		* Contrary to the base class EvaluationResult, the number of evaluation
		* stored in a ClassificationEvaluationResult corresponds to the total
		* number of times any action was performed.
		*
		* \param[in] scores a vector of double storing per-class scores.
		* \param[in] nbEvalPerClass a vector of integer storing per-class
		* number of evaluations.
		*/
		ClassificationEvaluationResult(const std::vector<double>& scores, const std::vector<size_t>& nbEvalPerClass) :
			EvaluationResult(std::accumulate(scores.cbegin(), scores.cend(), 0.0) / scores.size(),
				std::accumulate(nbEvalPerClass.cbegin(), nbEvalPerClass.cend(), size_t(0))),
			scorePerClass(scores), nbEvaluationPerClass(nbEvalPerClass) {
			if (scores.size() != nbEvalPerClass.size()) {
				throw std::runtime_error("Mismatch between scores and nbEvalPerClass vector sizes.");
			}
		};

		/**
		* \brief Get a const ref to the scorePerClass attribute.
		*/
		const std::vector<double>& getScorePerClass() const;

		/**
		* \brief Get a const ref to the nbEvaluationPerClass attribute.
		*/
		const std::vector<size_t>& getNbEvaluationPerClass() const;

		/**
		* \brief Override from EvaluationResult
		*
		* \throw std::runtime_error in case the number of classes of the two
		* ClassificationEvaluationResult are different.
		*/
		virtual EvaluationResult& operator+=(const EvaluationResult& other) override;
	};
};

#endif
