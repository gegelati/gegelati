/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2023 - 2025)
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

#include "algorithm/tpg/tpgAlgorithm.h"

#include "evoGraph/instrumented/actionInstrumented.h"
#include "evoGraph/instrumented/edgeInstrumented.h"
#include "evoGraph/instrumented/factoryInstrumented.h"
#include "evoGraph/instrumented/teamInstrumented.h"
#include "evoGraph/instrumented/vertexInstrumented.h"
#include "evoGraph/policyStats.h"
#include "evoGraph/graph.h"

#include "instructions/addPrimitiveType.h"
#include "mutator/rng.h"
#include "mutator/tpgMutator.h"

#include "learn/fakeMultiContinuousLearningEnvironment.h"
#include "learn/learningAgent.h"
#include "learn/learningEnvironment.h"
#include "learn/learningParameters.h"
#include "learn/parallelLearningAgent.h"
#include "learn/stickGameWithOpponent.h"

#include "util/counterReset.h"

// Set all file in comment

class LearningAgentTest : public ::testing::Test
{
  protected:
    Instructions::Set set;
    StickGameWithOpponent le;
    FakeMultiContinuousLearningEnvironment cle;
    Learn::LearningParameters params;
    std::shared_ptr<Algorithm::TPGAlgorithm> tpg;

    virtual void SetUp()
    {

        CounterReset::counterReset();
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
        params.nbProgramConstant = 5;

        tpg = std::make_shared<Algorithm::TPGAlgorithm>(params, set);
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

    ASSERT_NO_THROW(la = new Learn::LearningAgent(le, tpg, set, params))
        << "Construction of the learningAgent failed.";

    ASSERT_NO_THROW(delete la) << "Destruction of the LearningAgent failed.";
}

TEST_F(LearningAgentTest, Init)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    Learn::LearningAgent la(le, tpg, set, params);

    ASSERT_NO_THROW(la.init())
        << "Initialization of the LearningAgent should not fail.";

        
    params.selection._selectionMode = "wrongSelector";
    tpg =  std::make_shared<Algorithm::TPGAlgorithm>(params, set);
    Learn::LearningAgent la2(le, tpg, set, params);

    ASSERT_THROW(la2.init(), std::runtime_error)
        << "Initialization of the learningAgent with wrong selector should fail.";
}

TEST_F(LearningAgentTest, addLogger)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    Learn::LearningAgent la(le, tpg, set, params);

    Log::LALogger* l = nullptr;
    ASSERT_NO_THROW(
        l = new Log::LABasicLogger(la, std::cout)) // Call addLogger.
        << "Adding a logger should not fail.";
    if (l != nullptr) {
        delete l;
    }
}

/*TEST_F(LearningAgentTest, IsRootEvalSkipped)
{
    params.maxNbEvaluationPerPolicy = 2;

    Learn::LearningAgent la(le, tpg, set, params);
    la.init();

    // Test a new root
    std::shared_ptr<Learn::EvaluationResult> result1;
    ASSERT_FALSE(la.isRootEvalSkipped(
        *la.getGraph()->getRootVertices().at(0), result1))
        << "Method should return false for a root that has never been "
           "evaluated before.";
    ASSERT_EQ(result1, nullptr) << "Method should return a nullptr for a root "
                                   "that has not been evaluated before.";

    // Add an EvaluationResult artificially
    result1 = std::make_shared<Learn::EvaluationResult>(
        std::make_shared<Selector::SelectionMetrics>(1.0), 1);
    la.getSelector()->updateEvaluationRecords(
        {{result1, la.getGraph()->getRootVertices().at(0)}});

    // Test the root again
    std::shared_ptr<Learn::EvaluationResult> result2;
    ASSERT_FALSE(la.isRootEvalSkipped(
        *la.getGraph()->getRootVertices().at(0), result2))
        << "Method should return false for a root that has been evaluated "
           "before.";
    ASSERT_EQ(result2, result1)
        << "Method should return a valid pointer for a root that has not been "
           "evaluated enough times before.";

    // Update the EvaluationResult artificially
    result2 = std::make_shared<Learn::EvaluationResult>(
        std::make_shared<Selector::SelectionMetrics>(1.0), 2);
    la.getSelector()->updateEvaluationRecords(
        {{result2, la.getGraph()->getRootVertices().at(0)}});

    // Test the root again.
    std::shared_ptr<Learn::EvaluationResult> result3;
    ASSERT_TRUE(la.isRootEvalSkipped(*la.getGraph()->getRootVertices().at(0),
                                     result3))
        << "Method should return true for a root that has been evaluated "
           "before more times than maxNbEvaluationPerPolicy.";
    ASSERT_EQ(result3, result2)
        << "Method should return a the EvaluationResult from the "
           "resultsPerRoot map when the number of evaluation exceeds "
           "maxNbEvaluationPerPolicy.";
}*/

/*TEST_F(LearningAgentTest, MakeJob)
{
    Learn::LearningAgent la(le, tpg, set, params);
    la.init();
    auto job = *la.makeJob(la.getAlgorithmAt(0)->getAgents().at(0),
                           Learn::LearningMode::TRAINING);
    ASSERT_NO_THROW(job.getArchiveSeed()) << "job should have an archive seed";
    ASSERT_NO_THROW(job.getIdx()) << "job should have an idx";
    ASSERT_EQ(la.getAlgorithmAt(0)->getAgents().at(0), job.getAgent())
        << "Encapsulate the root in a job shouldn't change it";

    Learn::LearningAgent la2(le, tpg, set, params);
    ASSERT_THROW(la2.makeJob(nullptr, Learn::LearningMode::TRAINING), std::runtime_error)
        << "Create a job when the learning agent is not initialized should throw an error";
}

TEST_F(LearningAgentTest, MakeJobs)
{
    Learn::LearningAgent la(le, tpg, set, params);
    la.init();
    auto jobs = la.makeJobs(Learn::LearningMode::TRAINING);
    ASSERT_EQ(la.getAlgorithmAt(0)->getAgents().size(), jobs.size())
        << "There should be as many jobs as roots";
    for (int i = 0; i < la.getGraph()->getNbRootVertices(); i++) {
        ASSERT_EQ(la.getAlgorithmAt(0)->getAgents().at(i),
                  (*jobs.front()).getAgent())
            << "Encapsulate the root in a job shouldn't change it";
        jobs.pop();
    }
}*/

