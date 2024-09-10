/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2023) :
 *
 * QuentinVacher <98522623+QuentinVacher-rl@users.noreply.github.com> (2023)
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2023)
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

#include <algorithm>
#include <fstream>
#include <gtest/gtest.h>
#include <numeric>

#include "log/laBasicLogger.h"

#include "tpg/instrumented/tpgActionInstrumented.h"
#include "tpg/instrumented/tpgEdgeInstrumented.h"
#include "tpg/instrumented/tpgInstrumentedFactory.h"
#include "tpg/instrumented/tpgTeamInstrumented.h"
#include "tpg/instrumented/tpgVertexInstrumentation.h"
#include "tpg/policyStats.h"
#include "tpg/tpgGraph.h"

#include "instructions/addPrimitiveType.h"
#include "mutator/rng.h"
#include "mutator/tpgMutator.h"

#include "learn/gridWorld.h"
#include "learn/learningAgent.h"
#include "learn/learningEnvironment.h"
#include "learn/learningParameters.h"
#include "learn/parallelLearningAgent.h"
#include "learn/stickGameWithOpponent.h"

class LearningAgentTest : public ::testing::Test
{
  protected:
    Instructions::Set set;
    StickGameWithOpponent le;
    GridWorld marlLe;
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

class ParallelLearningAgentTest : public LearningAgentTest
{
};

TEST_F(LearningAgentTest, Constructor)
{
    Learn::LearningAgent* la;

    ASSERT_NO_THROW(la = new Learn::LearningAgent(le, set, params))
        << "Construction of the learningAgent failed.";

    ASSERT_NO_THROW(delete la) << "Destruction of the LearningAgent failed.";
}

TEST_F(LearningAgentTest, Init)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    Learn::LearningAgent la(le, set, params);

    ASSERT_NO_THROW(la.init())
        << "Initialization of the LearningAgent should not fail.";
}

TEST_F(LearningAgentTest, InitNbRoots)
{
    params.mutation.tpg.initNbRoots = 42;
    Learn::LearningAgent la(le, set, params);

    la.init();

    ASSERT_EQ(la.getTPGGraph()->getNbRootVertices(), 42)
        << "Initialization of the LearningAgent should have a number of roots "
           "equal to the number specify";
}

TEST_F(LearningAgentTest, InitNbEdgesAvailable)
{
    params.mutation.tpg.initNbRoots = 42;
    Learn::LearningAgent la(le, set, params);

    la.init();

    ASSERT_EQ(la.getParams().nbEdgesActivable, 1)
        << "Parameters of LearningAgent after initialisation should have the "
           "attribute nbEdgesActivable set to 1 for single action environment.";

    Learn::LearningAgent marlLa(marlLe, set, params);

    marlLa.init();

    ASSERT_EQ(marlLa.getParams().nbEdgesActivable, 2)
        << "Parameters of LearningAgent after initialisation should have the "
           "attribute nbEdgesActivable set to 2 for multi action environment.";
}

TEST_F(LearningAgentTest, setParams)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    Learn::LearningAgent la(le, set, params);

    params.archiveSize = 100;

    ASSERT_EQ(la.getParams().archiveSize, 50)
        << "Parameters of LearningAgent should not have changed.";
    la.setParams(params);

    ASSERT_EQ(la.getParams().archiveSize, 100)
        << "Parameters of LearningAgent should have changed.";
}

TEST_F(LearningAgentTest, addLogger)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    Learn::LearningAgent la(le, set, params);

    Log::LALogger* l = nullptr;
    ASSERT_NO_THROW(
        l = new Log::LABasicLogger(la, std::cout)) // Call addLogger.
        << "Adding a logger should not fail.";
    if (l != nullptr) {
        delete l;
    }
}

TEST_F(LearningAgentTest, IsRootEvalSkipped)
{
    params.maxNbEvaluationPerPolicy = 2;

    Learn::LearningAgent la(le, set, params);
    la.init();

    // Test a new root
    std::shared_ptr<Learn::EvaluationResult> result1;
    ASSERT_FALSE(la.isRootEvalSkipped(
        *la.getTPGGraph()->getRootVertices().at(0), result1))
        << "Method should return false for a root that has never been "
           "evaluated before.";
    ASSERT_EQ(result1, nullptr) << "Method should return a nullptr for a root "
                                   "that has not been evaluated before.";

    // Add an EvaluationResult artificially
    result1 = std::make_shared<Learn::EvaluationResult>(1.0, 1);
    la.updateEvaluationRecords(
        {{result1, la.getTPGGraph()->getRootVertices().at(0)}});

    // Test the root again
    std::shared_ptr<Learn::EvaluationResult> result2;
    ASSERT_FALSE(la.isRootEvalSkipped(
        *la.getTPGGraph()->getRootVertices().at(0), result2))
        << "Method should return false for a root that has been evaluated "
           "before.";
    ASSERT_EQ(result2, result1)
        << "Method should return a valid pointer for a root that has not been "
           "evaluated enough times before.";

    // Update the EvaluationResult artificially
    result2 = std::make_shared<Learn::EvaluationResult>(1.0, 2);
    la.updateEvaluationRecords(
        {{result2, la.getTPGGraph()->getRootVertices().at(0)}});

    // Test the root again.
    std::shared_ptr<Learn::EvaluationResult> result3;
    ASSERT_TRUE(la.isRootEvalSkipped(*la.getTPGGraph()->getRootVertices().at(0),
                                     result3))
        << "Method should return true for a root that has been evaluated "
           "before more times than maxNbEvaluationPerPolicy.";
    ASSERT_EQ(result3, result2)
        << "Method should return a the EvaluationResult from the "
           "resultsPerRoot map when the number of evaluation exceeds "
           "maxNbEvaluationPerPolicy.";
}

