/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
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

#include <gtest/gtest.h>

#include "data/primitiveTypeArray.h"

#include "instructions/addPrimitiveType.h"
#include "instructions/set.h"

#include "mutator/tpgMutator.h"

#include "learn/learningEnvironment.h"
#include "learn/learningParameters.h"

#include "learn/classificationLearningAgent.h"

#include "learn/fakeClassificationLearningEnvironment.h"

class ClassificationLearningAgentTest : public ::testing::Test
{
  protected:
    Instructions::Set set;
    Learn::LearningParameters params;
    FakeClassificationLearningEnvironment fle;

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

TEST_F(ClassificationLearningAgentTest, Constructor)
{
    Learn::ClassificationLearningAgent<Learn::LearningAgent>* cla;

    // Build with Learn::LearningAgent
    ASSERT_NO_THROW(
        cla = new Learn::ClassificationLearningAgent<Learn::LearningAgent>(
            fle, set, params))
        << "Error when building a ClassificationLearningAgent.";
    ASSERT_NO_THROW(delete cla)
        << "Error when deleting a ClassificationLearningAgent";

    // Build with Learn::ParallelLearningAgent
    Learn::ClassificationLearningAgent<Learn::ParallelLearningAgent>* pcla;
    ASSERT_NO_THROW(pcla = new Learn::ClassificationLearningAgent<
                        Learn::ParallelLearningAgent>(fle, set, params))
        << "Error when building a ClassificationLearningAgent.";
    ASSERT_NO_THROW(delete pcla)
        << "Error when deleting a ClassificationLearningAgent";
}

TEST_F(ClassificationLearningAgentTest, EvaluateRoot)
{
    params.archiveSize = 50;
    params.archivingProbability = 1.0;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 10;
    // Only 2 evaluations of each root should be done (one to create a result,
    // one to cover the line responsible for combining preexisting results with
    // new ones). Warning: in ClassificationLearningEnvironment, the number of
    // evaluation corresponds to the total number of action.
    params.maxNbEvaluationPerPolicy =
        2 * params.nbIterationsPerPolicyEvaluation * params.maxNbActionsPerEval;

    Learn::ClassificationLearningAgent cla(fle, set, params);
    Archive a; // For testing purposes, normally, the archive from the
               // LearningAgent is used.

    TPG::TPGExecutionEngine tee(cla.getTPGGraph()->getEnvironment(), &a);

    cla.init();
    std::shared_ptr<Learn::EvaluationResult> result1;
    ASSERT_NO_THROW(result1 = cla.evaluateJob(
                        tee, *cla.makeJob(0, Learn::LearningMode::TRAINING), 0,
                        Learn::LearningMode::TRAINING, fle))
        << "Evaluation from a root failed.";
    ASSERT_LE(result1->getResult(), 1.0)
        << "Average score should not exceed the score of a perfect player.";

    // Record this result
    cla.updateEvaluationRecords(
        {{result1, cla.getTPGGraph()->getRootVertices().at(0)}});

    // Reevaluate to check that the previous result1 is not returned.
    std::shared_ptr<Learn::EvaluationResult> result2;
    ASSERT_NO_THROW(result2 = cla.evaluateJob(
                        tee, *cla.makeJob(0, Learn::LearningMode::TRAINING), 0,
                        Learn::LearningMode::TRAINING, fle))
        << "Evaluation from a root failed.";
    ASSERT_NE(result1, result2);

    // Record this result
    cla.updateEvaluationRecords(
        {{result2, cla.getTPGGraph()->getRootVertices().at(0)}});

    // Reevaluate to check that the previous result2 is returned.
    std::shared_ptr<Learn::EvaluationResult> result3;
    ASSERT_NO_THROW(result3 = cla.evaluateJob(
                        tee, *cla.makeJob(0, Learn::LearningMode::TRAINING), 0,
                        Learn::LearningMode::TRAINING, fle))
        << "Evaluation from a root failed.";
    ASSERT_EQ(result3, result2);
}

TEST_F(ClassificationLearningAgentTest, DecimateWorstRoots)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 3;
    params.mutation.tpg.maxInitOutgoingEdges = 2;
    params.ratioDeletedRoots = 0.50;
    params.mutation.tpg.nbRoots = 50; // Param used in decimation
    params.nbThreads = 4;

    Learn::ClassificationLearningAgent cla(fle, set, params);

    // Initialize and populate the TPG
    cla.init(0);
    TPG::TPGGraph& graph = *cla.getTPGGraph();
    Mutator::TPGMutator::populateTPG(graph, cla.getArchive(), params.mutation,
                                     cla.getRNG());

    // Get roots
    auto roots = graph.getRootVertices();

    // Create and fill results for each "root" artificially with
    // EvaluationResults
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const TPG::TPGVertex*>
        results;
    double result = 0.0;
    for (const TPG::TPGVertex* root : roots) {
        results.emplace(new Learn::EvaluationResult(result++, 1), root);
    }

    // Do the decimation (must fail)
    ASSERT_THROW(cla.decimateWorstRoots(results), std::runtime_error)
        << "Decimating worst roots should fail with EvaluationResults instead "
           "of ClassificationEvaluationResults.";

    // Create and fill results for each "root" artificially with
    // ClassificationEvaluationResults
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const TPG::TPGVertex*>
        classifResults;
    result = 0.0;
    for (const TPG::TPGVertex* root : roots) {
        // Init all scores to the same value
        // Their general score will be 0.33.
        // With score for 1st class to 0.0
        std::vector<double> scores(fle.getNbActions(),
                                   0.33 / (fle.getNbActions() - 1) *
                                       fle.getNbActions());
        scores.at(0) = 0.0;
        std::vector<size_t> nbEval(fle.getNbActions(), 1);

        classifResults.emplace(
            new Learn::ClassificationEvaluationResult(scores, nbEval), root);
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
    ASSERT_EQ(cla.getTPGGraph()->getNbVertices(),
              originalNbVertices - std::ceil(params.mutation.tpg.nbRoots *
                                             (1.0 - params.ratioDeletedRoots)));

    // Check the presence of savedRoots among remaining roots.
    // i.e. check that their good result1 for one class saved them from
    // decimation.
    auto remainingRoots = cla.getTPGGraph()->getRootVertices();
    for (const TPG::TPGVertex* savedRoot : savedRoots) {
        ASSERT_TRUE(std::find(remainingRoots.begin(), remainingRoots.end(),
                              savedRoot) != remainingRoots.end())
            << "Roots with best classification score for 1st class were not "
               "preserved during decimation.";
    }

    // Check the presence of action root among remaining roots.
    ASSERT_TRUE(std::find(remainingRoots.begin(), remainingRoots.end(),
                          &actionRoot) != remainingRoots.end())
        << "Action roots with poor score were not preserved during decimation.";
    // Check the absence of team root among remaining roots.
    ASSERT_TRUE(std::find(remainingRoots.begin(), remainingRoots.end(),
                          &teamRoot) == remainingRoots.end())
        << "Action roots with poor score were not preserved during decimation.";
}