TEST_F(LearningAgentTest, EvalAgent)
{
    params.archiveSize = 50;
    params.archivingProbability = 1.0;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 10;

    Learn::LearningAgent la(le, tpg, set, params);
    Archive a; // For testing purposes, notmally, the archive from the
               // LearningAgent is used.

    la.init();
    std::unique_ptr<Algorithm::ExecutionEngine> execEngine = tpg->getManager()->createExecutionEngine();

    std::shared_ptr<Learn::EvaluationResult> result;
    auto job = tpg->createJob(la.getAlgorithmAt(0)->getAgents().at(0),
                           Learn::LearningMode::TRAINING, la.getRNG());
    la.setCurrentAlgorithm(tpg);
    ASSERT_NO_THROW(
        result = la.evaluateJob(*execEngine, *job, 0, Learn::LearningMode::TRAINING, le))
        << "Evaluation from a root failed.";
    ASSERT_LE(result->getSelectionMetrics()->getScore(), 1.0)
        << "Average score should not exceed the score of a perfect player.";
}

TEST_F(LearningAgentTest, EvaluateOneRoot)
{
    params.archiveSize = 50;
    params.archivingProbability = 1.0;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 10;

    Learn::LearningAgent la(le, tpg, set, params);
    Archive a; // For testing purposes, normally, the archive from the
               // LearningAgent is used.

    la.init();

    la.setCurrentAlgorithm(tpg);
    std::shared_ptr<Learn::EvaluationResult> result;
    ASSERT_NO_THROW(
        result = la.evaluateOneAgent(0, Learn::LearningMode::TRAINING,
                                    la.getAlgorithmAt(0)->getAgents().at(0)))
        << "Evaluation from a root failed.";
    ASSERT_LE(result->getSelectionMetrics()->getScore(), 1.0)
        << "Average score should not exceed the score of a perfect player.";
}

TEST_F(LearningAgentTest, EvalAllRoots)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 10;

    tpg = std::make_shared<Algorithm::TPGAlgorithm>(params, set);
    Learn::LearningAgent la(le, tpg, set, params);

    la.init();
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  std::shared_ptr<const Algorithm::Agent>>
        result;
    ASSERT_NO_THROW(result =
                        la.evaluateAllAgents(0, Learn::LearningMode::TRAINING))
        << "Evaluation from a root failed.";
    ASSERT_EQ(result.size(), la.getGraph()->getNbRootVertices())
        << "Number of evaluated roots is under the number of roots from the "
           "Graph.";
}



TEST_F(LearningAgentTest, TrainOnegeneration)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 3;
    params.selection.truncation.ratioDeletedRoots =
        0.95; // high number to force the apparition of root action.

    // we will validate in order to cover validation log
    params.doValidation = true;

    Learn::LearningAgent la(le, tpg, set, params);

    la.init();

    // we add a logger to la to check it logs things
    std::ofstream o("tempFileForTest", std::ofstream::out);
    Log::LABasicLogger l(la, o);

    // Do the populate call to keep know the number of initial vertex
    Archive a(0);
    auto tpg = la.getAlgorithmAt(0);
    tpg->getMutator()->mutatePopulation(la.getGraph(), tpg->getManager(), tpg->getSelector(), params, la.getRNG());

    size_t initialNbVertex = la.getGraph()->getNbVertices();
    // Seed selected so that an action becomes a root during next generation
    ASSERT_NO_THROW(la.trainOneGeneration(4, false))
        << "Training for one generation failed.";
    // Check the number of vertex in the graph.
    // Must be initial number of vertex - number of root removed
    ASSERT_EQ(la.getGraph()->getNbVertices(),
              initialNbVertex -
                  floor(params.selection.truncation.ratioDeletedRoots *
                        params.mutation.tpg.nbRoots))
        << "Number of remaining is under the number of roots from the "
           "Graph.";

    // Check that bestRoot has been set
    ASSERT_NE(tpg->getSelector()->getBestAgent().first, nullptr);

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
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.nbGenerations = 3;

    Learn::LearningAgent la(le, tpg, set, params);

    la.init();
    bool alt = false;

    ASSERT_NO_THROW(la.train(alt, true))
        << "Training a TPG for several generation should not fail.";
    alt = true;
    ASSERT_NO_THROW(la.train(alt, true))
        << "Using the boolean reference to stop the training should not fail.";

    // For coverage
    params.doValidation = true;
    params.stepValidation = 2;
    Learn::LearningAgent la2(le, tpg, set, params);
    alt = false;
    la2.init();
    ASSERT_NO_THROW(la2.train(alt, true))
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
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.nbGenerations = 20;
    params.mutation.tpg.nbRoots = 30;
    // A root may be evaluated at most for 3 generations
    params.maxNbEvaluationPerPolicy =
        params.nbIterationsPerPolicyEvaluation * 3;
    params.nbThreads = 3;

    tpg = std::make_shared<Algorithm::TPGAlgorithm>(params, set);
    Learn::LearningAgent la(le, tpg, set, params);

    la.init();
    bool alt = false;
    la.train(alt, false);
    EvoGraph::Graph& tpg = *la.getGraph();

    // Useful when determinism is changed
    /*std::cout << tpg.getNbVertices() << " "
             <<tpg.getNbRootVertices()<<" "
             <<tpg.getEdges().size()<<" "
             <<EvoGraph::Vertex::getVertexIDCounter()<<" "
             <<EvoGraph::Edge::getEdgeIDCounter()<<" "
             <<Algorithm::Agent::getAgentIDCounter()<<" "

             <<la.getRNG().getUnsignedInt64(0, UINT64_MAX)<<std::endl;*/

    // It is quite unlikely that two different TPGs after 20 generations
    // end up with the same number of vertices, roots, edges and calls to
    // the RNG without being identical.
    ASSERT_EQ(tpg.getNbVertices(), 29)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(tpg.getNbRootVertices(), 24)
        << "Graph does not have the expected determinist characteristics.";
    ASSERT_EQ(tpg.getEdges().size(), 91)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(EvoGraph::Vertex::getVertexIDCounter(), 149)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(EvoGraph::Edge::getEdgeIDCounter(), 566)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(Algorithm::Agent::getAgentIDCounter(), 367)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(la.getRNG().getUnsignedInt64(0, UINT64_MAX), 513874846099432898U)
        << "Graph does not have the expected determinst characteristics.";
}