TEST_F(LearningAgentTest, MakeJob)
{
    Learn::LearningAgent la(le, set, params);
    la.init();
    auto job = *la.makeJob(la.getTPGGraph()->getRootVertices().at(0),
                           Learn::LearningMode::TRAINING);
    ASSERT_NO_THROW(job.getArchiveSeed()) << "job should have an archive seed";
    ASSERT_NO_THROW(job.getIdx()) << "job should have an idx";
    ASSERT_EQ(la.getTPGGraph()->getRootVertices().at(0), job.getRoot())
        << "Encapsulate the root in a job shouldn't change it";

    Learn::LearningAgent la2(le, set, params);
    auto job2 = la2.makeJob(nullptr, Learn::LearningMode::TRAINING);
    ASSERT_EQ(nullptr, job2)
        << "Create a job when no root should return nullptr";
}

TEST_F(LearningAgentTest, MakeJobs)
{
    Learn::LearningAgent la(le, set, params);
    la.init();
    auto jobs = la.makeJobs(Learn::LearningMode::TRAINING);
    ASSERT_EQ(la.getTPGGraph()->getNbRootVertices(), jobs.size())
        << "There should be as many jobs as roots";
    for (int i = 0; i < la.getTPGGraph()->getNbRootVertices(); i++) {
        ASSERT_EQ(la.getTPGGraph()->getRootVertices().at(i),
                  (*jobs.front()).getRoot())
            << "Encapsulate the root in a job shouldn't change it";
        jobs.pop();
    }
}

TEST_F(LearningAgentTest, EvalRoot)
{
    params.archiveSize = 50;
    params.archivingProbability = 1.0;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 10;

    Learn::LearningAgent la(le, set, params);
    Archive a; // For testing purposes, notmally, the archive from the
               // LearningAgent is used.

    TPG::TPGExecutionEngine tee(la.getTPGGraph()->getEnvironment(), &a);

    la.init();
    std::shared_ptr<Learn::EvaluationResult> result;
    auto job = *la.makeJob(la.getTPGGraph()->getRootVertices().at(0),
                           Learn::LearningMode::TRAINING);
    ASSERT_NO_THROW(
        result = la.evaluateJob(tee, job, 0, Learn::LearningMode::TRAINING, le))
        << "Evaluation from a root failed.";
    ASSERT_LE(result->getResult(), 1.0)
        << "Average score should not exceed the score of a perfect player.";
}

TEST_F(LearningAgentTest, EvaluateOneRoot)
{
    params.archiveSize = 50;
    params.archivingProbability = 1.0;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 10;

    Learn::LearningAgent la(le, set, params);
    Archive a; // For testing purposes, normally, the archive from the
               // LearningAgent is used.

    la.init();

    std::shared_ptr<Learn::EvaluationResult> result;
    ASSERT_NO_THROW(
        result = la.evaluateOneRoot(0, Learn::LearningMode::TRAINING,
                                    la.getTPGGraph()->getRootVertices().at(0)))
        << "Evaluation from a root failed.";
    ASSERT_LE(result->getResult(), 1.0)
        << "Average score should not exceed the score of a perfect player.";
}

TEST_F(LearningAgentTest, EvalAllRoots)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 10;

    Learn::LearningAgent la(le, set, params);

    la.init();
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const TPG::TPGVertex*>
        result;
    ASSERT_NO_THROW(result =
                        la.evaluateAllRoots(0, Learn::LearningMode::TRAINING))
        << "Evaluation from a root failed.";
    ASSERT_EQ(result.size(), la.getTPGGraph()->getNbRootVertices())
        << "Number of evaluated roots is under the number of roots from the "
           "TPGGraph.";
}

TEST_F(LearningAgentTest, GetArchive)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 10;

    Learn::LearningAgent la(le, set, params);

    la.init();
    la.evaluateAllRoots(0, Learn::LearningMode::TRAINING);

    ASSERT_NO_THROW(la.getArchive())
        << "Cannot get the archive of a LearningAgent.";
}

TEST_F(LearningAgentTest, GetEnvironment)
{
    Learn::LearningAgent la(le, set, params);

    const Environment* env;
    ASSERT_NO_THROW(env = &la.getEnvironment())
        << "Getting the environment of the learning agent failed unexpectedly.";
}

