#include <gtest/gtest.h>
#include <numeric>

#include "mutator/rng.h"
#include "mutator/tpgMutator.h"
#include "instructions/addPrimitiveType.h"

#include "learn/learningEnvironment.h"
#include "learn/stickGameWithOpponent.h"
#include "learn/learningParameters.h"
#include "learn/learningAgent.h"
#include "learn/parallelLearningAgent.h"

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

class ParallelLearningAgentTest : public LearningAgentTest {};

TEST_F(LearningAgentTest, Constructor) {
	Learn::LearningAgent* la;

	ASSERT_NO_THROW(la = new Learn::LearningAgent(le, set, params)) << "Construction of the learningAgent failed.";

	ASSERT_NO_THROW(delete la) << "Destruction of the LearningAgent failed.";
}

TEST_F(LearningAgentTest, Init) {
	params.archiveSize = 50;
	params.archivingProbability = 0.5;
	Learn::LearningAgent la(le, set, params);

	ASSERT_NO_THROW(la.init()) << "Initialization of the LearningAgent should not fail.";
}

TEST_F(LearningAgentTest, EvalRoot) {
	params.archiveSize = 50;
	params.archivingProbability = 1.0;
	params.maxNbActionsPerEval = 11;
	params.nbIterationsPerPolicyEvaluation = 10;

	Learn::LearningAgent la(le, set, params);

	la.init();
	double result;
	ASSERT_NO_THROW(result = la.evaluateRoot(*la.getTPGGraph().getRootVertices().at(0), 0, Learn::LearningMode::TRAINING)) << "Evaluation from a root failed.";
	ASSERT_LE(result, 1.0) << "Average score should not exceed the score of a perfect player.";
}

TEST_F(LearningAgentTest, EvalAllRoots) {
	params.archiveSize = 50;
	params.archivingProbability = 0.5;
	params.maxNbActionsPerEval = 11;
	params.nbIterationsPerPolicyEvaluation = 10;

	Learn::LearningAgent la(le, set, params);

	la.init();
	std::multimap<double, const TPG::TPGVertex*> result;
	ASSERT_NO_THROW(result = la.evaluateAllRoots(0, Learn::LearningMode::TRAINING)) << "Evaluation from a root failed.";
	ASSERT_EQ(result.size(), la.getTPGGraph().getNbRootVertices()) << "Number of evaluated roots is under the number of roots from the TPGGraph.";
}

TEST_F(LearningAgentTest, TrainOnegeneration) {
	params.archiveSize = 50;
	params.archivingProbability = 0.5;
	params.maxNbActionsPerEval = 11;
	params.nbIterationsPerPolicyEvaluation = 3;
	params.ratioDeletedRoots = 0.95; // high number to force the apparition of root action.

	Learn::LearningAgent la(le, set, params);

	la.init();
	// Do the populate call to keep know the number of initial vertex
	Archive a(0);
	Mutator::TPGMutator::populateTPG(la.getTPGGraph(), a, params.mutation);
	size_t initialNbVertex = la.getTPGGraph().getNbVertices();
	// Seed selected so that an action becomes a root during next generation
	ASSERT_NO_THROW(la.trainOneGeneration(4)) << "Training for one generation failed.";
	// Check the number of vertex in the graph.
	// Must be initial number of vertex - number of root removed
	ASSERT_EQ(la.getTPGGraph().getNbVertices(), initialNbVertex - floor(params.ratioDeletedRoots * params.mutation.tpg.nbRoots)) << "Number of remaining is under the number of roots from the TPGGraph.";
	// Train a second generation, because most roots were removed, a root actions have appeared
	// and the training algorithm will attempt to remove them.
	ASSERT_NO_THROW(la.trainOneGeneration(0)) << "Training for one generation failed.";
}

