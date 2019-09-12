#include <gtest/gtest.h>

#include "instructions/addPrimitiveType.h"

#include "learn/learningEnvironment.h"
#include "learn/stickGameWithOpponent.h"

#include "learn/learningAgent.h"

class LearningAgentTest : public ::testing::Test {
protected:
	Instructions::Set set;
	StickGameWithOpponent le;
	Mutator::MutationParameters params;

	virtual void SetUp() {
		set.add(*(new Instructions::AddPrimitiveType<int>()));
		set.add(*(new Instructions::AddPrimitiveType<double>()));

		// Proba as in Kelly's paper
		params.tpg.maxInitOutgoingEdges = 3;
		params.prog.maxProgramSize = 96;
		params.tpg.nbRoots = 7;
		params.tpg.pEdgeDeletion = 0.7;
		params.tpg.pEdgeAddition = 0.7;
		params.tpg.pProgramMutation = 0.2;
		params.tpg.pEdgeDestinationChange = 0.1;
		params.tpg.pEdgeDestinationIsAction = 0.5;
		params.prog.pAdd = 0.5;
		params.prog.pDelete = 0.5;
		params.prog.pMutate = 1.0;
		params.prog.pSwap = 1.0;
	}

	virtual void TearDown() {
		delete (&set.getInstruction(0));
		delete (&set.getInstruction(1));
	}
};

TEST_F(LearningAgentTest, Constructor) {
	Learn::LearningAgent* la;

	ASSERT_NO_THROW(la = new Learn::LearningAgent(le, set, params)) << "Construction of the learningAgent failed.";

	ASSERT_NO_THROW(delete la) << "Destruction of the LearningAgent failed.";
}

TEST_F(LearningAgentTest, Init) {
	Learn::LearningAgent la(le, set, params);

	ASSERT_NO_THROW(la.init()) << "Initialization of the LearningAgent should not fail.";
}

TEST_F(LearningAgentTest, EvalRoot) {
	Learn::LearningAgent la(le, set, params);

	la.init();
	std::pair<double, double> result;
	ASSERT_NO_THROW(result = la.evaluateRoot(*la.getTPGGraph().getRootVertices().at(0), 0, 10, 11)) << "Evaluation from a root failed.";
	ASSERT_LE(result.second, 11) << "Average number of actions should not exceed the given limit.";
}