// Similar to previous test, but verifications of graphs properties are here to
// ensure the result of the training is identical on all OSes and Compilers.
TEST_F(LearningAgentTest, TrainLGPPortability)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 5;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.nbGenerations = 20;
    params.mutation.tpg.nbRoots = 30;
    // A root may be evaluated at most for 3 generations
    params.maxNbEvaluationPerPolicy =
        params.nbIterationsPerPolicyEvaluation * 3;
    params.nbThreads = 3;

    auto lgp = std::make_shared<Algorithm::LGPAlgorithm>(params, set);
    Learn::LearningAgent la(le, lgp, set, params);

    la.init();
    bool alt = false;
    la.train(alt, false);
    EvoGraph::Graph& tpg = *la.getGraph();

    // Useful when determinism is changed
    /*std::cout << tpg.getNbVertices() << " "
             <<tpg.getNbRootVertices()<<" "
             <<tpg.getEdges().size()<<" "
             <<EvoGraph::Vertex::getVertexIDCounter()<<" "
             <<EvoGraph::Edge::getEdgeIDCounter()<<" "
             <<Algorithm::Agent::getAgentIDCounter()<<" "

             <<la.getRNG().getUnsignedInt64(0, UINT64_MAX)<<std::endl;*/

    // It is quite unlikely that two different TPGs after 20 generations
    // end up with the same number of vertices, roots, edges and calls to
    // the RNG without being identical.
    ASSERT_EQ(tpg.getNbVertices(), 0)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(tpg.getNbRootVertices(), 0)
        << "Graph does not have the expected determinist characteristics.";
    ASSERT_EQ(tpg.getEdges().size(), 0)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(EvoGraph::Vertex::getVertexIDCounter(), 0)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(EvoGraph::Edge::getEdgeIDCounter(), 0)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(Algorithm::Agent::getAgentIDCounter(), 144)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(la.getRNG().getUnsignedInt64(0, UINT64_MAX), 12539348269636698672)
        << "Graph does not have the expected determinst characteristics.";
}


// Same as previous, but with a TPGInstrumentedFactory
TEST_F(LearningAgentTest, TrainInstrumented)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 5;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.nbGenerations = 20;
    params.mutation.tpg.nbRoots = 30;
    // A root may be evaluated at most for 3 generations
    params.maxNbEvaluationPerPolicy =
        params.nbIterationsPerPolicyEvaluation * 3;
    params.mutation.tpg.forceProgramBehaviorChangeOnMutation = true;
    params.nbThreads = 3;

    Learn::LearningAgent la(le, tpg, set, params, EvoGraph::TPGInstrumentedFactory());

    la.init();
    bool alt = false;
    la.train(alt, false);

    // It is quite unlikely that two different TPGs after 20 generations
    // end up with the same number of vertices, roots, edges and calls to
    // the RNG without being identical.
    EvoGraph::Graph& tpg = *la.getGraph();
    ASSERT_EQ(tpg.getNbVertices(), 27)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(tpg.getNbRootVertices(), 24)
        << "Graph does not have the expected determinist characteristics.";
    ASSERT_EQ(tpg.getEdges().size(), 86)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(EvoGraph::Vertex::getVertexIDCounter(), 147)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(EvoGraph::Edge::getEdgeIDCounter(), 596)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(Algorithm::Agent::getAgentIDCounter(), 346)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(la.getRNG().getUnsignedInt64(0, UINT64_MAX), 786532405746195131U)
        << "Graph does not have the expected determinst characteristics.";

    /*
    //To help to refind the values if the determinism is changed by an update
    size_t idx = 0;
    for (const auto& edge : tpg.getEdges()) {
        const EvoGraph::EdgeInstrumented* edgeInstrumented =
            dynamic_cast<const EvoGraph::EdgeInstrumented*>(edge.get());

        if (edgeInstrumented != nullptr) {
            std::cout << "Idx = " << idx++
                    << ", NbVisits = " << edgeInstrumented->getNbVisits()
                    << ", NbTraversal = " << edgeInstrumented->getNbTraversal()
                    << std::endl;
        }
    }
    for(auto vert: tpg.getVertices()){
        std::cout<<dynamic_cast<const
    EvoGraph::VertexInstrumentation*>(vert)->getNbVisits()<<std::endl;
    }*/

    // Check number of visits of a few edges & vertices
    auto edgesIterator = tpg.getEdges().begin();
    const auto* edge1 = edgesIterator->get();

    ASSERT_EQ(
        dynamic_cast<const EvoGraph::EdgeInstrumented*>(edge1)->getNbVisits(),
        63);
    ASSERT_EQ(
        dynamic_cast<const EvoGraph::EdgeInstrumented*>(edge1)->getNbTraversal(),
        0);

    std::advance(edgesIterator, 21);
    const auto* edge2 = edgesIterator->get();

    ASSERT_EQ(
        dynamic_cast<const EvoGraph::EdgeInstrumented*>(edge2)->getNbVisits(),
        87);
    ASSERT_EQ(
        dynamic_cast<const EvoGraph::EdgeInstrumented*>(edge2)->getNbTraversal(),
        4);

    auto& verticesIterator = tpg.getVertices();
    ASSERT_EQ(std::dynamic_pointer_cast<const EvoGraph::VertexInstrumentation>(
                  verticesIterator.at(0))
                  ->getNbVisits(),
              3327);

    ASSERT_EQ(std::dynamic_pointer_cast<const EvoGraph::VertexInstrumentation>(
                  verticesIterator.at(5))
                  ->getNbVisits(),
              63);
}

// Similar to previous test, but with continuous actions and no action programs
TEST_F(LearningAgentTest, TrainContinuousNoActionPrograms)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 5;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.nbGenerations = 20;
    params.mutation.tpg.nbRoots = 30;
    params.mutation.tpg.useActionProgram = false;
    // A root may be evaluated at most for 3 generations
    params.maxNbEvaluationPerPolicy =
        params.nbIterationsPerPolicyEvaluation * 3;
    params.mutation.tpg.forceProgramBehaviorChangeOnMutation = true;
    params.nbThreads = 1;
    tpg = std::make_shared<Algorithm::TPGAlgorithm>(params, set);

    Learn::LearningAgent la(cle, tpg, set, params);

    la.init();
    bool alt = false;
    la.train(alt, false);

    // It is quite unlikely that two different TPGs after 20 generations
    // end up with the same number of vertices, roots, edges and calls to
    // the RNG without being identical.
    EvoGraph::Graph& tpg = *la.getGraph();


    // Useful when determinism is changed
    /*std::cout << tpg.getNbVertices() << " "
             <<tpg.getNbRootVertices()<<" "
             <<tpg.getEdges().size()<<" "
             <<EvoGraph::Vertex::getVertexIDCounter()<<" "
             <<EvoGraph::Edge::getEdgeIDCounter()<<" "
             <<Algorithm::Agent::getAgentIDCounter()<<" "

             <<la.getRNG().getUnsignedInt64(0, UINT64_MAX)<<std::endl;*/

    ASSERT_EQ(tpg.getNbVertices(), 27)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(tpg.getNbRootVertices(), 24)
        << "Graph does not have the expected determinist characteristics.";
    ASSERT_EQ(tpg.getEdges().size(), 97)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(EvoGraph::Vertex::getVertexIDCounter(), 147)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(EvoGraph::Edge::getEdgeIDCounter(), 657)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(Algorithm::Agent::getAgentIDCounter(), 353)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(la.getRNG().getUnsignedInt64(0, UINT64_MAX), 7424346006285462471U)
        << "Graph does not have the expected determinst characteristics.";
}

