#include <gtest/gtest.h>

#include "learn/learningEnvironment.h"
#include "learn/stickGameWithOpponent.h"
#include "learn/classificationLearningEnvironment.h"
#include "learn/fakeClassificationLearningEnvironment.h"


TEST(ClassificationLearningEnvironmentTest, Constructor) {
	Learn::ClassificationLearningEnvironment* le = NULL;

	ASSERT_NO_THROW(le = new FakeClassificationLearningEnvironment()) << "Construction of the classification Learning Environment failed";

	ASSERT_NO_THROW(delete le) << "Destruction of the Learning Environment failed";
}

TEST(ClassificationLearningEnvironmentTest, resetDoActionGetClassificationTableGetScore) {
	FakeClassificationLearningEnvironment fle;

	auto actions = { 0, 2, 2, 1, 0, 1, 2, 0, 0, 1, 1, 1, 2, 2, 2, 1, 1, 2 };

	// reset
	ASSERT_NO_THROW(fle.reset(0, Learn::LearningMode::TRAINING)) << "resetting the ClassificationLearningEnvironment failed";

	// doActions
	for (auto action : actions) {
		ASSERT_NO_THROW(fle.doAction(action)) << "doAction failed within a known ClassificationLearningEnvironment.";
	}

	// getClassif table
	const std::vector<std::vector<uint64_t>>* classifTable = NULL;
	ASSERT_NO_THROW(classifTable = &fle.getClassificationTable()) << "Getting the classificationTable failed.";

	// check its content with known actions
	int table[3][3] = {
		{1, 3, 2},
		{2, 2, 2},
		{1, 2, 3}
	};

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			ASSERT_EQ(classifTable->at(i).at(j), table[i][j]) << "Classification table contains unexpected values with known actions.";
		}
	}

	// Get average f1 score
	ASSERT_NEAR(fle.getScore(), 0.323077, 0.000001) << "Score of the ClassificationLearningEnvironment is not as expected with known actions.";
}
