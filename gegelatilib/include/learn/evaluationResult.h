#ifndef EVALUATION_RESULT_H
#define EVALUATION_RESULT_H

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
			EvaluationResult(const double& res) : result{res} {};
			
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


	/**
	* \brief Comparison function to enable sorting of EvaluationResult with STL.
	*/
	bool operator==(const EvaluationResult& a, const EvaluationResult& b);
}

#endif