// Similar to previous test, but with continuous actions and no action programs
TEST_F(LearningAgentTest, TrainContinuousWithSingleActionPrograms)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 5;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.nbGenerations = 20;
    params.mutation.tpg.nbRoots = 30;
    params.mutation.tpg.useActionProgram = true;
    // A root may be evaluated at most for 3 generations
    params.maxNbEvaluationPerPolicy =
        params.nbIterationsPerPolicyEvaluation * 3;
    params.mutation.tpg.forceProgramBehaviorChangeOnMutation = true;
    params.nbThreads = 3;

    Learn::LearningAgent la(cle, tpg, set, params);

    la.init();
    bool alt = false;
    la.train(alt, false);

    // It is quite unlikely that two different TPGs after 20 generations
    // end up with the same number of vertices, roots, edges and calls to
    // the RNG without being identical.
    EvoGraph::Graph& tpg = *la.getGraph();
    ASSERT_EQ(tpg.getNbVertices(), 67)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(tpg.getNbRootVertices(), 24)
        << "Graph does not have the expected determinist characteristics.";
    ASSERT_EQ(tpg.getEdges().size(), 143)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(EvoGraph::Vertex::getVertexIDCounter(), 278)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(EvoGraph::Edge::getEdgeIDCounter(), 817)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(Program::Program::getProgramIDCounter(), 505)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(la.getRNG().getUnsignedInt64(0, UINT64_MAX),
              13533704402962106053U)
        << "Graph does not have the expected determinst characteristics.";
}

// Similar to previous test, but with MATPG solution (no need for MAPLE because
// it is included in MATPG)
TEST_F(LearningAgentTest, TrainContinuousWithMATPG)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 5;
    params.nbGenerations = 20;
    params.mutation.tpg.nbRoots = 30;
    params.mutation.tpg.useActionProgram = true;
    params.mutation.tpg.useMultiActionProgram = true;
    params.mutation.tpg.teamAccessAllActions = false;
    params.mutation.tpg.ratioTeamsOverActions = 0.3333;
    params.selection._selectionMode = "truncation";
    params.selection.truncation.ratioDeletedRoots = 0.5;
    params.mutation.tpg.nbRoots = 100;
    params.nbThreads = 3;
    // A root may be evaluated at most for 3 generations
    params.maxNbEvaluationPerPolicy =
        params.nbIterationsPerPolicyEvaluation * 3;
    params.mutation.tpg.forceProgramBehaviorChangeOnMutation = true;

    Learn::LearningAgent la(cle, tpg, set, params);

    la.init();
    bool alt = false;
    la.train(alt, false);
    EvoGraph::Graph& tpg = *la.getGraph();

    // Useful when determinism is changed
    /*std::cout<<tpg.getNbVertices()<<" "
             <<tpg.getNbRootVertices()<<" "
             <<tpg.getEdges().size()<<" "
             <<EvoGraph::Vertex::getVertexIDCounter()<<" "
             <<EvoGraph::Edge::getEdgeIDCounter()<<" "
             <<Program::Program::getProgramIDCounter()<<" "
             <<la.getRNG().getUnsignedInt64(0, UINT64_MAX)<<std::endl*/
    ;

    // It is quite unlikely that two different TPGs after 20 generations
    // end up with the same number of vertices, roots, edges and calls to
    // the RNG without being identical.
    ASSERT_EQ(tpg.getNbVertices(), 81)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(tpg.getNbRootVertices(), 50)
        << "Graph does not have the expected determinist characteristics.";
    ASSERT_EQ(tpg.getEdges().size(), 236)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(EvoGraph::Vertex::getVertexIDCounter(), 1318)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(EvoGraph::Edge::getEdgeIDCounter(), 4641)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(Program::Program::getProgramIDCounter(), 2506)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(la.getRNG().getUnsignedInt64(0, UINT64_MAX), 1975743868576999154U)
        << "Graph does not have the expected determinst characteristics.";
}

// Similar to previous test, but with MATPG solution and tournament (no need for
// MAPLE because it is included in MATPG)
TEST_F(LearningAgentTest, TrainContinuousWithMATPGTournament)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 5;
    params.nbGenerations = 20;
    params.mutation.tpg.nbRoots = 30;
    params.mutation.tpg.useActionProgram = true;
    params.mutation.tpg.useMultiActionProgram = true;
    params.mutation.tpg.teamAccessAllActions = false;
    params.mutation.tpg.ratioTeamsOverActions = 0.3333;
    params.selection._selectionMode = "tournament";
    params.selection.tournament.sizeTournament = 3;
    params.selection.tournament.ratioSavedRoots = 0.2;
    params.mutation.tpg.nbRoots = 100;
    params.nbThreads = 3;
    // A root may be evaluated at most for 3 generations
    params.maxNbEvaluationPerPolicy =
        params.nbIterationsPerPolicyEvaluation * 3;
    params.mutation.tpg.forceProgramBehaviorChangeOnMutation = true;

    Learn::LearningAgent la(cle, tpg, set, params);

    la.init();
    bool alt = false;
    la.train(alt, false);
    EvoGraph::Graph& graph = *la.getGraph();

    // Useful when determinism is changed
    /*std::cout<<tpg.getNbVertices()<<" "
             <<tpg.getNbRootVertices()<<" "
             <<tpg.getEdges().size()<<" "
             <<EvoGraph::Vertex::getVertexIDCounter()<<" "
             <<EvoGraph::Edge::getEdgeIDCounter()<<" "
             <<Program::Program::getProgramIDCounter()<<" "
             <<la.getRNG().getUnsignedInt64(0, UINT64_MAX)<<std::endl*/
    ;

    // It is quite unlikely that two different TPGs after 20 generations
    // end up with the same number of vertices, roots, edges and calls to
    // the RNG without being identical.
    ASSERT_EQ(graph.getNbVertices(), 88)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(graph.getNbRootVertices(), 48)
        << "Graph does not have the expected determinist characteristics.";
    ASSERT_EQ(graph.getEdges().size(), 249)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(EvoGraph::Vertex::getVertexIDCounter(), 2000)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(EvoGraph::Edge::getEdgeIDCounter(), 7293)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(Program::Program::getProgramIDCounter(), 3911)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(la.getRNG().getUnsignedInt64(0, UINT64_MAX), 3136819910087175608U)
        << "Graph does not have the expected determinst characteristics.";

    params.selection.tournament.areElitesReproductible = true;
    Learn::LearningAgent la2(cle, tpg, set, params);

    la2.init();
    la2.train(alt, false);
    EvoGraph::Graph& graph2 = *la2.getGraph();

    // It is quite unlikely that two different TPGs after 20 generations
    // end up with the same number of vertices, roots, edges and calls to
    // the RNG without being identical.
    ASSERT_EQ(graph2.getNbVertices(), 78)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(graph2.getNbRootVertices(), 46)
        << "Graph does not have the expected determinist characteristics.";
    ASSERT_EQ(graph2.getEdges().size(), 239)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(EvoGraph::Vertex::getVertexIDCounter(), 4011)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(EvoGraph::Edge::getEdgeIDCounter(), 14700)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(Program::Program::getProgramIDCounter(), 7769)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(la2.getRNG().getUnsignedInt64(0, UINT64_MAX),
              14919253376982807560U)
        << "Graph does not have the expected determinst characteristics.";
}

