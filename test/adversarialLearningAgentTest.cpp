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

#include <fstream>
#include <gtest/gtest.h>
#include <numeric>

#include "tpg/tpgGraph.h"

#include "instructions/addPrimitiveType.h"
#include "mutator/tpgMutator.h"

#include "learn/adversarialLearningAgent.h"
#include "learn/adversarialLearningAgentWithCustomMakeJobs.h"
#include "learn/fakeAdversarialLearningEnvironment.h"
#include "learn/fakeClassificationLearningEnvironment.h"
#include "learn/learningEnvironment.h"
#include "learn/learningParameters.h"
#include "learn/parallelLearningAgent.h"
#include "learn/stickGameAdversarial.h"

class adversarialLearningAgentTest : public ::testing::Test
{
  protected:
    Instructions::Set set;
    StickGameAdversarial le;
    Learn::LearningParameters params;

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
        params.mutation.tpg.maxOutgoingEdges = 4;
        params.mutation.prog.pAdd = 0.5;
        params.mutation.prog.pDelete = 0.5;
        params.mutation.prog.pMutate = 1.0;
        params.mutation.prog.pSwap = 1.0;
    }

    virtual void TearDown()
    {
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
    }
};

TEST_F(adversarialLearningAgentTest, Constructor)
{
    Learn::AdversarialLearningAgent* la;

    ASSERT_NO_THROW(la = new Learn::AdversarialLearningAgent(le, set, params))
        << "Construction of the AdversarialLearningAgent failed.";

    ASSERT_NO_THROW(delete la)
        << "Destruction of the AdversarialLearningAgent failed.";
}

TEST_F(adversarialLearningAgentTest, MakeJobs)
{
    params.nbIterationsPerPolicyEvaluation = 20;
    params.nbIterationsPerJob = 2;
    size_t agentsPerEval = 5;
    Learn::AdversarialLearningAgent la(le, set, params, agentsPerEval);
    // 5 agents per job, 2 eval per job
    // => It shall do 10 jobs per agent, and something close to nbRoots*2
    // (nbRoots*20/(5*2)) jobs at total the complete formula of the nb of jobs
    // is
    la.init();
    auto jobs = la.makeJobs(Learn::TRAINING);
    ASSERT_EQ(la.getTPGGraph().getNbRootVertices() * 2, jobs.size())
        << "There should be as many jobs as roots.";
    // this map will compute the number of iterations per root that are
    // scheduled
    std::map<const TPG::TPGVertex*, int> nbEvalPerRoot;
    for (auto root : la.getTPGGraph().getRootVertices()) {
        nbEvalPerRoot.emplace(root, 0);
    }
    while (!jobs.empty()) {
        auto job =
            std::dynamic_pointer_cast<Learn::AdversarialJob>(jobs.front());
        ASSERT_EQ(5, job->getSize())
            << "job doesn'nt contain the right roots number.";
        // updates number of iterations scheduled for the roots of the job
        for (auto root : job->getRoots()) {
            auto it = nbEvalPerRoot.find(root);
            it->second = it->second + params.nbIterationsPerJob;
        }
        jobs.pop();
    }
    // now check there are enough iterations per root scheduled
    for (auto pairRootNbEval : nbEvalPerRoot) {
        ASSERT_GE(pairRootNbEval.second, params.nbIterationsPerPolicyEvaluation)
            << "jobs don't evaluate enough a root.";
    }
}

TEST_F(adversarialLearningAgentTest, EvalJob)
{
    params.archiveSize = 50;
    params.archivingProbability = 1.0;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 10;

    Learn::AdversarialLearningAgent la(le, set, params);
    Archive a; // For testing purposes, notmally, the archive from the
    // LearningAgent is used.

    TPG::TPGExecutionEngine tee(la.getTPGGraph().getEnvironment(), &a);

    la.init();
    std::shared_ptr<Learn::EvaluationResult> result;
    auto job = Learn::AdversarialJob({la.getTPGGraph().getRootVertices()[0]});
    ASSERT_NO_THROW(
        result = la.evaluateJob(tee, job, 0, Learn::LearningMode::TRAINING, le))
        << "Evaluation from a root in no parallel and no adversarial mode "
           "failed.";
    ASSERT_LE(result->getResult(), 1.0)
        << "Average score should not exceed the score of a perfect layer.";

    auto jobs = la.makeJobs(Learn::LearningMode::TRAINING);
    ASSERT_NO_THROW(result = la.evaluateJob(tee, *jobs.front(), 0,
                                            Learn::LearningMode::TRAINING, le))
        << "Evaluation from a root in adversarial mode failed.";
    ASSERT_LE(result->getResult(), 1.0)
        << "Average score should not exceed the score of a perfect layer.";
}

