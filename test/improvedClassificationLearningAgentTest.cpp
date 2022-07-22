#include <gtest/gtest.h>

#include "data/primitiveTypeArray.h"

#include "instructions/addPrimitiveType.h"
#include "instructions/set.h"

#include "mutator/tpgMutator.h"

#include "learn/learningEnvironment.h"
#include "learn/learningParameters.h"

#include "learn/improvedClassificationLearningAgent.h"

#include "learn/fakeImprovedClassificationLearningEnvironment.h"

class ImprovedClassificationLearningAgentTest : public ::testing::Test
{
  protected:
    Instructions::Set set;
    Learn::LearningParameters params;
    FakeImprovedClassificationLearningEnvironment fle;
    Learn::LearningAlgorithm type;

  public:

  protected:
    virtual void SetUp()
    {
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
        params.mutation.prog.pConstantMutation = 0.5;
        params.mutation.prog.minConstValue = 0;
        params.mutation.prog.maxConstValue = 1;
    }

    virtual void TearDown()
    {
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
    }
};

TEST_F(ImprovedClassificationLearningAgentTest, Constructor)
{
    Learn::ImprovedClassificationLearningAgent<Learn::LearningAgent>* cla;

    // Build with Learn::LearningAgent
    ASSERT_NO_THROW(cla = new Learn::ImprovedClassificationLearningAgent<Learn::LearningAgent>(fle, set, params, TPG::TPGFactory(), type))
        << "Error when building a ClassificationLearningAgent.";
    ASSERT_NO_THROW(delete cla)
        << "Error when deleting a ClassificationLearningAgent";

    // Build with Learn::ParallelLearningAgent
    Learn::ImprovedClassificationLearningAgent<Learn::ParallelLearningAgent>* pcla;
    ASSERT_NO_THROW(pcla = new Learn::ImprovedClassificationLearningAgent<Learn::ParallelLearningAgent>(fle, set, params, TPG::TPGFactory(), type))
        << "Error when building a ClassificationLearningAgent.";
    ASSERT_NO_THROW(delete pcla)
        << "Error when deleting a ClassificationLearningAgent";
}

TEST_F(ImprovedClassificationLearningAgentTest, EvaluateRoot)
{
    params.archiveSize = 50;
    params.archivingProbability = 1.0;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 10;
    // Only 2 evaluations of each root should be done (one to create a result,
    // one to cover the line responsible for combining preexisting results with
    // new ones). Warning: in ClassificationLearningEnvironment, the number of
    // evaluation corresponds to the total number of action.
    params.maxNbEvaluationPerPolicy = 2 * params.nbIterationsPerPolicyEvaluation * params.maxNbActionsPerEval;

    Learn::ImprovedClassificationLearningAgent cla(fle, set, params);
    Archive a; // For testing purposes, normally, the archive from the
               // LearningAgent is used.

    TPG::TPGExecutionEngine tee(cla.getTPGGraph()->getEnvironment(), &a);

    cla.init();
    std::shared_ptr<Learn::EvaluationResult> result1;
    ASSERT_NO_THROW(result1 = cla.evaluateJob(tee, *cla.makeJob(0, Learn::LearningMode::TRAINING), 0,Learn::LearningMode::TRAINING, fle))
        << "Evaluation from a root failed.";
    ASSERT_LE(result1->getResult(), 1.0)
        << "Average score should not exceed the score of a perfect player.";

    // Record this result
    cla.updateEvaluationRecords({{result1, cla.getTPGGraph()->getRootVertices().at(0)}});

    // Reevaluate to check that the previous result1 is not returned.
    std::shared_ptr<Learn::EvaluationResult> result2;
    ASSERT_NO_THROW(result2 = cla.evaluateJob(tee, *cla.makeJob(0, Learn::LearningMode::TRAINING), 0,Learn::LearningMode::TRAINING, fle))
        << "Evaluation from a root failed.";
    ASSERT_NE(result1, result2);

    // Record this result
    cla.updateEvaluationRecords({{result2, cla.getTPGGraph()->getRootVertices().at(0)}});

    // Reevaluate to check that the previous result2 is returned.
    std::shared_ptr<Learn::EvaluationResult> result3;
    ASSERT_NO_THROW(result3 = cla.evaluateJob(tee, *cla.makeJob(0, Learn::LearningMode::TRAINING), 0,Learn::LearningMode::TRAINING, fle))
        << "Evaluation from a root failed.";
    ASSERT_EQ(result3, result2);
}