TEST_F(LearningAgentTest, TrainContinuousWithCrossingMAPLE)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 5;
    params.nbGenerations = 20;
    params.mutation.tpg.nbRoots = 30;
    params.mutation.tpg.useActionProgram = true;
    params.mutation.tpg.useMultiActionProgram = true;
    params.mutation.tpg.teamAccessAllActions = false;
    params.mutation.tpg.ratioTeamsOverActions = 0.0;
    params.mutation.tpg.pActionEdgeAddition = 0.7;
    params.mutation.tpg.pActionEdgeDeletion = 0.7;
    params.mutation.tpg.pCrossAgents = 0.7;
    params.mutation.tpg.pCrossPrograms = 0.3;
    params.selection._selectionMode = "truncation";
    params.selection.truncation.ratioDeletedRoots = 0.5;
    params.mutation.tpg.nbRoots = 100;
    params.nbThreads = 3;
    // A root may be evaluated at most for 3 generations
    params.maxNbEvaluationPerPolicy =
        params.nbIterationsPerPolicyEvaluation * 3;
    params.mutation.tpg.forceProgramBehaviorChangeOnMutation = true;

    Learn::LearningAgent la(cle, tpg, set, params);

    la.init();
    bool alt = false;
    la.train(alt, false);
    EvoGraph::Graph& graph = *la.getGraph();

    // Useful when determinism is changed
    /*std::cout<<tpg.getNbVertices()<<" "
             <<tpg.getNbRootVertices()<<" "
             <<tpg.getEdges().size()<<" "
             <<EvoGraph::Vertex::getVertexIDCounter()<<" "
             <<EvoGraph::Edge::getEdgeIDCounter()<<" "
             <<Program::Program::getProgramIDCounter()<<" "
             <<la.getRNG().getUnsignedInt64(0, UINT64_MAX)<<std::endl*/
    ;

    // It is quite unlikely that two different TPGs after 20 generations
    // end up with the same number of vertices, roots, edges and calls to
    // the RNG without being identical.
    ASSERT_EQ(graph.getNbVertices(), 50)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(graph.getNbRootVertices(), 50)
        << "Graph does not have the expected determinist characteristics.";
    ASSERT_EQ(graph.getEdges().size(), 150)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(EvoGraph::Vertex::getVertexIDCounter(), 1057)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(EvoGraph::Edge::getEdgeIDCounter(), 5280)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(Program::Program::getProgramIDCounter(), 2546)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(la.getRNG().getUnsignedInt64(0, UINT64_MAX), 8433195571775175959U)
        << "Graph does not have the expected determinst characteristics.";
}
/*
TEST_F(LearningAgentTest, GraphCleanProgramIntrons)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 1;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.nbGenerations = 5;

    Learn::LearningAgent la(le, tpg, set, params);
    la.init();
    bool alt = false;
    la.train(alt, false);


    EvoGraph::Graph& graph = *la.getGraph();
    
    la.getAlgorithmAt(0)->getSelector()->keepBestPolicy(la.getGraph());

    // Get policy stats
    EvoGraph::PolicyStats psOrigin;
    psOrigin.setEnvironment(graph.getEnvironment());
    psOrigin.analyzePolicy(graph.getRootVertices().at(0));

    // Check the presence of introns
    ASSERT_GT(std::accumulate(psOrigin.nbIntronPerProgram.begin(),
                              psOrigin.nbIntronPerProgram.end(), size_t(0)),
              0)
        << "Graph has no introns in its programs after training.";

    // Record the behavior of the TPG with introns
    le.reset();
    std::unique_ptr<Algorithm::ExecutionEngine> execEngine = tpg->createExecutionEngine();

    std::vector<const EvoGraph::Vertex*> pathOrigin =
        tee.executeFromRoot(*(graph.getRootVertices().at(0))).first;

    // Clear introns
    la.getAlgorithmAt(0)->clearUnusedAgentParts();

    // Get new policy stats
    EvoGraph::PolicyStats psNoIntrons;
    psNoIntrons.setEnvironment(graph.getEnvironment());
    psNoIntrons.analyzePolicy(graph.getRootVertices().at(0));

    // Compare
    ASSERT_EQ(std::accumulate(psNoIntrons.nbIntronPerProgram.begin(),
                              psNoIntrons.nbIntronPerProgram.end(), size_t(0)),
              0)
        << "Graph still contains introns after they were cleared.";

    // Check that the behavior is identical (empirically, not really foolproof)
    std::vector<const EvoGraph::Vertex*> pathNoIntrons =
        tee.executeFromRoot(*(graph.getRootVertices().at(0))).first;

    ASSERT_EQ(pathOrigin.size(), pathNoIntrons.size())
        << "Path length in TPG before and after inton removal is not "
           "identical.";
    for (auto idx = 0; idx < pathOrigin.size(); idx++) {
        ASSERT_EQ(pathOrigin.at(idx), pathNoIntrons.at(idx))
            << "Path element in Graph changed when removing introns.";
    }
}
    */

TEST_F(LearningAgentTest, TrainOnegenerationContinuousNoActionProg)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 3;
    params.selection.truncation.ratioDeletedRoots =
        0.5; // high number to force the apparition of root action.
    params.nbThreads = 1;
    Learn::LearningAgent la(cle, tpg, set, params);

    la.init();
    // Do the populate call to keep know the number of initial vertex
    Archive a(0);

    tpg->getMutator()->mutatePopulation(la.getGraph(), tpg->getManager(), tpg->getSelector(), params, la.getRNG());
    size_t initialNbVertex = la.getGraph()->getNbVertices();

    // Seed selected so that an action becomes a root during next generation
    ASSERT_NO_THROW(la.trainOneGeneration(4))
        << "Training for one generation failed.";
    // Check the number of vertex in the graph.
    // Must be lower or equal to initial number of vertex - number of root
    // removed (since some actions vertex are removed too)
    ASSERT_LE(la.getGraph()->getNbVertices(),
              initialNbVertex -
                  floor(params.selection.truncation.ratioDeletedRoots *
                        params.mutation.tpg.nbRoots))
        << "Number of remaining is under the number of roots from the "
           "Graph.";
    // Train a second generation, because most roots were removed, a root
    // actions have appeared and the training algorithm will attempt to remove
    // them.
    ASSERT_NO_THROW(la.trainOneGeneration(0))
        << "Training for one generation failed.";
}