TEST_F(adversarialLearningAgentTest, GetArchive)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 10;

    Learn::AdversarialLearningAgent la(le, set, params);

    la.init();
    la.evaluateAllRoots(0, Learn::LearningMode::TRAINING);

    ASSERT_NO_THROW(la.getArchive())
        << "Cannot get the archive of a LearningAgent.";
}

// Similat to previous test, but verifications of graphs properties are here to
// ensure the result of the training is identical on all OSes and Compilers.
TEST_F(adversarialLearningAgentTest, TrainPortability)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.05;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 2;
    params.maxNbEvaluationPerPolicy = 0;
    params.ratioDeletedRoots = 0.5;
    params.nbGenerations = 20;
    params.mutation.tpg.nbRoots = 30;
    params.mutation.tpg.nbActions = 3;
    params.mutation.tpg.maxInitOutgoingEdges = 3;
    params.mutation.tpg.maxOutgoingEdges = 10;
    params.nbIterationsPerJob = 1;

    Learn::AdversarialLearningAgent la(le, set, params, 2);

    la.init();
    bool alt = false;
    la.train(alt, false);

    // It is quite unlikely that two different TPGs after 20 generations
    // end up with the same number of vertices, roots, edges and calls to
    // the RNG without being identical.
    TPG::TPGGraph& tpg = la.getTPGGraph();
    ASSERT_EQ(tpg.getNbVertices(), 26)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(tpg.getNbRootVertices(), 18)
        << "Graph does not have the expected determinist characteristics.";
    ASSERT_EQ(tpg.getEdges().size(), 179)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(la.getRNG().getUnsignedInt64(0, UINT64_MAX), 2660350567691381690)
        << "Graph does not have the expected determinst characteristics.";
}

TEST_F(adversarialLearningAgentTest, EvalAllRootsSequential)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 10;
    params.nbThreads = 1;

    Learn::AdversarialLearningAgent la(le, set, params);

    la.init();
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const TPG::TPGVertex*>
        result;
    ASSERT_NO_THROW(result =
                        la.evaluateAllRoots(0, Learn::LearningMode::TRAINING))
        << "Evaluation from a root failed.";
    ASSERT_EQ(result.size(), la.getTPGGraph().getNbRootVertices())
        << "Number of evaluated roots is under the number of roots from the "
           "TPGGraph.";

    auto le2 = FakeClassificationLearningEnvironment();
    Learn::AdversarialLearningAgent laNotCopyabe(le2, set, params);

    ASSERT_THROW(
        laNotCopyabe.evaluateAllRoots(0, Learn::LearningMode::TRAINING),
        std::runtime_error);
}

TEST_F(adversarialLearningAgentTest, EvalAllRootsParallel)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 10;
    params.nbThreads = 4;

    Learn::AdversarialLearningAgent la(le, set, params);

    la.init();
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const TPG::TPGVertex*>
        result;
    ASSERT_NO_THROW(result =
                        la.evaluateAllRoots(0, Learn::LearningMode::TRAINING))
        << "Evaluation from a root failed.";
    ASSERT_EQ(result.size(), la.getTPGGraph().getNbRootVertices())
        << "Number of evaluated roots is under the number of roots from the "
           "TPGGraph.";
}