TEST_F(LearningAgentTest, UpdateEvaluationRecords)
{
    // test bestRoot methods
    Learn::LearningAgent la(le, set, params);

    // Check null at build
    ASSERT_EQ(la.getBestRoot().first, nullptr)
        << "Best root should be a nullptr after building a new LearningAgent.";
    ASSERT_EQ(la.getBestRoot().second, nullptr)
        << "Best root EvaluationResult should be a nullptr after building a "
           "new LearningAgent.";

    // Check null at init
    la.init();
    ASSERT_EQ(la.getBestRoot().first, nullptr)
        << "Best root should be a nullptr after init of a LearningAgent.";
    ASSERT_EQ(la.getBestRoot().second, nullptr)
        << "Best root EvaluationResult should be a nullptr after init of a "
           "LearningAgent.";

    // Update with a fake result for a root of the graph
    auto rootVertices = la.getTPGGraph()->getRootVertices();
    const TPG::TPGVertex* root = *rootVertices.begin();
    ASSERT_NO_THROW(la.updateEvaluationRecords(
        {{std::make_shared<Learn::EvaluationResult>(1.0, 10), root}}));
    ASSERT_EQ(la.getBestRoot().first, root)
        << "Best root not updated properly.";
    ASSERT_EQ(la.getBestRoot().second->getResult(), 1.0)
        << "Best root not updated properly.";

    // Update with a fake better result for another root of the graph
    const TPG::TPGVertex* root2 =
        *(la.getTPGGraph()->getRootVertices().begin() + 1);
    ASSERT_NO_THROW(la.updateEvaluationRecords(
        {{std::make_shared<Learn::EvaluationResult>(2.0, 10), root2}}));
    ASSERT_EQ(la.getBestRoot().first, root2)
        << "Best root not updated properly.";
    ASSERT_EQ(la.getBestRoot().second->getResult(), 2.0)
        << "Best root not updated properly.";

    // Update with a fake worse result for another root of the graph
    const TPG::TPGVertex* root3 =
        *(la.getTPGGraph()->getRootVertices().begin() + 2);
    ASSERT_NO_THROW(la.updateEvaluationRecords(
        {{std::make_shared<Learn::EvaluationResult>(1.5, 10), root3}}));
    ASSERT_EQ(la.getBestRoot().first, root2)
        << "Best root not updated properly.";
    ASSERT_EQ(la.getBestRoot().second->getResult(), 2.0)
        << "Best root not updated properly.";

    // Update with a root not from the graph
    TPG::TPGTeam fakeRoot;
    ASSERT_NO_THROW(la.updateEvaluationRecords(
        {{std::make_shared<Learn::EvaluationResult>(3.0, 10), &fakeRoot}}));
    ASSERT_EQ(la.getBestRoot().first, &fakeRoot)
        << "Best root not updated properly.";
    ASSERT_EQ(la.getBestRoot().second->getResult(), 3.0)
        << "Best root not updated properly.";

    // Update with a worse EvaluationResult (but still updated because previous
    // Root is not in the TPGGraph
    auto sharedPtr = std::make_shared<Learn::EvaluationResult>(1.5, 10);
    ASSERT_NO_THROW(la.updateEvaluationRecords({{sharedPtr, root3}}));
    ASSERT_EQ(la.getBestRoot().first, root3)
        << "Best root not updated properly.";
    ASSERT_EQ(la.getBestRoot().second->getResult(), 1.5)
        << "Best root not updated properly.";

    // Update with the EvaluationResult already registered in the resultsPerRoot
    // map (for code coverage)
    ASSERT_NO_THROW(la.updateEvaluationRecords({{sharedPtr, root3}}));
}

TEST_F(LearningAgentTest, forgetPreviousResults)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 10;
    params.mutation.tpg.maxInitOutgoingEdges = 2;
    params.ratioDeletedRoots = 0.50;
    params.mutation.tpg.nbRoots = 10;
    params.nbRegisters = 4;

    Learn::LearningAgent la(le, set, params);
    la.init();

    // Update with a fake result for a root of the graph
    auto rootVertices = la.getTPGGraph()->getRootVertices();
    const TPG::TPGVertex* root = *rootVertices.begin();
    ASSERT_NO_THROW(la.updateEvaluationRecords(
        {{std::make_shared<Learn::EvaluationResult>(1.0, 10), root}}));
    ASSERT_EQ(la.getBestRoot().second->getResult(), 1.0)
        << "Best root not updated properly.";
    ASSERT_NO_THROW(*la.getBestRoot().second +=
                    Learn::EvaluationResult(2.0, 10));
    ASSERT_EQ(la.getBestRoot().second->getResult(), 1.5)
        << "Best root not updated properly.";

    // Looks for the eval record the Learning Agent should keep
    std::shared_ptr<Learn::EvaluationResult> previousEval;
    la.isRootEvalSkipped(*la.getBestRoot().first, previousEval);

    ASSERT_NE(nullptr, previousEval)
        << "Learning agent should remember the last score of the root.";

    // Forgets the eval record
    ASSERT_NO_THROW(la.forgetPreviousResults())
        << "forgetPreviousResults throws exception.";

    // Looks for the eval record the Learning Agent should keep
    la.isRootEvalSkipped(*la.getBestRoot().first, previousEval);

    ASSERT_EQ(nullptr, previousEval)
        << "Learning agent should have forgotten the last score of the root";

    ASSERT_EQ(nullptr, la.getBestRoot().first)
        << "Learning agent should have forgotten the best root";

    ASSERT_EQ(nullptr, la.getBestRoot().second)
        << "Learning agent should have forgotten the last score of the root";

    ASSERT_NO_THROW(la.trainOneGeneration(0))
        << "trainOneGeneration doesn't work after a forgetPreviousResults";
}

