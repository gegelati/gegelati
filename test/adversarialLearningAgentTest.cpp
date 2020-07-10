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

#include "log/LABasicLogger.h"

#include "tpg/tpgGraph.h"

#include "instructions/addPrimitiveType.h"
#include "mutator/rng.h"
#include "mutator/tpgMutator.h"

#include "learn/adversarialLearningAgent.h"
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

    ASSERT_NO_THROW(delete la) << "Destruction of the AdversarialLearningAgent failed.";
}

TEST_F(adversarialLearningAgentTest, MakeJobs)
{
    params.nbIterationsPerPolicyEvaluation=20;
    size_t iterPerJob=2;
    size_t agentsPerEval=5;
    Learn::AdversarialLearningAgent la(le, set, params,agentsPerEval,iterPerJob);
    // 5 agents per job, 2 eval per job
    // => It shall do 10 jobs per agent, and something close to nbRoots*2 (nbRoots*20/(5*2)) jobs at total
    // the complete formula of the nb of jobs is
    la.init();
    auto jobs = la.makeJobs(Learn::TRAINING);
    ASSERT_EQ(la.getTPGGraph().getNbRootVertices()*2,jobs.size())
                                <<"There should be as many jobs as roots.";
    // this map will compute the number of iterations per root that are scheduled
    std::map<const TPG::TPGVertex*, int> nbEvalPerRoot;
    for(auto root : la.getTPGGraph().getRootVertices()){
        nbEvalPerRoot.emplace(root,0);
    }
    while(!jobs.empty()) {
        ASSERT_EQ(5, (*jobs.front()).getSize())
                                    << "job doesn'nt contain the right roots number.";
        // updates number of iterations scheduled for the roots of the job
        for(auto root : (*jobs.front()).getRoots()){
            auto it = nbEvalPerRoot.find(root);
            it->second=it->second+iterPerJob;
        }
        jobs.pop();
    }
    // now check there are enough iterations per root scheduled
    for(auto pairRootNbEval : nbEvalPerRoot){
        ASSERT_GE(pairRootNbEval.second,params.nbIterationsPerPolicyEvaluation)
        << "jobs don't evaluate enough a root.";
    }
}

TEST_F(adversarialLearningAgentTest, EvalRoot)
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
    auto job = *la.makeJob(0);
    ASSERT_NO_THROW(
            result = la.evaluateJob(tee, job,
                                    0, Learn::LearningMode::TRAINING, le))
                                << "Evaluation from a root in no parallel and no adversarial mode failed.";
    ASSERT_LE(result->getResult(), 1.0)
                                << "Average score should not exceed the score of a perfect layer.";


    auto jobs = la.makeJobs(Learn::LearningMode::TRAINING);
    ASSERT_NO_THROW(
            result = la.evaluateJob(tee, *jobs.front(),
                                    0, Learn::LearningMode::TRAINING, le))
                                << "Evaluation from a root in adversarial mode failed.";
    ASSERT_LE(result->getResult(), 1.0)
                                << "Average score should not exceed the score of a perfect layer.";
}
// TODO MakeJob MakeJobs

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
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 5;
    params.ratioDeletedRoots = 0.2;
    params.nbGenerations = 2;
    params.mutation.tpg.nbRoots = 30;
    // A root may be evaluated at most for 3 generations
    params.maxNbEvaluationPerPolicy =
            params.nbIterationsPerPolicyEvaluation * 3;

    Learn::AdversarialLearningAgent la(le, set, params);

    la.init();
    bool alt = false;
    la.train(alt, false);

    // It is quite unlikely that two different TPGs after 20 generations
    // end up with the same number of vertices, roots, edges and calls to
    // the RNG without being identical.
    TPG::TPGGraph& tpg = la.getTPGGraph();
    ASSERT_EQ(tpg.getNbVertices(), 30)
                                << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(tpg.getNbRootVertices(), 27)
                                << "Graph does not have the expected determinist characteristics.";
    ASSERT_EQ(tpg.getEdges().size(), 96)
                                << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(la.getRNG().getUnsignedInt64(0, UINT64_MAX), 5961433082191139529)
                                << "Graph does not have the expected determinst characteristics.";
}

TEST_F(adversarialLearningAgentTest, KeepBestPolicy)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 1;
    params.ratioDeletedRoots = 0.2;
    params.nbGenerations = 5;

    Learn::AdversarialLearningAgent la(le, set, params);
    la.init();
    bool alt = false;
    la.train(alt, true);

    ASSERT_NO_THROW(la.keepBestPolicy())
                                << "Keeping the best policy after training should not fail.";
    ASSERT_EQ(la.getTPGGraph().getNbRootVertices(), 1)
                                << "A single root TPGVertex should remain in the TPGGraph when keeping "
                                   "the best policy only";
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