TEST_F(ParallelLearningAgentTest, Constructor)
{
    Learn::ParallelLearningAgent* pla;

    ASSERT_NO_THROW(pla = new Learn::ParallelLearningAgent(le, tpg, set, params))
        << "Construction of the learningAgent failed.";

    ASSERT_NO_THROW(delete pla) << "Destruction of the LearningAgent failed.";
}

TEST_F(ParallelLearningAgentTest, Init)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    Learn::ParallelLearningAgent pla(le, tpg, set, params);

    ASSERT_NO_THROW(pla.init())
        << "Initialization of the LearningAgent should not fail.";

        
    params.selection._selectionMode = "wrongSelector";
    tpg =  std::make_shared<Algorithm::TPGAlgorithm>(params, set);
    Learn::ParallelLearningAgent pla2(le, tpg, set, params);

    ASSERT_THROW(pla2.init(), std::runtime_error)
        << "Initialization of the learningAgent with wrong selector should fail.";
}


TEST_F(ParallelLearningAgentTest, EvalRootSequential)
{
    params.archiveSize = 50;
    params.archivingProbability = 1.0;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 10;
    params.nbThreads = 1;

    Learn::ParallelLearningAgent pla(le, tpg, set, params);
    Archive a; // For testing purposes, notmally, the archive from the
               // LearningAgent is used.

    pla.init();
    std::unique_ptr<Algorithm::ExecutionEngine> execEngine = tpg->getManager()->createExecutionEngine();

    std::shared_ptr<Learn::EvaluationResult> result;
    auto job = tpg->createJob(pla.getAlgorithmAt(0)->getAgents().at(0),
                           Learn::LearningMode::TRAINING, pla.getRNG());
    pla.setCurrentAlgorithm(tpg);
    ASSERT_NO_THROW(
        result = pla.evaluateJob(*execEngine, *job, 0, Learn::LearningMode::TRAINING, le))
        << "Evaluation from a root failed.";
    ASSERT_LE(result->getSelectionMetrics()->getScore(), 1.0)
        << "Average score should not exceed the score of a perfect player.";
}

TEST_F(ParallelLearningAgentTest, EvalAllRootsSequential)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 10;
    params.nbThreads = 1;

    Learn::ParallelLearningAgent pla(le, tpg, set, params);

    pla.init();
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  std::shared_ptr<const Algorithm::Agent>>
        result;
    ASSERT_NO_THROW(result =
                        pla.evaluateAllAgents(0, Learn::LearningMode::TRAINING))
        << "Evaluation from a root failed.";
    ASSERT_EQ(result.size(), pla.getGraph()->getNbRootVertices())
        << "Number of evaluated roots is under the number of roots from the "
           "Graph.";
}

TEST_F(ParallelLearningAgentTest, EvalAllRootsParallel)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 10;
    params.nbThreads = 4;

    tpg = std::make_shared<Algorithm::TPGAlgorithm>(params, set);
    Learn::ParallelLearningAgent pla(le, tpg, set, params);

    pla.init();
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  std::shared_ptr<const Algorithm::Agent>>
        result;
    ASSERT_NO_THROW(result =
                        pla.evaluateAllAgents(0, Learn::LearningMode::TRAINING))
        << "Evaluation from a root failed.";
    ASSERT_EQ(result.size(), pla.getGraph()->getNbRootVertices())
        << "Number of evaluated roots is under the number of roots from the "
           "Graph.";
}