TEST_F(LearningAgentTest, DecimateWorstRoots)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 3;
    params.mutation.tpg.maxInitOutgoingEdges = 2;
    params.ratioDeletedRoots = 0.50;
    params.mutation.tpg.nbRoots =
        le.getNbActions() - 1; // Param used in decimation
    params.nbRegisters = 4;

    Learn::LearningAgent la(le, set, params);

    la.init();

    // Remove two teams (first and last) to make the first action a root
    TPG::TPGGraph& graph = *la.getTPGGraph();
    auto roots = graph.getRootVertices();
    graph.removeVertex(*roots.at(0));
    graph.removeVertex(*roots.at(le.getNbActions() - 1));

    // Check that the action is now a root
    roots = graph.getRootVertices();
    auto* root = roots.at(0);
    ASSERT_EQ(typeid(*root), typeid(TPG::TPGAction))
        << "An action should have become a root of the TPGGraph.";

    // Create and fill results for each "root" artificially
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const TPG::TPGVertex*>
        results;
    double result = 0.0;
    for (const TPG::TPGVertex* root : roots) {
        results.emplace(new Learn::EvaluationResult(result++, 5), root);
    }

    // Do the decimation
    ASSERT_NO_THROW(la.decimateWorstRoots(results))
        << "Decimating worst roots failed.";

    // Check the number of remaining roots.
    // Initial number of vertex - 2 removed vertices - deleted roots.
    ASSERT_EQ(la.getTPGGraph()->getNbVertices(),
              (le.getNbActions() * 2) - 2 -
                  params.ratioDeletedRoots * ((le.getNbActions() - 1)));
}

TEST_F(LearningAgentTest, TrainOnegeneration)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 3;
    params.ratioDeletedRoots =
        0.95; // high number to force the apparition of root action.

    // we will validate in order to cover validation log
    params.doValidation = true;

    Learn::LearningAgent la(le, set, params);

    la.init();

    // we add a logger to la to check it logs things
    std::ofstream o("tempFileForTest", std::ofstream::out);
    Log::LABasicLogger l(la, o);

    // Do the populate call to keep know the number of initial vertex
    Archive a(0);
    Mutator::TPGMutator::populateTPG(*la.getTPGGraph(), a, params.mutation,
                                     la.getRNG(), le.getVectActions(), 1);
    size_t initialNbVertex = la.getTPGGraph()->getNbVertices();
    // Seed selected so that an action becomes a root during next generation
    ASSERT_NO_THROW(la.trainOneGeneration(4))
        << "Training for one generation failed.";
    // Check the number of vertex in the graph.
    // Must be initial number of vertex - number of root removed
    ASSERT_EQ(la.getTPGGraph()->getNbVertices(),
              initialNbVertex -
                  floor(params.ratioDeletedRoots * params.mutation.tpg.nbRoots))
        << "Number of remaining is under the number of roots from the "
           "TPGGraph.";
    // Train a second generation, because most roots were removed, a root
    // actions have appeared and the training algorithm will attempt to remove
    // them.
    ASSERT_NO_THROW(la.trainOneGeneration(0))
        << "Training for one generation failed.";

    // Check that bestScoreLastGen has been set
    ASSERT_NE(la.getBestScoreLastGen(), 0.0);

    // Check that bestRoot has been set
    ASSERT_NE(la.getBestRoot().first, nullptr);

    o.close();
    std::ifstream i("tempFileForTest", std::ofstream::in);
    std::string s;
    i >> s;
    ASSERT_TRUE(s.size() > 0) << "Logger should have logged elements after a "
                                 "trainOneGeneration iteration.";
    i.close();
    // removing the temporary file
    remove("tempFileForTest");
}

TEST_F(LearningAgentTest, Train)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 5;
    params.ratioDeletedRoots = 0.2;
    params.nbGenerations = 3;

    Learn::LearningAgent la(le, set, params);

    la.init();
    bool alt = false;

    ASSERT_NO_THROW(la.train(alt, true))
        << "Training a TPG for several generation should not fail.";
    alt = true;
    ASSERT_NO_THROW(la.train(alt, true))
        << "Using the boolean reference to stop the training should not fail.";
}

// Similar to previous test, but verifications of graphs properties are here to
// ensure the result of the training is identical on all OSes and Compilers.
TEST_F(LearningAgentTest, TrainPortability)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 5;
    params.ratioDeletedRoots = 0.2;
    params.nbGenerations = 20;
    params.mutation.tpg.nbRoots = 30;
    // A root may be evaluated at most for 3 generations
    params.maxNbEvaluationPerPolicy =
        params.nbIterationsPerPolicyEvaluation * 3;
    params.mutation.tpg.forceProgramBehaviorChangeOnMutation = true;

    Learn::LearningAgent la(le, set, params);

    la.init();
    bool alt = false;
    la.train(alt, false);

    // It is quite unlikely that two different TPGs after 20 generations
    // end up with the same number of vertices, roots, edges and calls to
    // the RNG without being identical.
    TPG::TPGGraph& tpg = *la.getTPGGraph();
    ASSERT_EQ(tpg.getNbVertices(), 29)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(tpg.getNbRootVertices(), 25)
        << "Graph does not have the expected determinist characteristics.";
    ASSERT_EQ(tpg.getEdges().size(), 92)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(la.getRNG().getUnsignedInt64(0, UINT64_MAX), 8778232462724898875)
        << "Graph does not have the expected determinst characteristics.";
}