TEST_F(LearningAgentTest, Train) {
	params.archiveSize = 50;
	params.archivingProbability = 0.5;
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
	params.archivingProbability = 0.5;
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

TEST_F(ParallelLearningAgentTest, Constructor) {
	Learn::ParallelLearningAgent* pla;

	ASSERT_NO_THROW(pla = new Learn::ParallelLearningAgent(le, set, params)) << "Construction of the learningAgent failed.";

	ASSERT_NO_THROW(delete pla) << "Destruction of the LearningAgent failed.";
}

TEST_F(ParallelLearningAgentTest, Init) {
	params.archiveSize = 50;
	params.archivingProbability = 0.5;
	Learn::ParallelLearningAgent pla(le, set, params);

	ASSERT_NO_THROW(pla.init()) << "Initialization of the LearningAgent should not fail.";
}

TEST_F(ParallelLearningAgentTest, EvalRootSequential) {
	params.archiveSize = 50;
	params.archivingProbability = 1.0;
	params.maxNbActionsPerEval = 11;
	params.nbIterationsPerPolicyEvaluation = 10;
	params.mutation.tpg.nbActions = le.getNbActions();

	Environment env(set, le.getDataSources(), 8);

	TPG::TPGGraph tpg(env);

	// Initialize Randomness
	Mutator::RNG::setSeed(0);

	// Initialize the tpg
	Mutator::TPGMutator::initRandomTPG(tpg, params.mutation);

	// create the archive
	Archive archive;

	// The TPGExecutionEngine
	TPG::TPGExecutionEngine tee(env, &archive);

	double result;
	ASSERT_NO_THROW(result = Learn::ParallelLearningAgent::evaluateRoot(tee, *tpg.getRootVertices().at(0), 0, Learn::LearningMode::TRAINING, le, params)) << "Evaluation from a root failed.";
	ASSERT_LE(result, 1.0) << "Average score should not exceed the score of a perfect player.";
}

TEST_F(ParallelLearningAgentTest, EvalAllRootsSequential) {
	params.archiveSize = 50;
	params.archivingProbability = 0.5;
	params.maxNbActionsPerEval = 11;
	params.nbIterationsPerPolicyEvaluation = 10;

	Learn::ParallelLearningAgent pla(le, set, params, 1);

	pla.init();
	std::multimap<double, const TPG::TPGVertex*> result;
	ASSERT_NO_THROW(result = pla.evaluateAllRoots(0, Learn::LearningMode::TRAINING)) << "Evaluation from a root failed.";
	ASSERT_EQ(result.size(), pla.getTPGGraph().getNbRootVertices()) << "Number of evaluated roots is under the number of roots from the TPGGraph.";
}

TEST_F(ParallelLearningAgentTest, EvalAllRootsParallel) {
	params.archiveSize = 50;
	params.archivingProbability = 0.5;
	params.maxNbActionsPerEval = 11;
	params.nbIterationsPerPolicyEvaluation = 10;

	Learn::ParallelLearningAgent pla(le, set, params, 4);

	pla.init();
	std::multimap<double, const TPG::TPGVertex*> result;
	ASSERT_NO_THROW(result = pla.evaluateAllRoots(0, Learn::LearningMode::TRAINING)) << "Evaluation from a root failed.";
	ASSERT_EQ(result.size(), pla.getTPGGraph().getNbRootVertices()) << "Number of evaluated roots is under the number of roots from the TPGGraph.";
}

TEST_F(ParallelLearningAgentTest, TrainOnegenerationSequential) {
	params.archiveSize = 50;
	params.archivingProbability = 0.5;
	params.maxNbActionsPerEval = 11;
	params.nbIterationsPerPolicyEvaluation = 3;
	params.ratioDeletedRoots = 0.95; // high number to force the apparition of root action.

	Learn::ParallelLearningAgent pla(le, set, params, 1);

	pla.init();
	// Do the populate call to keep know the number of initial vertex
	Archive a(0);
	Mutator::TPGMutator::populateTPG(pla.getTPGGraph(), a, params.mutation);
	size_t initialNbVertex = pla.getTPGGraph().getNbVertices();
	// Seed selected so that an action becomes a root during next generation
	ASSERT_NO_THROW(pla.trainOneGeneration(4)) << "Training for one generation failed.";
	// Check the number of vertex in the graph.
	// Must be initial number of vertex - number of root removed
	ASSERT_EQ(pla.getTPGGraph().getNbVertices(), initialNbVertex - floor(params.ratioDeletedRoots * params.mutation.tpg.nbRoots)) << "Number of remaining is under the number of roots from the TPGGraph.";
	// Train a second generation, because most roots were removed, a root actions have appeared
	// and the training algorithm will attempt to remove them.
	ASSERT_NO_THROW(pla.trainOneGeneration(0)) << "Training for one generation failed.";
}

TEST_F(ParallelLearningAgentTest, TrainOneGenerationParallel) {
	params.archiveSize = 50;
	params.archivingProbability = 0.5;
	params.maxNbActionsPerEval = 11;
	params.nbIterationsPerPolicyEvaluation = 3;
	params.ratioDeletedRoots = 0.95; // high number to force the apparition of root action.

	Learn::ParallelLearningAgent pla(le, set, params, 4);

	pla.init();
	// Do the populate call to keep know the number of initial vertex
	Archive a(0);
	Mutator::TPGMutator::populateTPG(pla.getTPGGraph(), a, params.mutation);
	size_t initialNbVertex = pla.getTPGGraph().getNbVertices();
	// Seed selected so that an action becomes a root during next generation
	ASSERT_NO_THROW(pla.trainOneGeneration(4)) << "Training for one generation failed.";
	// Check the number of vertex in the graph.
	// Must be initial number of vertex - number of root removed
	ASSERT_EQ(pla.getTPGGraph().getNbVertices(), initialNbVertex - floor(params.ratioDeletedRoots * params.mutation.tpg.nbRoots)) << "Number of remaining is under the number of roots from the TPGGraph.";
	// Train a second generation, because most roots were removed, a root actions have appeared
	// and the training algorithm will attempt to remove them.
	ASSERT_NO_THROW(pla.trainOneGeneration(0)) << "Training for one generation failed.";
}

TEST_F(ParallelLearningAgentTest, TrainSequential) {
	params.archiveSize = 50;
	params.archivingProbability = 0.5;
	params.maxNbActionsPerEval = 11;
	params.nbIterationsPerPolicyEvaluation = 5;
	params.ratioDeletedRoots = 0.2;
	params.nbGenerations = 3;

	Learn::ParallelLearningAgent pla(le, set, params, 1);

	pla.init();
	bool alt = false;

	ASSERT_NO_THROW(pla.train(alt, true)) << "Training a TPG for several generation should not fail.";
	alt = true;
	ASSERT_NO_THROW(pla.train(alt, true)) << "Using the boolean reference to stop the training should not fail.";
}

TEST_F(ParallelLearningAgentTest, TrainParallel) {
	params.archiveSize = 50;
	params.archivingProbability = 0.5;
	params.maxNbActionsPerEval = 11;
	params.nbIterationsPerPolicyEvaluation = 5;
	params.ratioDeletedRoots = 0.2;
	params.nbGenerations = 3;

	Learn::ParallelLearningAgent pla(le, set, params, std::thread::hardware_concurrency());

	pla.init();
	bool alt = false;

	ASSERT_NO_THROW(pla.train(alt, true)) << "Training a TPG for several generation should not fail.";
	alt = true;
	ASSERT_NO_THROW(pla.train(alt, true)) << "Using the boolean reference to stop the training should not fail.";
}

TEST_F(ParallelLearningAgentTest, KeepBestPolicy) {
	params.archiveSize = 50;
	params.archivingProbability = 0.5;
	params.maxNbActionsPerEval = 11;
	params.nbIterationsPerPolicyEvaluation = 1;
	params.ratioDeletedRoots = 0.2;
	params.nbGenerations = 5;

	Learn::ParallelLearningAgent pla(le, set, params);
	pla.init();
	bool alt = false;
	pla.train(alt, true);

	ASSERT_NO_THROW(pla.keepBestPolicy()) << "Keeping the best policy after training should not fail.";
	ASSERT_EQ(pla.getTPGGraph().getNbRootVertices(), 1) << "A single root TPGVertex should remain in the TPGGraph when keeping the best policy only";
}