TEST_F(adversarialLearningAgentTest, EvalAllRootsGoodResults)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 10;
    params.nbThreads = 1;
    params.mutation.tpg.nbRoots = 3; // important : keep it at 3 or more !
    // the custom learning agent will only put the 3 first roots in jobs

    FakeAdversarialLearningEnvironment customLe;

    AdversarialLearningAgentWithCustomMakeJobs la(customLe, set, params);

    la.init();
    auto roots = la.getTPGGraph().getRootVertices();
    auto firstRoot = roots[0];
    auto secondRoot = roots[1];
    auto thirdRoot = roots[2];

    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const TPG::TPGVertex*>
        result;

    // this evaluation is custom, see AdversarialLearningAgentWithCustomMakeJobs
    // and FakeAdversarialLearningEnvironment.
    // to be brief, the 3 first roots of the Learning Env will have known scores
    ASSERT_NO_THROW(result =
                        la.evaluateAllRoots(0, Learn::LearningMode::TRAINING))
        << "Evaluation from a root failed.";

    auto iter = result.begin();
    ASSERT_EQ(firstRoot, iter->second) << "Wrong root has 1st place.";
    ASSERT_EQ(-0.5, iter++->first->getResult())
        << "Wrong score for 1st root after an eval.";
    ASSERT_EQ(secondRoot, iter->second) << "Wrong root has 2nd place.";
    ASSERT_EQ(0.75, iter++->first->getResult())
        << "Wrong score for 2nd root after an eval.";
    ASSERT_EQ(thirdRoot, iter->second) << "Wrong root has 3rd place.";
    ASSERT_EQ(1.75, iter->first->getResult())
        << "Wrong score for 3rd root after an eval.";
}

TEST_F(adversarialLearningAgentTest, EvalAllRootsParallelTrainingDeterminism)
{
    // Check that parallel execution leads to the exact same results as
    // sequential
    params.archiveSize = 50;
    params.archivingProbability = 0.1;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 10;

    Learn::AdversarialLearningAgent la(le, set, params);
    la.init(0); // Reset RNG to 0
    auto results = la.evaluateAllRoots(0, Learn::LearningMode::TRAINING);
    auto nextInt = la.getRNG().getUnsignedInt64(0, UINT64_MAX);

    Learn::LearningParameters paramsSequential = params;
    paramsSequential.nbThreads = 1;
    Learn::AdversarialLearningAgent laSequential(le, set, paramsSequential);

    laSequential.init(0); // Reset centralized RNG to 0
    auto resultsSequential =
        laSequential.evaluateAllRoots(0, Learn::LearningMode::TRAINING);
    auto nextIntSequential =
        laSequential.getRNG().getUnsignedInt64(0, UINT64_MAX);

    Learn::LearningParameters paramsParallel = params;
    paramsParallel.nbThreads = 4;
    Learn::AdversarialLearningAgent laParallel(le, set, paramsParallel);

    laParallel.init(0); // Reset centralized RNG to 0
    auto resultsParallel =
        laParallel.evaluateAllRoots(0, Learn::LearningMode::TRAINING);
    auto nextIntParallel = laParallel.getRNG().getUnsignedInt64(0, UINT64_MAX);

    // Check equality between LearningAgent and ParallelLearningAgent
    ASSERT_EQ(results.size(), resultsSequential.size())
        << "Result maps have a different size.";
    auto iter = results.begin();
    auto iterSequential = resultsSequential.begin();
    while (iter != results.end()) {
        ASSERT_EQ(iter->first->getResult(), iterSequential->first->getResult())
            << "Average score between sequential and parallel executions are "
               "differents.";
        iter++;
        iterSequential++;
    }

    // Check determinism of bestRoot score
    ASSERT_EQ(la.getBestRoot().second, laSequential.getBestRoot().second);

    // Check determinism of the number of RNG calls.
    ASSERT_EQ(nextInt, nextIntSequential)
        << "Mutator::RNG was called a different number of time in parallel and "
           "sequential execution.";

    // Check archives
    ASSERT_GT(la.getArchive().getNbRecordings(), 0)
        << "For the archive determinism tests to be meaningful, Archive should "
           "not be empty.";
    ASSERT_EQ(la.getArchive().getNbRecordings(),
              laSequential.getArchive().getNbRecordings())
        << "Archives have different sizes.";
    for (auto i = 0; i < la.getArchive().getNbRecordings(); i++) {
        ASSERT_EQ(la.getArchive().at(i).dataHash,
                  laSequential.getArchive().at(i).dataHash)
            << "Archives have different content.";
        ASSERT_EQ(la.getArchive().at(i).result,
                  laSequential.getArchive().at(i).result)
            << "Archives have different content.";
    }

    // Check equality between ParallelLearningAgent in parallel and sequential
    // mode
    ASSERT_EQ(resultsParallel.size(), resultsParallel.size())
        << "Result maps have a different size.";
    iterSequential = resultsSequential.begin();
    auto iterParallel = resultsParallel.begin();
    while (iterSequential != resultsSequential.end()) {
        ASSERT_EQ(iterSequential->first->getResult(),
                  iterParallel->first->getResult())
            << "Average score between sequential and parallel executions are "
               "differents.";
        iterSequential++;
        iterParallel++;
    }

    // Check determinism of bestRoot score
    ASSERT_EQ(laSequential.getBestRoot().second,
              laParallel.getBestRoot().second);

    // Check determinism of the number of RNG calls.
    ASSERT_EQ(nextIntSequential, nextIntParallel)
        << "Mutator::RNG was called a different number of time in parallel and "
           "sequential execution.";

    // Check archives
    ASSERT_EQ(laParallel.getArchive().getNbRecordings(),
              laSequential.getArchive().getNbRecordings())
        << "Archives have different sizes.";
    for (auto i = 0; i < laParallel.getArchive().getNbRecordings(); i++) {
        ASSERT_EQ(laParallel.getArchive().at(i).dataHash,
                  laSequential.getArchive().at(i).dataHash)
            << "Archives have different content.";
        ASSERT_EQ(laParallel.getArchive().at(i).result,
                  laSequential.getArchive().at(i).result)
            << "Archives have different content.";
    }
}