// Same as previous, but with a TPGInstrumentedFactory
TEST_F(LearningAgentTest, TrainInstrumented)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 5;
    params.ratioDeletedRoots = 0.2;
    params.nbGenerations = 20;
    params.mutation.tpg.nbRoots = 30;
    // A root may be evaluated at most for 3 generations
    params.maxNbEvaluationPerPolicy =
        params.nbIterationsPerPolicyEvaluation * 3;
    params.mutation.tpg.forceProgramBehaviorChangeOnMutation = true;

    Learn::LearningAgent la(le, set, params, TPG::TPGInstrumentedFactory());

    la.init();
    bool alt = false;
    la.train(alt, false);

    // It is quite unlikely that two different TPGs after 20 generations
    // end up with the same number of vertices, roots, edges and calls to
    // the RNG without being identical.
    TPG::TPGGraph& tpg = *la.getTPGGraph();
    ASSERT_EQ(tpg.getNbVertices(), 29)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(tpg.getNbRootVertices(), 25)
        << "Graph does not have the expected determinist characteristics.";
    ASSERT_EQ(tpg.getEdges().size(), 92)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(la.getRNG().getUnsignedInt64(0, UINT64_MAX), 8778232462724898875)
        << "Graph does not have the expected determinst characteristics.";

    /*
    //To help to refind the values if the determinism is changed by a update
    for (const auto& edge : tpg.getEdges()) {
        const TPG::TPGEdgeInstrumented* edgeInstrumented =
            dynamic_cast<const TPG::TPGEdgeInstrumented*>(edge.get());

        if (edgeInstrumented != nullptr) {
            std::cout << "NbVisits = " << edgeInstrumented->getNbVisits()
                    << ", NbTraversal = " << edgeInstrumented->getNbTraversal()
                    << std::endl;
        }
    }
    for(auto vert: tpg.getVertices()){
        std::cout<<dynamic_cast<const
    TPG::TPGVertexInstrumentation*>(vert)->getNbVisits()<<std::endl;
    }*/

    // Check number of visits of a few edges & vertices
    auto edgesIterator = tpg.getEdges().begin();
    const auto* edge1 = edgesIterator->get();

    ASSERT_EQ(
        dynamic_cast<const TPG::TPGEdgeInstrumented*>(edge1)->getNbVisits(),
        304);
    ASSERT_EQ(
        dynamic_cast<const TPG::TPGEdgeInstrumented*>(edge1)->getNbTraversal(),
        0);

    std::advance(edgesIterator, 3);
    const auto* edge2 = edgesIterator->get();

    ASSERT_EQ(
        dynamic_cast<const TPG::TPGEdgeInstrumented*>(edge2)->getNbVisits(),
        107);
    ASSERT_EQ(
        dynamic_cast<const TPG::TPGEdgeInstrumented*>(edge2)->getNbTraversal(),
        107);

    auto& verticesIterator = tpg.getVertices();
    ASSERT_EQ(dynamic_cast<const TPG::TPGVertexInstrumentation*>(
                  verticesIterator.at(0))
                  ->getNbVisits(),
              5533);

    ASSERT_EQ(dynamic_cast<const TPG::TPGVertexInstrumentation*>(
                  verticesIterator.at(5))
                  ->getNbVisits(),
              107);
}

// Similar to previous test, but verifications of graphs properties are here to
// ensure the result of the training is identical on all OSes and Compilers,
// even for multi-action cases.
TEST_F(LearningAgentTest, TrainMARLPortability)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 5;
    params.ratioDeletedRoots = 0.2;
    params.nbGenerations = 20;
    params.mutation.tpg.nbRoots = 30;
    // A root may be evaluated at most for 3 generations
    params.maxNbEvaluationPerPolicy =
        params.nbIterationsPerPolicyEvaluation * 3;

    Learn::LearningAgent la(marlLe, set, params);

    la.init();
    bool alt = false;
    la.train(alt, false);

    // It is quite unlikely that two different TPGs after 20 generations
    // end up with the same number of vertices, roots, edges and calls to
    // the RNG without being identical.
    TPG::TPGGraph& tpg = *la.getTPGGraph();
    ASSERT_EQ(tpg.getNbVertices(), 37)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(tpg.getNbRootVertices(), 25)
        << "Graph does not have the expected determinist characteristics.";
    ASSERT_EQ(tpg.getEdges().size(), 119)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(la.getRNG().getUnsignedInt64(0, UINT64_MAX), 7086172217659134612)
        << "Graph does not have the expected determinst characteristics.";
}

TEST_F(LearningAgentTest, KeepBestPolicy)
{
    params.archiveSize = 1;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 1;
    params.ratioDeletedRoots = 0.2;
    params.nbGenerations = 5;

    Learn::LearningAgent la(le, set, params);
    la.init();
    bool alt = false;
    la.train(alt, true);

    ASSERT_NO_THROW(la.keepBestPolicy())
        << "Keeping the best policy after training should not fail.";
    ASSERT_EQ(la.getTPGGraph()->getNbRootVertices(), 1)
        << "A single root TPGVertex should remain in the TPGGraph when keeping "
           "the best policy only";
}