TEST_F(ParallelLearningAgentTest, EvalAllRootsParallelTrainingDeterminism)
{
    // Check that parallel execution leads to the exact same results as
    // sequential
    params.archiveSize = 50;
    params.archivingProbability = 0.1;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 10;


    auto tpgLa = std::make_shared<Algorithm::TPGAlgorithm>(params, set);
    Learn::LearningAgent la(le, tpgLa, set, params);
    la.init(0); // Reset RNG to 0
    auto results = la.evaluateAllAgents(0, Learn::LearningMode::TRAINING);
    auto nextInt = la.getRNG().getUnsignedInt64(0, UINT64_MAX);

    Learn::LearningParameters paramsSequential = params;
    paramsSequential.nbThreads = 1;
    auto tpgSequential = std::make_shared<Algorithm::TPGAlgorithm>(params, set);
    Learn::ParallelLearningAgent plaSequential(le, tpgSequential, set, paramsSequential);

    plaSequential.init(0); // Reset centralized RNG to 0
    auto resultsSequential =
        plaSequential.evaluateAllAgents(0, Learn::LearningMode::TRAINING);
    auto nextIntSequential =
        plaSequential.getRNG().getUnsignedInt64(0, UINT64_MAX);

    Learn::LearningParameters paramsParallel = params;
    paramsParallel.nbThreads = 4;
    auto tpgParallel = std::make_shared<Algorithm::TPGAlgorithm>(params, set);
    Learn::ParallelLearningAgent plaParallel(le, tpgParallel, set, paramsParallel);

    plaParallel.init(0); // Reset centralized RNG to 0
    auto resultsParallel =
        plaParallel.evaluateAllAgents(0, Learn::LearningMode::TRAINING);
    auto nextIntParallel = plaParallel.getRNG().getUnsignedInt64(0, UINT64_MAX);

    // Check equality between LearningAgent and ParallelLearningAgent
    ASSERT_EQ(results.size(), resultsSequential.size())
        << "Result maps have a different size.";
    auto iter = results.begin();
    auto iterSequential = resultsSequential.begin();
    while (iter != results.end()) {
        ASSERT_EQ(iter->first->getSelectionMetrics()->getScore(),
                  iterSequential->first->getSelectionMetrics()->getScore())
            << "Average score between sequential and parallel executions are "
               "differents.";
        iter++;
        iterSequential++;
    }

    // Check determinism of bestAgent score
    ASSERT_EQ(tpgLa->getSelector()->getBestAgent().first,
              tpgParallel->getSelector()->getBestAgent().first);

    // Check determinism of the number of RNG calls.
    ASSERT_EQ(nextInt, nextIntSequential)
        << "RNG::RNG was called a different number of time in parallel and "
           "sequential execution.";

    // Check archives
    ASSERT_GT(tpgLa->getArchive()->getNbRecordings(), 0)
        << "For the archive determinism tests to be meaningful, Archive should "
           "not be empty.";
    ASSERT_EQ(tpgLa->getArchive()->getNbRecordings(),
              tpgSequential->getArchive()->getNbRecordings())
        << "Archives have different sizes.";
    for (auto i = 0; i < tpgLa->getArchive()->getNbRecordings(); i++) {
        ASSERT_EQ(tpgLa->getArchive()->at(i).dataHash,
                  tpgSequential->getArchive()->at(i).dataHash)
            << "Archives have different content.";
        ASSERT_EQ(tpgLa->getArchive()->at(i).result,
                  tpgSequential->getArchive()->at(i).result)
            << "Archives have different content.";
    }

    // Check equality between ParallelLearningAgent in parallel and sequential
    // mode
    ASSERT_EQ(resultsParallel.size(), resultsParallel.size())
        << "Result maps have a different size.";
    iterSequential = resultsSequential.begin();
    auto iterParallel = resultsParallel.begin();
    while (iterSequential != resultsSequential.end()) {
        ASSERT_EQ(iterSequential->first->getSelectionMetrics()->getScore(),
                  iterParallel->first->getSelectionMetrics()->getScore())
            << "Average score between sequential and parallel executions are "
               "differents.";
        iterSequential++;
        iterParallel++;
    }

    // Check determinism of bestAgent score
    ASSERT_EQ(tpgSequential->getSelector()->getBestAgent().first,
              tpgParallel->getSelector()->getBestAgent().first);

    // Check determinism of the number of RNG calls.
    ASSERT_EQ(nextIntSequential, nextIntParallel)
        << "RNG::RNG was called a different number of time in parallel and "
           "sequential execution.";

    // Check archives
    ASSERT_EQ(tpgParallel->getArchive()->getNbRecordings(),
              tpgSequential->getArchive()->getNbRecordings())
        << "Archives have different sizes.";
    for (auto i = 0; i < tpgParallel->getArchive()->getNbRecordings(); i++) {
        ASSERT_EQ(tpgParallel->getArchive()->at(i).dataHash,
                  tpgSequential->getArchive()->at(i).dataHash)
            << "Archives have different content.";
        ASSERT_EQ(tpgParallel->getArchive()->at(i).result,
                  tpgSequential->getArchive()->at(i).result)
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

    auto tpgLa = std::make_shared<Algorithm::TPGAlgorithm>(params, set);
    Learn::LearningAgent la(le, tpgLa, set, params);
    la.init(0); // Reset centralized RNG to 0
    auto results = la.evaluateAllAgents(0, Learn::LearningMode::VALIDATION);
    auto nextInt = la.getRNG().getUnsignedInt64(0, UINT64_MAX);

    Learn::LearningParameters paramsSequential = params;
    paramsSequential.nbThreads = 1;
    
    auto tpgSequential = std::make_shared<Algorithm::TPGAlgorithm>(params, set);
    Learn::ParallelLearningAgent plaSequential(le, tpgSequential, set, paramsSequential);

    plaSequential.init(0); // Reset centralized RNG to 0
    auto resultsSequential =
        plaSequential.evaluateAllAgents(0, Learn::LearningMode::VALIDATION);
    auto nextIntSequential =
        plaSequential.getRNG().getUnsignedInt64(0, UINT64_MAX);

    Learn::LearningParameters paramsParallel = params;
    paramsParallel.nbThreads = 4;
    auto tpgParallel = std::make_shared<Algorithm::TPGAlgorithm>(params, set);
    Learn::ParallelLearningAgent plaParallel(le, tpgParallel, set, paramsParallel);

    plaParallel.init(0); // Reset centralized RNG to 0
    auto resultsParallel =
        plaParallel.evaluateAllAgents(0, Learn::LearningMode::VALIDATION);
    auto nextIntParallel = plaParallel.getRNG().getUnsignedInt64(0, UINT64_MAX);

    // Check equality between LearningAgent and ParallelLearningAgent
    ASSERT_EQ(results.size(), resultsSequential.size())
        << "Result maps have a different size.";
    auto iter = results.begin();
    auto iterSequential = resultsSequential.begin();
    while (iter != results.end()) {
        ASSERT_EQ(iter->first->getSelectionMetrics()->getScore(),
                  iterSequential->first->getSelectionMetrics()->getScore())
            << "Average score between sequential and parallel executions are "
               "differents.";
        iter++;
        iterSequential++;
    }

    // Check determinism of the number of RNG calls.
    ASSERT_EQ(nextInt, nextIntSequential)
        << "RNG::RNG was called a different number of time in parallel and "
           "sequential execution.";

    // Check archives
    ASSERT_EQ(tpgLa->getArchive()->getNbRecordings(), 0)
        << "Archives should be empty in Validation mode.";
    ASSERT_EQ(tpgSequential->getArchive()->getNbRecordings(), 0)
        << "Archives should be empty in Validation mode.";

    // Check equality between ParallelLearningAgent in parallel and sequential
    // mode
    ASSERT_EQ(resultsParallel.size(), resultsParallel.size())
        << "Result maps have a different size.";
    iterSequential = resultsSequential.begin();
    auto iterParallel = resultsParallel.begin();
    while (iterSequential != resultsSequential.end()) {
        ASSERT_EQ(iterSequential->first->getSelectionMetrics()->getScore(),
                  iterParallel->first->getSelectionMetrics()->getScore())
            << "Average score between sequential and parallel executions are "
               "differents.";
        iterSequential++;
        iterParallel++;
    }

    // Check determinism of the number of RNG calls.
    ASSERT_EQ(nextIntSequential, nextIntParallel)
        << "RNG::RNG was called a different number of time in parallel and "
           "sequential execution.";

    // Check archives
    ASSERT_EQ(tpgParallel->getArchive()->getNbRecordings(), 0)
        << "Archives should be empty in Validation mode.";
}

TEST_F(ParallelLearningAgentTest, TrainOnegenerationSequential)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 3;
    params.selection.truncation.ratioDeletedRoots =
        0.85; // high number to force the apparition of root action.
    params.nbThreads = 1;

    Learn::ParallelLearningAgent pla(le, tpg, set, params);

    pla.init();
    // Do the populate call to keep know the number of initial vertex
    Archive a(0);
    auto tpg = pla.getAlgorithmAt(0);
    tpg->getMutator()->mutatePopulation(pla.getGraph(), tpg->getManager(), tpg->getSelector(), params, pla.getRNG());
    size_t initialNbVertex = pla.getGraph()->getNbVertices();
    // Seed selected so that an action becomes a root during next generation
    ASSERT_NO_THROW(pla.trainOneGeneration(4, false))
        << "Training for one generation failed.";
    // Check the number of vertex in the graph.
    // Must be initial number of vertex - number of root removed
    ASSERT_EQ(pla.getGraph()->getNbVertices(),
              initialNbVertex -
                  floor(params.selection.truncation.ratioDeletedRoots *
                        params.mutation.tpg.nbRoots))
        << "Number of remaining is under the number of roots from the "
           "Graph.";

    // Check that bestRoot has been set
    ASSERT_NE(tpg->getSelector()->getBestAgent().first, nullptr);
}