TEST_F(ImprovedClassificationLearningAgentTest, DecimateWorstRoots)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 3;
    params.mutation.tpg.maxInitOutgoingEdges = 2;
    params.ratioDeletedRoots = 0.50;
    params.mutation.tpg.nbRoots = 50; // Param used in decimation
    params.nbThreads = 4;

    Learn::ImprovedClassificationLearningAgent cla(fle, set, params);

    // Initialize and populate the TPG
    cla.init(0);
    TPG::TPGGraph& graph = *cla.getTPGGraph();
    Mutator::TPGMutator::populateTPG(graph, cla.getArchive(), params.mutation,cla.getRNG());

    // Get roots
    auto roots = graph.getRootVertices();

    // Create and fill results for each "root" artificially with
    // EvaluationResults
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,const TPG::TPGVertex*> results;
    double result = 0.0;
    for (const TPG::TPGVertex* root : roots)
        results.emplace(new Learn::EvaluationResult(result++, 1), root);

    // Do the decimation (must fail)
    ASSERT_THROW(cla.decimateWorstRoots(results), std::runtime_error)
        << "Decimating worst roots should fail with EvaluationResults instead "
           "of ClassificationEvaluationResults.";

    // Create and fill results for each "root" artificially with
    // ClassificationEvaluationResults
    std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*> classifResults;
    result = 0.0;
    for (const TPG::TPGVertex* root : roots)
    {
        // Init all scores to the same value
        // Their general score will be 0.33.
        // With score for 1st class to 0.0
        std::vector<double> scores(fle.getNbActions(), 0.33 / (double)(fle.getNbActions() - 1) * (double)fle.getNbActions());
        scores.at(0) = 0.0;
        std::vector<size_t> nbEval(fle.getNbActions(), 1);

        classifResults.emplace(new Learn::ClassificationEvaluationResult(scores, nbEval), root);
    }

    // Change score for 4 roots, so that
    // the first three have worse than average general score, but good score for
    // 1st class the last has better than average general score, and good score
    // for 1st class (current code valid for 3 classes only because of 0.25
    // constant)
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

        // Add custom result1
        std::vector<double> scores(fle.getNbActions(), 0.0);
        scores.at(0) = 0.25 * (idx + 1.0);
        std::vector<size_t> nbEvals(fle.getNbActions(), 10);
        classifResults.emplace(
            new Learn::ClassificationEvaluationResult(scores, nbEvals), root);
    }

    // Add an additional
    // - root action (should not be removed, despite having the worst score)
    // - team root (will be removed with the same score)
    const TPG::TPGVertex& actionRoot = graph.addNewAction(0);
    const TPG::TPGVertex& teamRoot = graph.addNewTeam();

    uint64_t originalNbVertices = graph.getNbVertices();

    // Create a poor score for the action and team root
    classifResults.emplace(
        new Learn::ClassificationEvaluationResult(
            std::vector<double>(fle.getNbActions(), 0.0),
            std::vector<size_t>(fle.getNbActions(), size_t(10))),
        &actionRoot);
    classifResults.emplace(
        new Learn::ClassificationEvaluationResult(
            std::vector<double>(fle.getNbActions(), 0.0),
            std::vector<size_t>(fle.getNbActions(), size_t(10))),
        &teamRoot);

    // Do the decimation
    ASSERT_NO_THROW(cla.decimateWorstRoots(classifResults))
        << "Decimating worst roots should not fail with "
           "ClassificationEvaluationResults.";

    // Check the number of remaining vertices.
    ASSERT_EQ(cla.getTPGGraph()->getNbVertices(), originalNbVertices - std::ceil(params.mutation.tpg.nbRoots * (1.0 - params.ratioDeletedRoots)));

    // Check the presence of savedRoots among remaining roots.
    // i.e. check that their good result1 for one class saved them from
    // decimation.
    auto remainingRoots = cla.getTPGGraph()->getRootVertices();
    for (const TPG::TPGVertex* savedRoot : savedRoots)
    {
        ASSERT_TRUE(std::find(remainingRoots.begin(), remainingRoots.end(),savedRoot) != remainingRoots.end())
            << "Roots with best classification score for 1st class were not "
               "preserved during decimation.";
    }

    // Check the presence of action root among remaining roots.
    ASSERT_TRUE(std::find(remainingRoots.begin(), remainingRoots.end(),&actionRoot) != remainingRoots.end())
        << "Action roots with poor score were not preserved during decimation.";
    // Check the absence of team root among remaining roots.
    ASSERT_TRUE(std::find(remainingRoots.begin(), remainingRoots.end(),&teamRoot) == remainingRoots.end())
        << "Action roots with poor score were not preserved during decimation.";
}