TEST_F(LearningAgentTest, TPGGraphCleanProgramIntrons)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 1;
    params.ratioDeletedRoots = 0.2;
    params.nbGenerations = 5;

    Learn::LearningAgent la(le, set, params);
    la.init();
    bool alt = false;
    la.train(alt, false);

    la.keepBestPolicy();

    TPG::TPGGraph& tpg = *la.getTPGGraph();

    // Get policy stats
    TPG::PolicyStats psOrigin;
    psOrigin.setEnvironment(tpg.getEnvironment());
    psOrigin.analyzePolicy(tpg.getRootVertices().at(0));

    // Check the presence of introns
    ASSERT_GT(std::accumulate(psOrigin.nbIntronPerProgram.begin(),
                              psOrigin.nbIntronPerProgram.end(), size_t(0)),
              0)
        << "TPGGraph has no introns in its programs after training.";

    // Record the behavior of the TPG with introns
    le.reset();
    TPG::TPGExecutionEngine tee(tpg.getEnvironment());
    std::vector<const TPG::TPGVertex*> pathOrigin =
        tee.executeFromRoot(*(tpg.getRootVertices().at(0)), le.getInitActions(),
                            params.nbEdgesActivable)
            .first;

    // Clear introns
    tpg.clearProgramIntrons();

    // Get new policy stats
    TPG::PolicyStats psNoIntrons;
    psNoIntrons.setEnvironment(tpg.getEnvironment());
    psNoIntrons.analyzePolicy(tpg.getRootVertices().at(0));

    // Compare
    ASSERT_EQ(std::accumulate(psNoIntrons.nbIntronPerProgram.begin(),
                              psNoIntrons.nbIntronPerProgram.end(), size_t(0)),
              0)
        << "TPGGraph still contains introns after they were cleared.";

    // Check that the behavior is identical (empirically, not really foolproof)
    std::vector<const TPG::TPGVertex*> pathNoIntrons =
        tee.executeFromRoot(*(tpg.getRootVertices().at(0)), le.getInitActions(),
                            params.nbEdgesActivable)
            .first;

    ASSERT_EQ(pathOrigin.size(), pathNoIntrons.size())
        << "Path length in TPG before and after inton removal is not "
           "identical.";
    for (auto idx = 0; idx < pathOrigin.size(); idx++) {
        ASSERT_EQ(pathOrigin.at(idx), pathNoIntrons.at(idx))
            << "Path element in TPGGraph changed when removing introns.";
    }
}

TEST_F(ParallelLearningAgentTest, Constructor)
{
    Learn::ParallelLearningAgent* pla;

    ASSERT_NO_THROW(pla = new Learn::ParallelLearningAgent(le, set, params))
        << "Construction of the learningAgent failed.";

    ASSERT_NO_THROW(delete pla) << "Destruction of the LearningAgent failed.";
}

TEST_F(ParallelLearningAgentTest, Init)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    Learn::ParallelLearningAgent pla(le, set, params);

    ASSERT_NO_THROW(pla.init())
        << "Initialization of the LearningAgent should not fail.";
}

TEST_F(ParallelLearningAgentTest, EvalRootSequential)
{
    params.archiveSize = 50;
    params.archivingProbability = 1.0;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 10;
    params.nbThreads = 1;

    Environment env(set, le.getDataSources(), 8, params.nbProgramConstant);

    TPG::TPGGraph tpg(env);

    // Initialize Randomness
    Mutator::RNG rng;
    rng.setSeed(0);

    params.mutation.tpg.initNbRoots = le.getNbActions();

    // Initialize the tpg
    Mutator::TPGMutator::initRandomTPG(tpg, params.mutation, rng,
                                       le.getVectActions());

    // create the archive
    Archive archive;

    // The TPGExecutionEngine
    TPG::TPGExecutionEngine tee(env, &archive);

    std::shared_ptr<Learn::EvaluationResult> result;
    Learn::ParallelLearningAgent pla(le, set, params);
    ASSERT_NO_THROW(result = pla.evaluateJob(
                        tee,
                        *pla.makeJob(tpg.getRootVertices().at(0),
                                     Learn::LearningMode::TRAINING, 0, &tpg),
                        0, Learn::LearningMode::TRAINING, le))
        << "Evaluation from a root failed.";
    ASSERT_LE(result->getResult(), 1.0)
        << "Average score should not exceed the score of a perfect player.";
}

TEST_F(ParallelLearningAgentTest, EvalAllRootsSequential)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 10;
    params.nbThreads = 1;

    Learn::ParallelLearningAgent pla(le, set, params);

    pla.init();
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const TPG::TPGVertex*>
        result;
    ASSERT_NO_THROW(result =
                        pla.evaluateAllRoots(0, Learn::LearningMode::TRAINING))
        << "Evaluation from a root failed.";
    ASSERT_EQ(result.size(), pla.getTPGGraph()->getNbRootVertices())
        << "Number of evaluated roots is under the number of roots from the "
           "TPGGraph.";
}

TEST_F(ParallelLearningAgentTest, EvalAllRootsParallel)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 10;
    params.nbThreads = 4;

    Learn::ParallelLearningAgent pla(le, set, params);

    pla.init();
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const TPG::TPGVertex*>
        result;
    ASSERT_NO_THROW(result =
                        pla.evaluateAllRoots(0, Learn::LearningMode::TRAINING))
        << "Evaluation from a root failed.";
    ASSERT_EQ(result.size(), pla.getTPGGraph()->getNbRootVertices())
        << "Number of evaluated roots is under the number of roots from the "
           "TPGGraph.";
}