TEST_F(adversarialLearningAgentTest, TrainOnegenerationSequential)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 3;
    params.ratioDeletedRoots =
            0.95; // high number to force the apparition of root action.
    params.nbThreads = 1;

    auto noCopyLe = StickGameAdversarial(false);

    Learn::AdversarialLearningAgent la(noCopyLe, set, params);

    la.init();
    // Do the populate call to keep know the number of initial vertex
    Archive a(0);
    Mutator::TPGMutator::populateTPG(la.getTPGGraph(), a, params.mutation,
                                     la.getRNG());
    size_t initialNbVertex = la.getTPGGraph().getNbVertices();
    // Seed selected so that an action becomes a root during next generation
    ASSERT_NO_THROW(la.trainOneGeneration(4))
                                << "Training for one generation failed.";
    // Check the number of vertex in the graph.
    // Must be initial number of vertex - number of root removed
    ASSERT_EQ(la.getTPGGraph().getNbVertices(),
              initialNbVertex -
              floor(params.ratioDeletedRoots * params.mutation.tpg.nbRoots))
                                << "Number of remaining is under the number of roots from the "
                                   "TPGGraph.";
    // Train a second generation, because most roots were removed, a root
    // actions have appeared and the training algorithm will attempt to remove
    // them.
    ASSERT_NO_THROW(la.trainOneGeneration(0))
                                << "Training for one generation failed.";
}

TEST_F(adversarialLearningAgentTest, TrainOneGenerationParallel)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 3;
    params.ratioDeletedRoots =
            0.95; // high number to force the apparition of root action.
    params.nbThreads = 4;

    Learn::AdversarialLearningAgent la(le, set, params);

    la.init();
    // Do the populate call to keep know the number of initial vertex
    Archive a(0);
    Mutator::TPGMutator::populateTPG(la.getTPGGraph(), a, params.mutation,
                                     la.getRNG());
    size_t initialNbVertex = la.getTPGGraph().getNbVertices();
    // Seed selected so that an action becomes a root during next generation
    ASSERT_NO_THROW(la.trainOneGeneration(4))
                                << "Training for one generation failed.";
    // Check the number of vertex in the graph.
    // Must be initial number of vertex - number of root removed
    ASSERT_EQ(la.getTPGGraph().getNbVertices(),
              initialNbVertex -
              floor(params.ratioDeletedRoots * params.mutation.tpg.nbRoots))
                                << "Number of remaining is under the number of roots from the "
                                   "TPGGraph.";
}

TEST_F(adversarialLearningAgentTest, TrainSequential)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 5;
    params.ratioDeletedRoots = 0.2;
    params.nbGenerations = 3;
    params.maxNbEvaluationPerPolicy =
            params.nbIterationsPerPolicyEvaluation * 2;
    params.nbThreads = 1;

    Learn::AdversarialLearningAgent la(le, set, params);

    la.init();
    bool alt = false;

    ASSERT_NO_THROW(la.train(alt, true))
                                << "Training a TPG for several generation should not fail.";
    alt = true;
    ASSERT_NO_THROW(la.train(alt, true))
                                << "Using the boolean reference to stop the training should not fail.";
}

TEST_F(adversarialLearningAgentTest, TrainParallel)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 5;
    params.ratioDeletedRoots = 0.2;
    params.nbGenerations = 3;
    params.maxNbEvaluationPerPolicy =
            params.nbIterationsPerPolicyEvaluation * 2;
    params.nbThreads = std::thread::hardware_concurrency();

    Learn::AdversarialLearningAgent la(le, set, params);

    la.init();
    bool alt = false;

    ASSERT_NO_THROW(la.train(alt, true))
                                << "Training a TPG for several generation should not fail.";
    alt = true;
    ASSERT_NO_THROW(la.train(alt, true))
                                << "Using the boolean reference to stop the training should not fail.";
}

TEST_F(adversarialLearningAgentTest, TrainParallelDeterminism)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 5;
    params.ratioDeletedRoots = 0.2;
    // Set a large number of generations and roots
    // so that the chances of something going wrong is higher.
    params.nbGenerations = 20;
    params.mutation.tpg.nbRoots = 30;
    params.maxNbEvaluationPerPolicy =
            params.nbIterationsPerPolicyEvaluation * 5;

    Learn::AdversarialLearningAgent la(le, set, params);

    la.init();

    // Train for several generation
    bool alt = false;
    la.train(alt, false);

    params.nbThreads = 4;

    la.init();

    // Train for several generation
    la.train(alt, false);

    // Check number of vertex in graphs
    // Non-zero to avoid false positive.
    // These checks guarantee determinism between sequential and parallel
    // version on a given latform. They do not guarantee portability between
    // compilers and OS
    ASSERT_GT(la.getTPGGraph().getNbVertices(), 0)
                                << "Number of vertex in the trained graph should not be 0.";
    ASSERT_EQ(la.getTPGGraph().getNbVertices(),
              la.getTPGGraph().getNbVertices())
                                << "LearningAgent and ParallelLearning agent result in different "
                                   "TPGGraphs.";
}