TEST_F(ParallelLearningAgentTest, TrainOneGenerationParallel)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 3;
    params.selection.truncation.ratioDeletedRoots =
        0.85; // high number to force the apparition of root action.
    params.nbThreads = 4;

    tpg = std::make_shared<Algorithm::TPGAlgorithm>(params, set);
    Learn::ParallelLearningAgent pla(le, tpg, set, params);

    pla.init();
    // Do the populate call to keep know the number of initial vertex
    tpg->getMutator()->mutatePopulation(pla.getGraph(), tpg->getManager(), tpg->getSelector(), params, pla.getRNG());
    
    size_t initialNbVertex = pla.getGraph()->getNbVertices();
    // Seed selected so that an action becomes a root during next generation
    ASSERT_NO_THROW(pla.trainOneGeneration(4, false))
        << "Training for one generation failed.";
    // Check the number of vertex in the graph.
    // Must be initial number of vertex - number of root removed
    ASSERT_EQ(pla.getGraph()->getNbVertices(),
              initialNbVertex -
                  floor(params.selection.truncation.ratioDeletedRoots *
                        params.mutation.tpg.nbRoots))
        << "Number of remaining is under the number of roots from the "
           "Graph.";
}

TEST_F(ParallelLearningAgentTest, TrainSequential)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 5;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.nbGenerations = 3;
    params.maxNbEvaluationPerPolicy =
        params.nbIterationsPerPolicyEvaluation * 2;
    params.nbThreads = 1;

    tpg = std::make_shared<Algorithm::TPGAlgorithm>(params, set);
    Learn::ParallelLearningAgent pla(le, tpg, set, params);

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
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.nbGenerations = 3;
    params.maxNbEvaluationPerPolicy =
        params.nbIterationsPerPolicyEvaluation * 2;
    params.nbThreads = std::thread::hardware_concurrency();

    Learn::ParallelLearningAgent pla(le, tpg, set, params);

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
    params.selection.truncation.ratioDeletedRoots = 0.2;
    // Set a large number of generations and roots
    // so that the chances of something going wrong is higher.
    params.nbGenerations = 20;
    params.mutation.tpg.nbRoots = 30;
    params.maxNbEvaluationPerPolicy =
        params.nbIterationsPerPolicyEvaluation * 5;

    Learn::LearningAgent la(le, tpg, set, params);

    la.init();

    // Train for several generation
    bool alt = false;
    la.train(alt, false);

    params.nbThreads = 4;
    Learn::ParallelLearningAgent pla(le, tpg, set, params);

    pla.init();

    // Train for several generation
    pla.train(alt, false);

    // Check number of vertex in graphs
    // Non-zero to avoid false positive.
    // These checks guarantee determinism between sequential and parallel
    // version on a given platform. They do not guarantee portability between
    // compilers and OS
    ASSERT_GT(la.getGraph()->getNbVertices(), 0)
        << "Number of vertex in the trained graph should not be 0.";
    ASSERT_EQ(la.getGraph()->getNbVertices(),
              pla.getGraph()->getNbVertices())
        << "LearningAgent and ParallelLearning agent result in different "
           "Graphs.";
}

TEST_F(ParallelLearningAgentTest, TrainPortability)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 5;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.nbGenerations = 20;
    params.mutation.tpg.nbRoots = 30;
    // A root may be evaluated at most for 3 generations
    params.maxNbEvaluationPerPolicy =
        params.nbIterationsPerPolicyEvaluation * 3;
    params.nbThreads = 3;

    tpg = std::make_shared<Algorithm::TPGAlgorithm>(params, set);
    Learn::ParallelLearningAgent la(le, tpg, set, params);

    la.init();
    bool alt = false;
    la.train(alt, false);
    EvoGraph::Graph& tpg = *la.getGraph();

    // Useful when determinism is changed
    /* std::cout << tpg.getNbVertices() << " "
             <<tpg.getNbRootVertices()<<" "
             <<tpg.getEdges().size()<<" "
             <<EvoGraph::Vertex::getVertexIDCounter()<<" "
             <<EvoGraph::Edge::getEdgeIDCounter()<<" "
             <<Algorithm::Agent::getAgentIDCounter()<<" "

             <<la.getRNG().getUnsignedInt64(0, UINT64_MAX)<<std::endl;*/

    // It is quite unlikely that two different TPGs after 20 generations
    // end up with the same number of vertices, roots, edges and calls to
    // the RNG without being identical.
    ASSERT_EQ(tpg.getNbVertices(), 29)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(tpg.getNbRootVertices(), 24)
        << "Graph does not have the expected determinist characteristics.";
    ASSERT_EQ(tpg.getEdges().size(), 91)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(EvoGraph::Vertex::getVertexIDCounter(), 149)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(EvoGraph::Edge::getEdgeIDCounter(), 566)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(Algorithm::Agent::getAgentIDCounter(), 367)
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(la.getRNG().getUnsignedInt64(0, UINT64_MAX), 513874846099432898U)
        << "Graph does not have the expected determinst characteristics.";
}

/*
TEST_F(ParallelLearningAgentTest, KeepBestPolicy)
{
    params.archiveSize = 50;
    params.archivingProbability = 0.5;
    params.maxNbActionsPerEval = 11;
    params.nbIterationsPerPolicyEvaluation = 1;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.nbGenerations = 5;
    params.maxNbEvaluationPerPolicy =
        params.nbIterationsPerPolicyEvaluation * 2;

    Learn::ParallelLearningAgent pla(le, tpg, set, params);
    pla.init();
    bool alt = false;
    pla.train(alt, true);

    ASSERT_NO_THROW(pla.getSelector()->keepBestPolicy())
        << "Keeping the best policy after training should not fail.";
    ASSERT_EQ(pla.getGraph()->getNbRootVertices(), 1)
        << "A single root Vertex should remain in the Graph when keeping "
           "the best policy only";
}

TEST_F(LearningAgentTest, EvaluateJobWithUtility)
{
    // Fake env to use utilisty
    class UtilityEnv : public StickGameWithOpponent
    {
      public:
        bool isUsingUtility() const override
        {
            return true;
        }
        double getUtility() const override
        {
            return 42.0;
        }
    } utilityEnv;

    params.nbIterationsPerPolicyEvaluation = 2;
    Learn::LearningAgent la(utilityEnv, set, params);
    la.init();

    Archive a;
    EvoGraph::OldExecutionEngine tee(la.getGraph()->getEnvironment(), &a);
    auto job = *la.makeJob(la.getGraph()->getRootVertices().at(0),
                           Learn::LearningMode::TRAINING);

    // Check that the job can be evaluated without throwing an exception
    ASSERT_NO_THROW({
        auto res = la.evaluateJob(tee, job, 0, Learn::LearningMode::TRAINING,
                                  utilityEnv);
        ASSERT_GE(res->getSelectionMetrics()->getUtility(), 0.0);
    });
}

TEST_F(LearningAgentTest, EvaluateOneRootThrowsIfNotInGraph)
{
    Learn::LearningAgent la(le, tpg, set, params);
    la.init();
    EvoGraph::Team fakeTeam;
    ASSERT_THROW(
        la.evaluateOneRoot(0, Learn::LearningMode::TRAINING, &fakeTeam),
        std::runtime_error);
}
*/