#include <gtest/gtest.h>
#include <numeric>

#include "mutator/tpgMutator.h"
#include "instructions/addPrimitiveType.h"

#include "learn/learningEnvironment.h"
#include "learn/stickGameWithOpponent.h"
#include "learn/learningParameters.h"
#include "learn/learningAgent.h"

class LearningAgentTest : public ::testing::Test {
protected:
	Instructions::Set set;
	StickGameWithOpponent le;
	Learn::LearningParameters params;

	virtual void SetUp() {
		set.add(*(new Instructions::AddPrimitiveType<int>()));
		set.add(*(new Instructions::AddPrimitiveType<double>()));

		// Proba as in Kelly's paper
		params.mutation.tpg.maxInitOutgoingEdges = 3;
		params.mutation.prog.maxProgramSize = 96;
		params.mutation.tpg.nbRoots = 15;
		params.mutation.tpg.pEdgeDeletion = 0.7;
		params.mutation.tpg.pEdgeAddition = 0.7;
		params.mutation.tpg.pProgramMutation = 0.2;
		params.mutation.tpg.pEdgeDestinationChange = 0.1;
		params.mutation.tpg.pEdgeDestinationIsAction = 0.5;
		params.mutation.prog.pAdd = 0.5;
		params.mutation.prog.pDelete = 0.5;
		params.mutation.prog.pMutate = 1.0;
		params.mutation.prog.pSwap = 1.0;
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
	params.archiveSize = 50;
	Learn::LearningAgent la(le, set, params);

	ASSERT_NO_THROW(la.init()) << "Initialization of the LearningAgent should not fail.";
}

TEST_F(LearningAgentTest, EvalRoot) {
	params.archiveSize = 50;
	params.maxNbActionsPerEval = 11;
	params.nbIterationsPerPolicyEvaluation = 10;

	Learn::LearningAgent la(le, set, params);

	la.init();
	double result;
	ASSERT_NO_THROW(result = la.evaluateRoot(*la.getTPGGraph().getRootVertices().at(0), 0)) << "Evaluation from a root failed.";
	ASSERT_LE(result, 1.0) << "Average score should not exceed the score of a perfect player.";
}

TEST_F(LearningAgentTest, EvalAllRoots) {
	params.archiveSize = 50;
	params.maxNbActionsPerEval = 11;
	params.nbIterationsPerPolicyEvaluation = 10;

	Learn::LearningAgent la(le, set, params);

	la.init();
	std::multimap<double, const TPG::TPGVertex*> result;
	ASSERT_NO_THROW(result = la.evaluateAllRoots(0)) << "Evaluation from a root failed.";
	ASSERT_EQ(result.size(), la.getTPGGraph().getNbRootVertices()) << "Number of evaluated roots is under the number of roots from the TPGGraph.";
}

TEST_F(LearningAgentTest, TrainOnegeneration) {
	params.archiveSize = 50;
	params.maxNbActionsPerEval = 11;
	params.nbIterationsPerPolicyEvaluation = 3;
	params.ratioDeletedRoots = 0.90; // high number to force the apparition of root action.

	Learn::LearningAgent la(le, set, params);

	la.init();
	// Do the populate call to keep know the number of initial vertex
	Archive a(0);
	Mutator::TPGMutator::populateTPG(la.getTPGGraph(), a, params.mutation );
	int initialNbVertex = la.getTPGGraph().getVertices().size();
	ASSERT_NO_THROW(la.trainOneGeneration(0)) << "Training for one generation failed.";
	// Check the number of vertex in the graph.
	// Must be initial number of vertex - number of root removed
	ASSERT_EQ(la.getTPGGraph().getVertices().size(), initialNbVertex - floor(params.ratioDeletedRoots * params.mutation.tpg.nbRoots)) << "Number of remaining is under the number of roots from the TPGGraph.";
	// Train a second generation, because most roots were removed, a root actions have appeared
	// and the training algorithm will attempt to remove them.
	ASSERT_NO_THROW(la.trainOneGeneration(0)) << "Training for one generation failed.";
}

TEST_F(LearningAgentTest, Train) {
	params.archiveSize = 50;
	params.maxNbActionsPerEval = 11;
	params.nbIterationsPerPolicyEvaluation = 5;
	params.ratioDeletedRoots = 0.2;
	params.nbGenerations = 3;

	Learn::LearningAgent la(le, set, params);

	la.init();
	bool alt = false;

	ASSERT_NO_THROW(la.train(alt, true)) << "Training a TPG for several generation should not fail.";
	alt = true;
	ASSERT_NO_THROW(la.train(alt, true)) << "Using the boolean reference to stop the training should not fail.";
}

TEST_F(LearningAgentTest, KeepBestPolicy) {
	params.archiveSize = 50;
	params.maxNbActionsPerEval = 11;
	params.nbIterationsPerPolicyEvaluation = 1;
	params.ratioDeletedRoots = 0.2;
	params.nbGenerations = 5;

	Learn::LearningAgent la(le, set, params);
	la.init();
	bool alt = false;
	la.train(alt, true);

	ASSERT_NO_THROW(la.keepBestPolicy()) << "Keeping the best policy after training should not fail.";
	ASSERT_EQ(la.getTPGGraph().getNbRootVertices(), 1) << "A single root TPGVertex should remain in the TPGGraph when keeping the best policy only";
}