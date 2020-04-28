/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
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

#ifndef EVALUATION_RESULT_H
#define EVALUATION_RESULT_H

#include <memory>

namespace Learn {
	/**
	* \brief Base class for storing all result of a policy evaluation within a
	* LearningEnvironment.
	*
	* To enable generic learning with the default LearningAgent, all policy
	* evaluation must be convertible to a simple double value. For more complex
	* LearningAgent behavior, like for classification purposes, specific child
	* class can be created.
	*/
	class EvaluationResult {
	protected:
		/// Double value for the result.
		double result;

	public:
		/**
		* \brief Deleted default constructor.
		*/
		EvaluationResult() = delete;

		/**
		* \brief Virtual destructor for polymorphism.
		*/
		virtual ~EvaluationResult() = default;

		/**
		* \brief Construct a result from a simple double value.
		*
		* \param[in] res the double value representing the result of an
		* evaluation.
		*/
		EvaluationResult(const double& res) : result{ res } {};

		/**
		* \brief Virtual method to get the default double equivalent of
		* the EvaluationResult.
		*/
		virtual double getResult() const;
	};

	/**
	* \brief Comparison function to enable sorting of EvaluationResult with STL.
	*/
	bool operator<(const EvaluationResult& a, const EvaluationResult& b);
}

namespace std {
	/// Struct specialization for EvaluationResult
	template<>
	struct less<std::shared_ptr<Learn::EvaluationResult>>
	{
		/// Comparison operator for sorted containers.
		bool operator()(const std::shared_ptr<Learn::EvaluationResult>& a, const std::shared_ptr<Learn::EvaluationResult>& b) const
		{
			return *a < *b;
		}
	};
};

#endif
