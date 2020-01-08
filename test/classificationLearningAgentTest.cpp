#include <gtest/gtest.h>

#include "dataHandlers/primitiveTypeArray.h"

#include "instructions/set.h"
#include "instructions/addPrimitiveType.h"

#include "mutator/tpgMutator.h"

#include "learn/learningEnvironment.h"
#include "learn/learningParameters.h"

#include "learn/classificationLearningAgent.h"

#include "learn/fakeClassificationLearningEnvironment.h"

class ClassificationLearningAgentTest : public ::testing::Test {
protected:
	Instructions::Set set;
	Learn::LearningParameters params;
	FakeClassificationLearningEnvironment fle;

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


TEST_F(ClassificationLearningAgentTest, Constructor) {
	Learn::ClassificationLearningAgent<Learn::LearningAgent>* cla;

	// Build with Learn::LearningAgent
	ASSERT_NO_THROW(cla = new Learn::ClassificationLearningAgent<Learn::LearningAgent>(fle, set, params)) << "Error when building a ClassificationLearningAgent.";
	ASSERT_NO_THROW(delete cla) << "Error when deleting a ClassificationLearningAgent";

	// Build with Learn::ParallelLearningAgent
	Learn::ClassificationLearningAgent<Learn::ParallelLearningAgent>* pcla;
	ASSERT_NO_THROW(pcla = new Learn::ClassificationLearningAgent<Learn::ParallelLearningAgent>(fle, set, params)) << "Error when building a ClassificationLearningAgent.";
	ASSERT_NO_THROW(delete pcla) << "Error when deleting a ClassificationLearningAgent";
}

TEST_F(ClassificationLearningAgentTest, EvaluateRoot) {
	params.archiveSize = 50;
	params.archivingProbability = 1.0;
	params.maxNbActionsPerEval = 11;
	params.nbIterationsPerPolicyEvaluation = 10;

	Learn::ClassificationLearningAgent cla(fle, set, params);
	Archive a; // For testing purposes, notmally, the archive from the LearningAgent is used.

	TPG::TPGExecutionEngine tee(cla.getTPGGraph().getEnvironment(), &a);

	cla.init();
	std::shared_ptr<Learn::EvaluationResult> result;
	ASSERT_NO_THROW(result = cla.evaluateRoot(tee, *cla.getTPGGraph().getRootVertices().at(0), 0, Learn::LearningMode::TRAINING)) << "Evaluation from a root failed.";
	ASSERT_LE(result->getResult(), 1.0) << "Average score should not exceed the score of a perfect player.";
}

TEST_F(ClassificationLearningAgentTest, DecimateWorstRoots) {
	params.archiveSize = 50;
	params.archivingProbability = 0.5;
	params.maxNbActionsPerEval = 11;
	params.nbIterationsPerPolicyEvaluation = 3;
	params.mutation.tpg.maxInitOutgoingEdges = 2;
	params.ratioDeletedRoots = 0.50;
	params.mutation.tpg.nbRoots = 50; // Param used in decimation

	Learn::ClassificationLearningAgent cla(fle, set, params, 4);

	// Initialize and populate the TPG
	cla.init();
	TPG::TPGGraph& graph = cla.getTPGGraph();
	Mutator::TPGMutator::populateTPG(graph, cla.getArchive(), params.mutation, cla.getRNG());
	uint64_t originalNbVertices = graph.getNbVertices();

	// Check that the action is now a root
	auto roots = graph.getRootVertices();
	// TODO: ASSERT_EQ(typeid(*roots.at(0)), typeid(TPG::TPGAction)) << "An action should have become a root of the TPGGraph.";

	// Create and fill results for each "root" artificially with EvaluationResults
	std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*> results;
	double result = 0.0;
	for (const TPG::TPGVertex* root : roots) {
		results.emplace(new Learn::EvaluationResult(result++), root);
	}

	// Do the decimation (must fail)
	ASSERT_THROW(cla.decimateWorstRoots(results), std::runtime_error) << "Decimating worst roots should fail with EvaluationResults instead of ClassificationEvaluationResults.";

	// Create and fill results for each "root" artificially with ClassificationEvaluationResults
	std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*> classifResults;
	result = 0.0;
	for (const TPG::TPGVertex* root : roots) {
		// Init all scores to the same value 
		// Their general score will be 0.33.
		// With score for 1st class to 0.0
		std::vector<double> scores(fle.getNbActions(), 0.33 / (fle.getNbActions() - 1) * fle.getNbActions());
		std::for_each(scores.begin(), scores.end(), [](double& score) {score = 0.0; });

		classifResults.emplace(new Learn::ClassificationEvaluationResult(scores), root);
	}

	// Change score for 4 roots, so that 
	// the first three have worse than average general score, but good score for 1st class 
	// the last has better than average general score, and good score for 1st class 
	// (current code valid for 3 classes only because of 0.25 constant)
	ASSERT_EQ(fle.getNbActions(), 3);
	std::vector<const TPG::TPGVertex*> savedRoots;
	for (auto idx = 0; idx < 4; idx++) {
		// Select a root results to erase-replace
		auto iterClassifResults = classifResults.begin();
		std::advance(iterClassifResults, 3 * idx);

		// get the root
		const TPG::TPGVertex* root = iterClassifResults->second;
		savedRoots.push_back(root);

		// Remove from map
		classifResults.erase(iterClassifResults);

		// Add custom result
		std::vector<double> scores(fle.getNbActions(), 0.0);
		scores.at(0) = 0.25 * (idx + 1);
		classifResults.emplace(new Learn::ClassificationEvaluationResult(scores), root);
	}

	// Do the decimation (must fail)
	ASSERT_NO_THROW(cla.decimateWorstRoots(classifResults)) << "Decimating worst roots should fail with EvaluationResults instead of ClassificationEvaluationResults.";

	// Check the number of remaining vertices.
	ASSERT_EQ(cla.getTPGGraph().getNbVertices(), originalNbVertices - std::ceil(params.mutation.tpg.nbRoots * (1.0 - params.ratioDeletedRoots)));

	// Check the presence of savedRoots among remaining roots.
	// i.e. check that there good result for one class saved them from decimation.
	auto remainingRoots = cla.getTPGGraph().getRootVertices();
	for (const TPG::TPGVertex* savedRoot : savedRoots) {
		ASSERT_TRUE(std::find(remainingRoots.begin(), remainingRoots.end(), savedRoot) != remainingRoots.end()) << "Roots with best classification score for 1st class were not preserved during decimation.";
	}


}