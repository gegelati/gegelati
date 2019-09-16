#include <gtest/gtest.h>

#include "learn/learningEnvironment.h"
#include "learn/stickGameWithOpponent.h"

TEST(LearningEnvironmentTest, Constructor) {
	Learn::LearningEnvironment* le = NULL;

	ASSERT_NO_THROW(le = new StickGameWithOpponent()) << "Construction of the Learning Environment failed";

	ASSERT_NO_THROW(delete le) << "Destruction of the Learning Environment failed";
}

TEST(LearningEnvironmentTest, getNbAction) {
	StickGameWithOpponent le;

	ASSERT_EQ(le.getNbActions(), 3) << "Number of action is incorrect";
}

TEST(LearningEnvironmentTest, getDataSource) {
	StickGameWithOpponent le;

	std::vector<std::reference_wrapper<DataHandlers::DataHandler>> dataSrc;
	ASSERT_NO_THROW(dataSrc = le.getDataSources()) << "Getting data sources should not fail";
	ASSERT_EQ(dataSrc.size(), 2) << "Number of dataSource is incorrect";

	// Check initial number of sticks
	int initNr = (const int)(const PrimitiveType<int>&)dataSrc.at(1).get().getDataAt(typeid(PrimitiveType<int>), 0);
	ASSERT_EQ(initNr, 21) << "Initial number of stick is incorrect";
}

TEST(LearningEnvironmentTest, doAction) {
	StickGameWithOpponent le;

	ASSERT_NO_THROW(le.doAction(1)) << "Remove 2 stick after game init should not fail.";
	const PrimitiveType<int>& nbSticks = (const PrimitiveType<int>&)le.getDataSources().at(1).get().getDataAt(typeid(PrimitiveType<int>), 0);
	// Remove 2 sticks brings us to 19 sticks
	// Other player removes between 1 and 3 sticks
	// thus, number of remaining sticks is within 18 and 16
	ASSERT_TRUE(nbSticks <= 18 && nbSticks >= 16) << "Number of stick remaining after one action is not within expected range.";

	// Check the illegal action
	ASSERT_THROW(le.doAction(3), std::runtime_error) << "Illegal action not detected as such.";
}

TEST(LearningEnvironmentTest, getScoreAndIsTerminal) {
	StickGameWithOpponent le;

	ASSERT_EQ(le.getScore(), 0.0) << "Score should be zero until the game is over";
	const PrimitiveType<int>& nbSticks = (const PrimitiveType<int>&)le.getDataSources().at(1).get().getDataAt(typeid(PrimitiveType<int>), 0);

	// Play the full game and lose with known seed (0)
	std::vector<int> actions = { 0, 1, 2, 2, 2, 0 };
	for (auto& action : actions) {
		ASSERT_FALSE(le.isTerminal()) << "With a known seed and action sequence, the game should not be over.";
		le.doAction(action);
	}

	ASSERT_TRUE(le.isTerminal()) << "With a known seed and action sequence, the game should be over.";
	ASSERT_EQ(le.getScore(), 0.0) << "Score when losing the game should be 0.";

	le.reset(0);
	actions = { 0, 1, 2, 2, 2, 1 };
	for (auto& action : actions) {
		ASSERT_FALSE(le.isTerminal()) << "With a known seed and action sequence, the game should not be over.";
		le.doAction(action);
	}
	ASSERT_TRUE(le.isTerminal()) << "With a known seed and action sequence, the game should be over.";
	ASSERT_EQ(le.getScore(), -1.0) << "Score when losing the game with an illegal action should be -1.0.";

	le.reset(0);
	actions = { 0, 1, 2 , 2, 0, 0 };
	for (auto action : actions) {
		ASSERT_FALSE(le.isTerminal()) << "With a known seed and action sequence, the game should not be over.";
		le.doAction(action);
	}
	ASSERT_TRUE(le.isTerminal()) << "With a known seed and action sequence, the game should be over.";
	ASSERT_EQ(le.getScore(), 1.0) << "Score when winning the game should be 1.0.";
}