TEST_F(adversarialLearningAgentTest, EvalAllRootsParallelValidationDeterminism)
{
    // Check that parallel execution leads to the exact same results as
    // sequential
    params.archiveSize = 50;
    params.archivingProbability = 0.1;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 10;

    Learn::AdversarialLearningAgent la(le, set, params);
    la.init(0); // Reset centralized RNG to 0
    auto results = la.evaluateAllRoots(0, Learn::LearningMode::VALIDATION);
    auto nextInt = la.getRNG().getUnsignedInt64(0, UINT64_MAX);

    Learn::LearningParameters paramsSequential = params;
    paramsSequential.nbThreads = 1;
    Learn::AdversarialLearningAgent laSequential(le, set, paramsSequential);

    laSequential.init(0); // Reset centralized RNG to 0
    auto resultsSequential =
        laSequential.evaluateAllRoots(0, Learn::LearningMode::VALIDATION);
    auto nextIntSequential =
        laSequential.getRNG().getUnsignedInt64(0, UINT64_MAX);

    Learn::LearningParameters paramsParallel = params;
    paramsParallel.nbThreads = 4;
    Learn::AdversarialLearningAgent laParallel(le, set, paramsParallel);

    laParallel.init(0); // Reset centralized RNG to 0
    auto resultsParallel =
        laParallel.evaluateAllRoots(0, Learn::LearningMode::VALIDATION);
    auto nextIntParallel = laParallel.getRNG().getUnsignedInt64(0, UINT64_MAX);

    // Check equality between LearningAgent and ParallelLearningAgent
    ASSERT_EQ(results.size(), resultsSequential.size())
        << "Result maps have a different size.";
    auto iter = results.begin();
    auto iterSequential = resultsSequential.begin();
    while (iter != results.end()) {
        ASSERT_EQ(iter->first->getResult(), iterSequential->first->getResult())
            << "Average score between sequential and parallel executions are "
               "differents.";
        iter++;
        iterSequential++;
    }

    // Check determinism of the number of RNG calls.
    ASSERT_EQ(nextInt, nextIntSequential)
        << "Mutator::RNG was called a different number of time in parallel and "
           "sequential execution.";

    // Check archives
    ASSERT_EQ(la.getArchive().getNbRecordings(), 0)
        << "Archives should be empty in Validation mode.";
    ASSERT_EQ(laSequential.getArchive().getNbRecordings(), 0)
        << "Archives should be empty in Validation mode.";

    // Check equality between ParallelLearningAgent in parallel and sequential
    // mode
    ASSERT_EQ(resultsParallel.size(), resultsParallel.size())
        << "Result maps have a different size.";
    iterSequential = resultsSequential.begin();
    auto iterParallel = resultsParallel.begin();
    while (iterSequential != resultsSequential.end()) {
        ASSERT_EQ(iterSequential->first->getResult(),
                  iterParallel->first->getResult())
            << "Average score between sequential and parallel executions are "
               "differents.";
        iterSequential++;
        iterParallel++;
    }

    // Check determinism of the number of RNG calls.
    ASSERT_EQ(nextIntSequential, nextIntParallel)
        << "Mutator::RNG was called a different number of time in parallel and "
           "sequential execution.";

    // Check archives
    ASSERT_EQ(laParallel.getArchive().getNbRecordings(), 0)
        << "Archives should be empty in Validation mode.";
}