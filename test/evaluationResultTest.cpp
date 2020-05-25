#include <gtest/gtest.h>
#include <stdexcept>

#include "learn/evaluationResult.h"
#include "learn/classificationEvaluationResult.h"

TEST(EvaluationResultTest, Constructor) {
	Learn::EvaluationResult* eval;

	ASSERT_NO_THROW(eval = new Learn::EvaluationResult(1.0, 50)) << "Building an EvaluationResult failed unexpectedly.";

	ASSERT_NO_THROW(delete eval);
}

TEST(EvaluationResultTest, GetResult) {
	Learn::EvaluationResult eval(1.0, 10);

	ASSERT_EQ(eval.getResult(), 1.0) << "Getter returned an unexpected value.";
}

TEST(EvaluationResultTest, GetNbEvaluation) {
	Learn::EvaluationResult eval(1.0, 10);

	ASSERT_EQ(eval.getNbEvaluation(), 10) << "Getter returned an unexpected value.";
}

TEST(EvaluationResultTest, AssignmentAdditionOperator) {
	Learn::EvaluationResult eval1(1.0, 10);
	Learn::EvaluationResult eval2(2.0, 20);

	ASSERT_NO_THROW(eval1 += eval2) << "Call to operator+= failed unexpectedly.";

	ASSERT_EQ(eval1.getResult(), (10 * 1.0 + 20 * 2.0) / (10.0 + 20.0)) << "Getter returned an unexpected value after call to operator+=.";
	ASSERT_EQ(eval2.getResult(), 2.0) << "Getter returned an unexpected value after call to operator+=.";
	ASSERT_EQ(eval1.getNbEvaluation(), 20 + 10) << "Getter returned an unexpected value after call to operator+=.";
	ASSERT_EQ(eval2.getNbEvaluation(), 20) << "Getter returned an unexpected value after call to operator+=.";

	Learn::ClassificationEvaluationResult eval3({ 3.0, 4.0 }, { 2, 3 });
	ASSERT_THROW(eval1 += eval3, std::runtime_error) << "Call to operator += should not work with heterogeneous EvaluationResult classes.";
}

TEST(ClassificationEvaluationResultTest, Constructor) {
	Learn::EvaluationResult* eval;

	ASSERT_NO_THROW(eval = new Learn::ClassificationEvaluationResult({ 1.0, 2.0 }, { 2, 3 })) << "Building a ClassificationEvaluationResult failed unexpectedly.";

	ASSERT_NO_THROW(delete eval);

	ASSERT_THROW(eval = new Learn::ClassificationEvaluationResult({ 1.0, 2.0 }, { 2, 3, 5 }), std::runtime_error) << "Building a ClassificationEvaluationResult with vectors of different sizes should fail.";
}

TEST(ClassificationEvaluationResultTest, GetResult) {
	Learn::ClassificationEvaluationResult eval({ 1.0, 2.0 }, { 2, 3 });

	ASSERT_EQ(eval.getResult(), (1.0 + 2.0) / 2.0) << "Getter returned an unexpected value.";
}

TEST(ClassificationEvaluationResultTest, GetNbEvaluation) {
	Learn::ClassificationEvaluationResult eval({ 1.0, 2.0 }, { 2, 3 });

	ASSERT_EQ(eval.getNbEvaluation(), 5) << "Getter returned an unexpected value.";
}

TEST(ClassificationEvaluationResultTest, GetScorePerClass) {
	Learn::ClassificationEvaluationResult eval({ 1.0, 2.0 }, { 2, 3 });

	ASSERT_EQ(eval.getScorePerClass().size(), 2) << "Getter returned an unexpected value.";
	ASSERT_EQ(eval.getScorePerClass().at(0), 1.0) << "Getter returned an unexpected value.";
	ASSERT_EQ(eval.getScorePerClass().at(1), 2.0) << "Getter returned an unexpected value.";
}

TEST(ClassificationEvaluationResultTest, GetNbEvaluationPerClass) {
	Learn::ClassificationEvaluationResult eval({ 1.0, 2.0 }, { 2, 3 });

	ASSERT_EQ(eval.getScorePerClass().size(), 2) << "Getter returned an unexpected value.";
	ASSERT_EQ(eval.getNbEvaluationPerClass().at(0), 2) << "Getter returned an unexpected value.";
	ASSERT_EQ(eval.getNbEvaluationPerClass().at(1), 3) << "Getter returned an unexpected value.";
}

TEST(ClassificationEvaluationResultTest, AssignmentAdditionOperator) {
	Learn::ClassificationEvaluationResult eval1({ 1.0, 2.0 }, { 2, 3 });
	Learn::ClassificationEvaluationResult eval2({ 2.0, 3.0 }, { 2, 2 });

	ASSERT_NO_THROW(eval1 += eval2) << "Call to operator+= failed unexpectedly.";

	ASSERT_EQ(eval1.getScorePerClass().at(0), 1.5) << "Getter returned an unexpected value after call to operator+=.";
	ASSERT_EQ(eval1.getScorePerClass().at(1), 2.4) << "Getter returned an unexpected value after call to operator+=.";

	ASSERT_EQ(eval1.getNbEvaluationPerClass().at(0), 4) << "Getter returned an unexpected value after call to operator+=.";
	ASSERT_EQ(eval1.getNbEvaluationPerClass().at(1), 5) << "Getter returned an unexpected value after call to operator+=.";

	ASSERT_EQ(eval1.getResult(), 1.95) << "Getter returned an unexpected value after call to operator+=.";
	ASSERT_EQ(eval1.getNbEvaluation(), 9) << "Getter returned an unexpected value after call to operator+=.";

	Learn::ClassificationEvaluationResult eval3({ 3.0, 4.0, 5.0 }, { 2, 3, 4 });
	ASSERT_THROW(eval1 += eval3, std::runtime_error) << "Call to operator += should not work with incompatible vector size.";
}