TEST_F(ParallelLearningAgentTest, EvalAllRootsParallelTrainingDeterminism)
{
    // Check that parallel execution leads to the exact same results as
    // sequential
    params.archiveSize = 50;
    params.archivingProbability = 0.1;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 10;

    Learn::LearningAgent la(le, set, params);
    la.init(0); // Reset RNG to 0
    auto results = la.evaluateAllRoots(0, Learn::LearningMode::TRAINING);
    auto nextInt = la.getRNG().getUnsignedInt64(0, UINT64_MAX);

    Learn::LearningParameters paramsSequential = params;
    paramsSequential.nbThreads = 1;
    Learn::ParallelLearningAgent plaSequential(le, set, paramsSequential);

    plaSequential.init(0); // Reset centralized RNG to 0
    auto resultsSequential =
        plaSequential.evaluateAllRoots(0, Learn::LearningMode::TRAINING);
    auto nextIntSequential =
        plaSequential.getRNG().getUnsignedInt64(0, UINT64_MAX);

    Learn::LearningParameters paramsParallel = params;
    paramsParallel.nbThreads = 4;
    Learn::ParallelLearningAgent plaParallel(le, set, paramsParallel);

    plaParallel.init(0); // Reset centralized RNG to 0
    auto resultsParallel =
        plaParallel.evaluateAllRoots(0, Learn::LearningMode::TRAINING);
    auto nextIntParallel = plaParallel.getRNG().getUnsignedInt64(0, UINT64_MAX);

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

    // Check determinism of bestScoreLastGen
    ASSERT_EQ(la.getBestScoreLastGen(), plaSequential.getBestScoreLastGen());

    // Check determinism of bestRoot score
    ASSERT_EQ(la.getBestRoot().second, plaSequential.getBestRoot().second);

    // Check determinism of the number of RNG calls.
    ASSERT_EQ(nextInt, nextIntSequential)
        << "Mutator::RNG was called a different number of time in parallel and "
           "sequential execution.";

    // Check archives
    ASSERT_GT(la.getArchive().getNbRecordings(), 0)
        << "For the archive determinism tests to be meaningful, Archive should "
           "not be empty.";
    ASSERT_EQ(la.getArchive().getNbRecordings(),
              plaSequential.getArchive().getNbRecordings())
        << "Archives have different sizes.";
    for (auto i = 0; i < la.getArchive().getNbRecordings(); i++) {
        ASSERT_EQ(la.getArchive().at(i).dataHash,
                  plaSequential.getArchive().at(i).dataHash)
            << "Archives have different content.";
        ASSERT_EQ(la.getArchive().at(i).result,
                  plaSequential.getArchive().at(i).result)
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

    // Check determinism of bestScoreLastGen
    ASSERT_EQ(plaSequential.getBestScoreLastGen(),
              plaParallel.getBestScoreLastGen());

    // Check determinism of bestRoot score
    ASSERT_EQ(plaSequential.getBestRoot().second,
              plaParallel.getBestRoot().second);

    // Check determinism of the number of RNG calls.
    ASSERT_EQ(nextIntSequential, nextIntParallel)
        << "Mutator::RNG was called a different number of time in parallel and "
           "sequential execution.";

    // Check archives
    ASSERT_EQ(plaParallel.getArchive().getNbRecordings(),
              plaSequential.getArchive().getNbRecordings())
        << "Archives have different sizes.";
    for (auto i = 0; i < plaParallel.getArchive().getNbRecordings(); i++) {
        ASSERT_EQ(plaParallel.getArchive().at(i).dataHash,
                  plaSequential.getArchive().at(i).dataHash)
            << "Archives have different content.";
        ASSERT_EQ(plaParallel.getArchive().at(i).result,
                  plaSequential.getArchive().at(i).result)
            << "Archives have different content.";
    }
}

TEST_F(ParallelLearningAgentTest, EvalAllRootsParallelValidationDeterminism)
{
    // Check that parallel execution leads to the exact same results as
    // sequential
    params.archiveSize = 50;
    params.archivingProbability = 0.1;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 10;

    Learn::LearningAgent la(le, set, params);
    la.init(0); // Reset centralized RNG to 0
    auto results = la.evaluateAllRoots(0, Learn::LearningMode::VALIDATION);
    auto nextInt = la.getRNG().getUnsignedInt64(0, UINT64_MAX);

    Learn::LearningParameters paramsSequential = params;
    paramsSequential.nbThreads = 1;
    Learn::ParallelLearningAgent plaSequential(le, set, paramsSequential);

    plaSequential.init(0); // Reset centralized RNG to 0
    auto resultsSequential =
        plaSequential.evaluateAllRoots(0, Learn::LearningMode::VALIDATION);
    auto nextIntSequential =
        plaSequential.getRNG().getUnsignedInt64(0, UINT64_MAX);

    Learn::LearningParameters paramsParallel = params;
    paramsParallel.nbThreads = 4;
    Learn::ParallelLearningAgent plaParallel(le, set, paramsParallel);

    plaParallel.init(0); // Reset centralized RNG to 0
    auto resultsParallel =
        plaParallel.evaluateAllRoots(0, Learn::LearningMode::VALIDATION);
    auto nextIntParallel = plaParallel.getRNG().getUnsignedInt64(0, UINT64_MAX);

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
    ASSERT_EQ(plaSequential.getArchive().getNbRecordings(), 0)
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
    ASSERT_EQ(plaParallel.getArchive().getNbRecordings(), 0)
        << "Archives should be empty in Validation mode.";
}

TEST_F(ParallelLearningAgentTest, TrainOnegenerationSequential)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 3;
    params.ratioDeletedRoots =
        0.95; // high number to force the apparition of root action.
    params.nbThreads = 1;

    Learn::ParallelLearningAgent pla(le, set, params);

    pla.init();
    // Do the populate call to keep know the number of initial vertex
    Archive a(0);
    Mutator::TPGMutator::populateTPG(*pla.getTPGGraph(), a, params.mutation,
                                     pla.getRNG(), le.getVectActions());
    size_t initialNbVertex = pla.getTPGGraph()->getNbVertices();
    // Seed selected so that an action becomes a root during next generation
    ASSERT_NO_THROW(pla.trainOneGeneration(4))
        << "Training for one generation failed.";
    // Check the number of vertex in the graph.
    // Must be initial number of vertex - number of root removed
    ASSERT_EQ(pla.getTPGGraph()->getNbVertices(),
              initialNbVertex -
                  floor(params.ratioDeletedRoots * params.mutation.tpg.nbRoots))
        << "Number of remaining is under the number of roots from the "
           "TPGGraph.";
    // Train a second generation, because most roots were removed, a root
    // actions have appeared and the training algorithm will attempt to remove
    // them.
    ASSERT_NO_THROW(pla.trainOneGeneration(0))
        << "Training for one generation failed.";
}

TEST_F(ParallelLearningAgentTest, TrainOneGenerationParallel)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 3;
    params.ratioDeletedRoots =
        0.95; // high number to force the apparition of root action.
    params.nbThreads = 4;

    Learn::ParallelLearningAgent pla(le, set, params);

    pla.init();
    // Do the populate call to keep know the number of initial vertex
    Archive a(0);
    Mutator::TPGMutator::populateTPG(*pla.getTPGGraph(), a, params.mutation,
                                     pla.getRNG(), le.getVectActions());
    size_t initialNbVertex = pla.getTPGGraph()->getNbVertices();
    // Seed selected so that an action becomes a root during next generation
    ASSERT_NO_THROW(pla.trainOneGeneration(4))
        << "Training for one generation failed.";
    // Check the number of vertex in the graph.
    // Must be initial number of vertex - number of root removed
    ASSERT_EQ(pla.getTPGGraph()->getNbVertices(),
              initialNbVertex -
                  floor(params.ratioDeletedRoots * params.mutation.tpg.nbRoots))
        << "Number of remaining is under the number of roots from the "
           "TPGGraph.";
}

TEST_F(ParallelLearningAgentTest, TrainSequential)
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

    Learn::ParallelLearningAgent pla(le, set, params);

    pla.init();
    bool alt = false;

    ASSERT_NO_THROW(pla.train(alt, true))
        << "Training a TPG for several generation should not fail.";
    alt = true;
    ASSERT_NO_THROW(pla.train(alt, true))
        << "Using the boolean reference to stop the training should not fail.";
}

TEST_F(ParallelLearningAgentTest, TrainParallel)
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

    Learn::ParallelLearningAgent pla(le, set, params);

    pla.init();
    bool alt = false;

    ASSERT_NO_THROW(pla.train(alt, true))
        << "Training a TPG for several generation should not fail.";
    alt = true;
    ASSERT_NO_THROW(pla.train(alt, true))
        << "Using the boolean reference to stop the training should not fail.";
}

TEST_F(ParallelLearningAgentTest, TrainParallelDeterminism)
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

    Learn::LearningAgent la(le, set, params);

    la.init();

    // Train for several generation
    bool alt = false;
    la.train(alt, false);

    params.nbThreads = 4;
    Learn::ParallelLearningAgent pla(le, set, params);

    pla.init();

    // Train for several generation
    pla.train(alt, false);

    // Check number of vertex in graphs
    // Non-zero to avoid false positive.
    // These checks guarantee determinism between sequential and parallel
    // version on a given platform. They do not guarantee portability between
    // compilers and OS
    ASSERT_GT(la.getTPGGraph()->getNbVertices(), 0)
        << "Number of vertex in the trained graph should not be 0.";
    ASSERT_EQ(la.getTPGGraph()->getNbVertices(),
              pla.getTPGGraph()->getNbVertices())
        << "LearningAgent and ParallelLearning agent result in different "
           "TPGGraphs.";
}

TEST_F(ParallelLearningAgentTest, KeepBestPolicy)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 1;
    params.ratioDeletedRoots = 0.2;
    params.nbGenerations = 5;
    params.maxNbEvaluationPerPolicy =
        params.nbIterationsPerPolicyEvaluation * 2;

    Learn::ParallelLearningAgent pla(le, set, params);
    pla.init();
    bool alt = false;
    pla.train(alt, true);

    ASSERT_NO_THROW(pla.keepBestPolicy())
        << "Keeping the best policy after training should not fail.";
    ASSERT_EQ(pla.getTPGGraph()->getNbRootVertices(), 1)
        << "A single root TPGVertex should remain in the TPGGraph when keeping "
           "the best policy only";
}
