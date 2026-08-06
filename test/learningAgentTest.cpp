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

#include "representation/tpg/tpgRepresentation.h"
#include "representation/lgp/lgpRepresentation.h"
#include "representation/cgp/cgpRepresentation.h"
#include "representation/tgp/tgpRepresentation.h"
#include "representation/maple/mapleRepresentation.h"
#include "representation/atpg/atpgRepresentation.h"

#include "evoGraph/instrumented/actionInstrumented.h"
#include "evoGraph/instrumented/edgeInstrumented.h"
#include "evoGraph/instrumented/factoryInstrumented.h"
#include "evoGraph/instrumented/teamInstrumented.h"
#include "evoGraph/instrumented/vertexInstrumented.h"
#include "evoGraph/graph.h"

#include "instructions/addPrimitiveType.h"
#include "instructions/lambdaInstruction.h"
#include "mutator/rng.h"

#include "learn/fakeMultiContinuousLearningEnvironment.h"
#include "learn/learningAgent.h"
#include "learn/learningEnvironment.h"
#include "parameters.h"
#include "learn/parallelLearningAgent.h"
#include "learn/stickGameWithOpponent.h"

#include "selector/mapElites/mapElitesDefaultDescriptors.h"

#include "util/counterReset.h"

// Set all file in comment

class LearningAgentTest : public ::testing::Test
{
  protected:
    Instructions::Set set;
    StickGameWithOpponent le;
    FakeMultiContinuousLearningEnvironment cle;
    Parameters params;
    Representation::LGPRepresentation* lgp;
    Representation::TPGRepresentation* tpg;
    Selector::Selector* selector;

    virtual void SetUp()
    {
        CounterReset::counterReset();
        set.add(*(new Instructions::AddPrimitiveType<int>()));
        set.add(*(new Instructions::AddPrimitiveType<double>()));

        // Proba as in Kelly's paper
        params.representation.tpg.maxInitOutgoingEdges = 3;
        params.representation.lgp.maxProgramSize = 96;
        params.representation.nbIndividuals = 15;
        params.representation.tpg.pEdgeDeletion = 0.7;
        params.representation.tpg.pEdgeAddition = 0.7;
        params.representation.tpg.pProgramMutation = 0.2;
        params.representation.tpg.pEdgeDestinationChange = 0.1;
        params.representation.tpg.pEdgeDestinationIsAction = 0.5;
        params.representation.tpg.maxOutgoingEdges = 4;
        params.representation.lgp.pAdd = 0.5;
        params.representation.lgp.pDelete = 0.5;
        params.representation.lgp.pMutate = 1.0;
        params.representation.lgp.pSwap = 1.0;
        params.representation.lgp.pConstantMutation = 0.5;
        params.representation.lgp.minConstValue = 0;
        params.representation.lgp.maxConstValue = 1;
        params.representation.lgp.nbProgramConstant = 5;

        lgp = new Representation::LGPRepresentation(set, std::make_unique<Representation::RepresentationParameters>(params.representation));
        tpg = new Representation::TPGRepresentation(*lgp, std::make_unique<Representation::RepresentationParameters>(params.representation));

        selector = new Selector::TruncationSelector(std::make_unique<Selector::SelectionParameters>(params.selection));
        tpg->setSelector(*selector);
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

    ASSERT_NO_THROW(la = new Learn::LearningAgent(le, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation)))
        << "Construction of the LearningAgent failed.";

    ASSERT_NO_THROW(delete la) << "Destruction of the LearningAgent failed.";
}

TEST_F(LearningAgentTest, Init)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    Learn::LearningAgent la(le, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation));

    ASSERT_NO_THROW(la.init())
        << "Initialization of the LearningAgent should not fail.";
}

TEST_F(LearningAgentTest, addLogger)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    Learn::LearningAgent la(le, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation));

    Log::LALogger* l = nullptr;
    ASSERT_NO_THROW(
        l = new Log::LABasicLogger(la, std::cout)) // Call addLogger.
        << "Adding a logger should not fail.";
    if (l != nullptr) {
        delete l;
    }
}

TEST_F(LearningAgentTest, IsIndividualEvalSkipped)
{
    params.evaluation.maxNbEvaluationPerPolicy = 2;
    params.evaluation.nbIterationsPerPolicyEvaluation = 1;

    Learn::LearningAgent la(le, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation));
    la.init();

    // Test a result
    std::shared_ptr<Learn::EvaluationResult> result1;
    ASSERT_EQ(
        la.getNbEvaluationIndiv(result1, Learn::LearningMode::TRAINING),
        params.evaluation.nbIterationsPerPolicyEvaluation)
        << "Method should return false for a root that has never been "
           "evaluated before.";
                                ASSERT_EQ(result1, nullptr) << "Method should return a nullptr for a root "
       "that has not been evaluated before.";

    // Add an EvaluationResult artificially
    result1 = std::make_shared<Learn::EvaluationResult>(
        std::make_shared<Selector::SelectionMetrics>(1.0), 1);
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                    std::reference_wrapper<const Representation::Individual>> fakeMap{{result1, tpg->getIndividuals().at(0)}};
    tpg->getSelector().updateEvaluationRecords(fakeMap);

    // Test the result again
    ASSERT_EQ(la.getNbEvaluationIndiv(
        result1, Learn::LearningMode::TRAINING),
        1)
        << "Method should return 1";

    // Update the EvaluationResult artificially
    std::shared_ptr<Learn::EvaluationResult> result2 = std::make_shared<Learn::EvaluationResult>(
        std::make_shared<Selector::SelectionMetrics>(1.0), 2);
    fakeMap = {{result2, tpg->getIndividuals().at(0)}};
    tpg->getSelector().updateEvaluationRecords(fakeMap);

    // Test the result again
    ASSERT_EQ(la.getNbEvaluationIndiv(
        result2, Learn::LearningMode::TRAINING),
        0)
        << "Method should return 0.";
}


TEST_F(LearningAgentTest, MakeJobs)
{
    Learn::LearningAgent la(le, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation));
    la.init();
    la.setCurrentRepresentation(tpg);
    auto jobs = la.makeJobs(Learn::LearningMode::TRAINING);
    ASSERT_EQ(tpg->getIndividuals().size(), jobs.size())
        << "There should be as many jobs as roots";
    for (int i = 0; i < tpg->getIndividuals().size(); i++) {
        ASSERT_EQ(tpg->getIndividuals().at(i),
                  (*jobs.front()).getIndividual())
            << "Encapsulate the root in a job shouldn't change it";
        jobs.erase(jobs.begin());
    }
}

TEST_F(LearningAgentTest, EvalIndividual)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 1.0;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 10;

    Learn::LearningAgent la(le, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation));
    Representation::TPG::TPGArchive a; // For testing purposes, notmally, the archive from the
               // LearningAgent is used.

    la.init();
    std::unique_ptr<Representation::ExecutionEngine> execEngine = tpg->getPopulation().createExecutionEngine();

    std::shared_ptr<Learn::EvaluationResult> result;
    auto job = tpg->createJob(la.getRepresentationAt(tpg->getRepresentationID()).getIndividuals().at(0),
                           Learn::LearningMode::TRAINING, la.getRNG());
    la.setCurrentRepresentation(tpg);
    ASSERT_NO_THROW(
        result = la.evaluateJob(*execEngine, *job, 0, Learn::LearningMode::TRAINING, le))
        << "Evaluation from a root failed.";
    ASSERT_LE(result->getSelectionMetrics()->getScore(), 1.0)
        << "Average score should not exceed the score of a perfect player.";
}

TEST_F(LearningAgentTest, EvaluateOneRoot)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 1.0;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 10;

    Learn::LearningAgent la(le, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation));
    Representation::TPG::TPGArchive a; // For testing purposes, normally, the archive from the
               // LearningAgent is used.

    la.init();

    la.setCurrentRepresentation(tpg);
    std::shared_ptr<Learn::EvaluationResult> result;
    ASSERT_NO_THROW(
        result = la.evaluateOneIndividual(0, Learn::LearningMode::TRAINING,
                                    la.getRepresentationAt(tpg->getRepresentationID()).getIndividuals().at(0)))
        << "Evaluation from a root failed.";
    ASSERT_LE(result->getSelectionMetrics()->getScore(), 1.0)
        << "Average score should not exceed the score of a perfect player.";
}

TEST_F(LearningAgentTest, EvalAllRoots)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 10;

    tpg = new Representation::TPGRepresentation(*lgp, std::make_unique<Representation::RepresentationParameters>(params.representation));
    Learn::LearningAgent la(le, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation));

    la.init();
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  std::reference_wrapper<const Representation::Individual>>
        result;
    ASSERT_NO_THROW(result =
                        la.evaluateAllIndividuals(0, Learn::LearningMode::TRAINING))
        << "Evaluation from a root failed.";
    ASSERT_EQ(result.size(), la.getGraph().getNbRootVertices())
        << "Number of evaluated roots is under the number of roots from the "
           "Graph.";
}



TEST_F(LearningAgentTest, TrainOnegeneration)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 3;
    params.selection.truncation.ratioDeletedRoots =
        0.95; // high number to force the apparition of root action.

    // we will validate in order to cover validation log
    params.evaluation.doValidation = true;

    tpg = new Representation::TPGRepresentation(*lgp, std::make_unique<Representation::RepresentationParameters>(params.representation));
    Learn::LearningAgent la(le, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation));
    selector = new Selector::TruncationSelector(std::make_unique<Selector::SelectionParameters>(params.selection));
    la.getRepresentations().front().get().setSelector(*selector);

    la.init();

    // we add a logger to la to check it logs things
    std::ofstream o("tempFileForTest", std::ofstream::out);
    Log::LABasicLogger l(la, o);

    // Do the populate call to keep know the number of initial vertex
    Representation::TPG::TPGArchive a(0);
    Representation::Representation& tpgRef = la.getRepresentationAt(tpg->getRepresentationID());
    tpgRef.getMutator().mutatePopulation(la.getGraph(), tpgRef.getPopulation(), params.representation, la.getRNG());

    size_t initialNbVertex = la.getGraph().getNbVertices();
    // Seed selected so that an action becomes a root during next generation
    ASSERT_NO_THROW(la.trainOneGeneration(4, false))
        << "Training for one generation failed.";
    // Check the number of vertex in the graph.
    // Must be initial number of vertex - number of root removed
    ASSERT_EQ(la.getGraph().getNbVertices(),
              initialNbVertex -
                  floor(params.selection.truncation.ratioDeletedRoots *
                        params.representation.nbIndividuals))
        << "Number of remaining is under the number of roots from the "
           "Graph.";

    // Check that bestRoot has been set
    ASSERT_NE(tpgRef.getSelector().getBestIndividual().first, std::nullopt)
        << "Best root should be set after a trainOneGeneration iteration.";

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
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 5;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.evaluation.nbGenerations = 3;

    tpg = new Representation::TPGRepresentation(*lgp, std::make_unique<Representation::RepresentationParameters>(params.representation));
    Learn::LearningAgent la(le, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation));
    selector = new Selector::TruncationSelector(std::make_unique<Selector::SelectionParameters>(params.selection));
    la.getRepresentations().front().get().setSelector(*selector);

    la.init();
    bool alt = false;

    ASSERT_NO_THROW(la.train(alt, true))
        << "Training a TPG for several generation should not fail.";
    alt = true;
    ASSERT_NO_THROW(la.train(alt, true))
        << "Using the boolean reference to stop the training should not fail.";

    // For coverage
    params.evaluation.doValidation = true;
    params.evaluation.stepValidation = 2;
    tpg = new Representation::TPGRepresentation(*lgp, std::make_unique<Representation::RepresentationParameters>(params.representation));
    Learn::LearningAgent la2(le, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation));
    selector = new Selector::TruncationSelector(std::make_unique<Selector::SelectionParameters>(params.selection));
    la.getRepresentations().front().get().setSelector(*selector);
    alt = false;
    la2.init();
    ASSERT_NO_THROW(la2.train(alt, true))
        << "Using the boolean reference to stop the training should not fail.";
}

static void trainAndTestDeterminissm(Learn::LearningAgent& la, std::vector<size_t> expectedValues, bool print=false, bool is_init = false)
{
    if (!is_init){
        la.init();
    }
    bool alt = false;
    la.train(alt, false);
    EvoGraph::Graph& graph = la.getGraph();  

    uint64_t rngValue = la.getRNG().getUnsignedInt64(0, UINT64_MAX);
    
    if(print){
        // Useful when determinism is changed
        std::cout<<"{" << graph.getNbVertices() << ", "
                << graph.getNbRootVertices() << ", "
                << graph.getEdges().size() << ", "
                << EvoGraph::Vertex::getVertexIDCounter() << ", "
                << EvoGraph::Edge::getEdgeIDCounter() << ", "
                << Representation::Individual::getIndividualIDCounter() << ", "
                << rngValue
                << "}"<<std::endl;
        // Nice printed infos
        std::cout << "Graph number of vertices: " << graph.getNbVertices() << "\n"
                << "Number of root vertices: " << graph.getNbRootVertices() << "\n"
                << "Number of edges: " << graph.getEdges().size() << "\n"
                << "Vertex ID counter: " << EvoGraph::Vertex::getVertexIDCounter() << "\n"
                << "Edge ID counter: " << EvoGraph::Edge::getEdgeIDCounter() << "\n"
                << "Individual ID counter: " << Representation::Individual::getIndividualIDCounter() << "\n"

                << "RNG value: " << rngValue << std::endl;
    }
    
    // It is quite unlikely that two different TPGs after 20 generations
    // end up with the same number of vertices, roots, edges and calls to
    // the RNG without being identical.
    ASSERT_EQ(graph.getNbVertices(), expectedValues[0])
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(graph.getNbRootVertices(), expectedValues[1])
        << "Graph does not have the expected determinist characteristics.";
    ASSERT_EQ(graph.getEdges().size(), expectedValues[2])
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(EvoGraph::Vertex::getVertexIDCounter(), expectedValues[3])
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(EvoGraph::Edge::getEdgeIDCounter(), expectedValues[4])
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(Representation::Individual::getIndividualIDCounter(), expectedValues[5])
        << "Graph does not have the expected determinst characteristics.";
    ASSERT_EQ(rngValue, expectedValues[6])
        << "Graph does not have the expected determinst characteristics.";

}


// Similar to previous test, but verifications of graphs properties are here to
// ensure the result of the training is identical on all OSes and Compilers.
TEST_F(LearningAgentTest, TrainPortability)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 5;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.evaluation.nbGenerations = 20;
    params.representation.nbIndividuals = 30;
    // A root may be evaluated at most for 3 generations
    params.evaluation.maxNbEvaluationPerPolicy =
        params.evaluation.nbIterationsPerPolicyEvaluation * 3;
    params.evaluation.nbThreads = 3;

    tpg = new Representation::TPGRepresentation(*lgp, std::make_unique<Representation::RepresentationParameters>(params.representation));

    selector = new Selector::TruncationSelector(std::make_unique<Selector::SelectionParameters>(params.selection));
    tpg->setSelector(*selector);
    Learn::LearningAgent la(le, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation));

    trainAndTestDeterminissm(la, {32, 25, 83, 152, 518, 366, 2465407677219726593U}, false);
}

// Similar to previous test, but verifications of graphs properties are here to
// ensure the result of the training is identical on all OSes and Compilers.
TEST_F(LearningAgentTest, TrainLGPPortability)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 5;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.evaluation.nbGenerations = 20;
    params.representation.nbIndividuals = 30;
    // A root may be evaluated at most for 3 generations
    params.evaluation.maxNbEvaluationPerPolicy =
        params.evaluation.nbIterationsPerPolicyEvaluation * 3;
    params.evaluation.nbThreads = 3;
    params.representation.lgp.nbProgramConstant=0;


    auto lgp1 = new Representation::LGPRepresentation(set, std::make_unique<Representation::RepresentationParameters>(params.representation));
    Learn::LearningAgent la(le, *lgp1, std::make_unique<Learn::LearningParameters>(params.evaluation));
    selector = new Selector::TruncationSelector(std::make_unique<Selector::SelectionParameters>(params.selection));
    la.getRepresentations().front().get().setSelector(*selector);

    trainAndTestDeterminissm(la, {0, 0, 0, 0, 0, 144, 10372451540396096029U}, false);
}

// Similar to previous test, but verifications of graphs properties are here to
// ensure the result of the training is identical on all OSes and Compilers.
TEST_F(LearningAgentTest, TrainCGPPortability)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 5;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.evaluation.nbGenerations = 20;
    params.representation.nbIndividuals = 30;
    // A root may be evaluated at most for 3 generations
    params.evaluation.maxNbEvaluationPerPolicy =
        params.evaluation.nbIterationsPerPolicyEvaluation * 3;
    params.evaluation.nbThreads = 3;
    params.representation.lgp.pMutateOutput = 1;
    params.representation.cgp.nbLayers = 3;
    params.representation.cgp.nbNodesPerLayer = 5;
    params.representation.cgp.pMutateNode = params.representation.lgp.pMutate;

    Instructions::Set set2;
    set2.add(*(new Instructions::LambdaInstruction<int, int>([](int a, int b) -> double { return a - b; })));
    set2.add(*(new Instructions::LambdaInstruction<int, int>([](int a, int b) -> double { return a + b; })));
    auto cgp = new Representation::CGPRepresentation(set2, std::make_unique<Representation::RepresentationParameters>(params.representation));
    selector = new Selector::TruncationSelector(std::make_unique<Selector::SelectionParameters>(params.selection));
    cgp->setSelector(*selector);
    Learn::LearningAgent la(le, *cgp, std::make_unique<Learn::LearningParameters>(params.evaluation));

    trainAndTestDeterminissm(la, {0, 0, 0, 0, 0, 144, 15496931788443563000U}, false);
}
// Similar to previous test, but verifications of graphs properties are here to
// ensure the result of the training is identical on all OSes and Compilers.
TEST_F(LearningAgentTest, TrainTGPPortability)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 5;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.evaluation.nbGenerations = 20;
    params.representation.nbIndividuals = 30;
    // A root may be evaluated at most for 3 generations
    params.evaluation.maxNbEvaluationPerPolicy =
        params.evaluation.nbIterationsPerPolicyEvaluation * 3;
    params.evaluation.nbThreads = 3;
    params.representation.tgp.maxDepth = 4;
    params.representation.tgp.maxInitDepth = 2;
    params.representation.tgp.maxNbEdgePerNode = 2;

    
    Instructions::Set set2;
    set2.add(*(new Instructions::LambdaInstruction<int, int>([](int a, int b) -> double { return a - b; })));
    set2.add(*(new Instructions::LambdaInstruction<int, int>([](int a, int b) -> double { return a + b; })));
    
    auto tgp = new Representation::TGPRepresentation(set2, std::make_unique<Representation::RepresentationParameters>(params.representation));
    Learn::LearningAgent la(le, *tgp, std::make_unique<Learn::LearningParameters>(params.evaluation));
    selector = new Selector::TruncationSelector(std::make_unique<Selector::SelectionParameters>(params.selection));

    trainAndTestDeterminissm(la, {0, 0, 0, 0, 0, 315, 12566938387490903784U}, false);
}


// Same as previous, but with a TPGInstrumentedFactory
TEST_F(LearningAgentTest, TrainInstrumented)
{
    return;
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 5;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.evaluation.nbGenerations = 20;
    params.representation.nbIndividuals = 30;
    // A root may be evaluated at most for 3 generations
    params.evaluation.maxNbEvaluationPerPolicy =
        params.evaluation.nbIterationsPerPolicyEvaluation * 3;
    params.representation.lgp.forceProgramBehaviorChangeOnMutation = false;
    params.evaluation.nbThreads = 3;

    Learn::LearningAgent la(le, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation), EvoGraph::TPGInstrumentedFactory());

    trainAndTestDeterminissm(la, {29, 25, 93, 149, 534, 347, 10145656939287947145U}, false);

    /*
    //To help to refind the expectedValues if the determinism is changed by an update
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
    }

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
              63);*/
}

// Similar to previous test, but with continuous actions and no action programs
TEST_F(LearningAgentTest, TrainContinuousNoActionPrograms)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 5;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.evaluation.nbGenerations = 20;
    params.representation.nbIndividuals = 30;
    // A root may be evaluated at most for 3 generations
    params.evaluation.maxNbEvaluationPerPolicy =
        params.evaluation.nbIterationsPerPolicyEvaluation * 3;
    params.representation.lgp.forceProgramBehaviorChangeOnMutation = false;
    params.evaluation.nbThreads = 1;
    tpg = new Representation::TPGRepresentation(*lgp, std::make_unique<Representation::RepresentationParameters>(params.representation));

    Learn::LearningAgent la(cle, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation));
    selector = new Selector::TruncationSelector(std::make_unique<Selector::SelectionParameters>(params.selection));
    la.getRepresentations().front().get().setSelector(*selector);

    trainAndTestDeterminissm(la, {29, 25, 93, 149, 534, 347, 10145656939287947145U}, false);
}

// Similar to previous test, but with continuous actions and no action programs
TEST_F(LearningAgentTest, TrainContinuousWithSingleActionPrograms)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 5;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.evaluation.nbGenerations = 20;
    params.representation.atpg.probaContextOverActionProgram = 0.1;
    params.representation.nbIndividuals = 30;
    // A root may be evaluated at most for 3 generations
    params.evaluation.maxNbEvaluationPerPolicy =
        params.evaluation.nbIterationsPerPolicyEvaluation * 3;
    params.representation.lgp.forceProgramBehaviorChangeOnMutation = false;
    params.evaluation.nbThreads = 1;

    
    auto actionLgp = std::make_shared<Representation::LGPRepresentation>(set, std::make_unique<Representation::RepresentationParameters>(params.representation), "LGPAction");
    tpg = new Representation::ATPGRepresentation(*lgp, *actionLgp, std::make_unique<Representation::RepresentationParameters>(params.representation));
    Learn::LearningAgent la(cle, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation));
    selector = new Selector::TruncationSelector(std::make_unique<Selector::SelectionParameters>(params.selection));
    la.getRepresentations().front().get().setSelector(*selector);

    trainAndTestDeterminissm(la, {56, 24, 115, 235, 559, 431, 11530679664757575767U}, false);
}

// Similar to previous test, but with MATPG solution (no need for MAPLE because
// it is included in MATPG)
TEST_F(LearningAgentTest, TrainContinuousWithMATPG)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 5;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.evaluation.nbGenerations = 20;
    params.representation.nbIndividuals = 30;
    // A root may be evaluated at most for 3 generations
    params.evaluation.maxNbEvaluationPerPolicy =
        params.evaluation.nbIterationsPerPolicyEvaluation * 3;
    params.representation.lgp.forceProgramBehaviorChangeOnMutation = false;
    params.representation.maple.pMutateActionProgram = 0.9;
    params.representation.atpg.probaContextOverActionProgram = 0.9;
    params.representation.tpg.pProgramMutation = 0.6;
    params.evaluation.nbThreads = 1;

    
    auto actionLgp = std::make_shared<Representation::LGPRepresentation>(set, std::make_unique<Representation::RepresentationParameters>(params.representation), "LGPAction");
    auto actionMaple = new Representation::MapleRepresentation(*actionLgp, std::make_unique<Representation::RepresentationParameters>(params.representation));
    tpg = new Representation::ATPGRepresentation(*lgp, *actionMaple, std::make_unique<Representation::RepresentationParameters>(params.representation));
    Learn::LearningAgent la(cle, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation));
    selector = new Selector::TruncationSelector(std::make_unique<Selector::SelectionParameters>(params.selection));
    la.getRepresentations().front().get().setSelector(*selector);

    trainAndTestDeterminissm(la, {160, 96, 264, 717, 1225, 986, 16096251753827571400U}, false);
}



// Similar to previous test, but with MATPG solution (no need for MAPLE because
// it is included in MATPG)
TEST_F(LearningAgentTest, TrainContinuousWithMATPG_MapleInde)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 5;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.evaluation.nbGenerations = 20;
    params.representation.nbIndividuals = 30;
    // A root may be evaluated at most for 3 generations
    params.evaluation.maxNbEvaluationPerPolicy =
        params.evaluation.nbIterationsPerPolicyEvaluation * 3;
    params.representation.lgp.forceProgramBehaviorChangeOnMutation = false;
    params.representation.maple.pMutateActionProgram = 0.9;
    params.representation.atpg.probaContextOverActionProgram = 0.9;
    params.representation.tpg.pProgramMutation = 0.6;
    params.evaluation.nbThreads = 1;

    auto actionLgp = std::make_shared<Representation::LGPRepresentation>(set, std::make_unique<Representation::RepresentationParameters>(params.representation), "LGPAction");
    auto actionMaple = new Representation::MapleRepresentation(*actionLgp, std::make_unique<Representation::RepresentationParameters>(params.representation));
    auto atpg = new Representation::ATPGRepresentation(*lgp, *actionMaple, std::make_unique<Representation::RepresentationParameters>(params.representation));
    atpg->addAggregatedActionProgramRepresentation(*actionMaple);
    std::vector<std::reference_wrapper<Representation::Representation>> listAlgo = { *actionMaple, *atpg}; // ORDER MUST BE AUTOMATIC
    Learn::LearningAgent la(cle, listAlgo, std::make_unique<Learn::LearningParameters>(params.evaluation));
    selector = new Selector::TruncationSelector(std::make_unique<Selector::SelectionParameters>(params.selection));
    actionMaple->setSelector(*selector);
    selector = new Selector::TruncationSelector(std::make_unique<Selector::SelectionParameters>(params.selection));
    atpg->setSelector(*selector);


    trainAndTestDeterminissm(la, {175, 115, 316, 839, 1529, 1383, 2709782175046343699U}, false);
}

// Similar to previous test, but with continuous actions and no action programs
TEST_F(LearningAgentTest, TrainContinuousMaple)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 5;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.evaluation.nbGenerations = 20;
    params.representation.nbIndividuals = 30;
    params.representation.maple.pCrossIndividuals = 0.7;
    params.representation.maple.pCrossPrograms = 0.5;
    // A root may be evaluated at most for 3 generations
    params.evaluation.maxNbEvaluationPerPolicy =
        params.evaluation.nbIterationsPerPolicyEvaluation * 3;
    params.representation.lgp.forceProgramBehaviorChangeOnMutation = false;
    params.evaluation.nbThreads = 1;

    auto maple = new Representation::MapleRepresentation(*lgp, std::make_unique<Representation::RepresentationParameters>(params.representation));
    Learn::LearningAgent la(cle, *maple, std::make_unique<Learn::LearningParameters>(params.evaluation));
    selector = new Selector::TruncationSelector(std::make_unique<Selector::SelectionParameters>(params.selection));
    la.getRepresentations().front().get().setSelector(*selector);


    trainAndTestDeterminissm(la, {27, 24, 72, 151, 385, 330, 11466624659474942238U}, false);
}


// Similar to previous test, but with MATPG solution (no need for MAPLE because
// it is included in MATPG)
TEST_F(LearningAgentTest, TrainContinuousWithMATPGandLGPandMAPLE)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 5;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.evaluation.nbGenerations = 20;
    params.representation.nbIndividuals = 30;
    // A root may be evaluated at most for 3 generations
    params.evaluation.maxNbEvaluationPerPolicy =
        params.evaluation.nbIterationsPerPolicyEvaluation * 3;
    params.representation.lgp.forceProgramBehaviorChangeOnMutation = false;
    params.representation.maple.pMutateActionProgram = 0.9;
    params.representation.atpg.probaContextOverActionProgram = 0.9;
    params.representation.tpg.pProgramMutation = 0.6;
    params.evaluation.nbThreads = 1;

    
    auto actionLgp = std::make_shared<Representation::LGPRepresentation>(set, std::make_unique<Representation::RepresentationParameters>(params.representation), "LGPAction");
    auto actionMaple =
        new Representation::MapleRepresentation(*actionLgp, std::make_unique<Representation::RepresentationParameters>(params.representation), "MAPLEAction");
    auto matpg = new Representation::ATPGRepresentation(*lgp, *actionMaple, std::make_unique<Representation::RepresentationParameters>(params.representation), "MATPG");

    auto standaloneLGP = std::make_shared<Representation::LGPRepresentation>(set, std::make_unique<Representation::RepresentationParameters>(params.representation), "LGP1");

    auto standaloneLGPforMaple = std::make_shared<Representation::LGPRepresentation>(set, std::make_unique<Representation::RepresentationParameters>(params.representation), "LGP2");
    auto standaloneMaple = new Representation::MapleRepresentation(*standaloneLGPforMaple, std::make_unique<Representation::RepresentationParameters>(params.representation), "Maple2");
    std::vector<std::reference_wrapper<Representation::Representation>> listAlgo = {*matpg, *standaloneLGP, *standaloneMaple};
    Learn::LearningAgent la(cle, listAlgo, std::make_unique<Learn::LearningParameters>(params.evaluation));

    selector = new Selector::TruncationSelector(std::make_unique<Selector::SelectionParameters>(params.selection));
    standaloneLGP->setSelector(*selector);
    selector = new Selector::TruncationSelector(std::make_unique<Selector::SelectionParameters>(params.selection));
    matpg->setSelector(*selector);
    selector = new Selector::TruncationSelector(std::make_unique<Selector::SelectionParameters>(params.selection));
    standaloneMaple->setSelector(*selector);


    trainAndTestDeterminissm(la, {186, 123, 317, 833, 1528, 1505, 5900707158955917725U}, false);
}


// Similar to previous test, but with MATPG solution (no need for MAPLE because
// it is included in MATPG)
TEST_F(LearningAgentTest, TrainContinuousWithMATPGandLGPandMAPLETournament)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 5;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.evaluation.nbGenerations = 20;
    params.representation.nbIndividuals = 30;
    // A root may be evaluated at most for 3 generations
    params.evaluation.maxNbEvaluationPerPolicy =
        params.evaluation.nbIterationsPerPolicyEvaluation * 3;
    params.representation.lgp.forceProgramBehaviorChangeOnMutation = false;
    params.representation.maple.pMutateActionProgram = 0.9;
    params.representation.atpg.probaContextOverActionProgram = 0.9;
    params.representation.tpg.pProgramMutation = 0.6;
    params.evaluation.nbThreads = 1;
    params.selection._selectionMode = "tournament";

    
    auto actionLgp = std::make_shared<Representation::LGPRepresentation>(set, std::make_unique<Representation::RepresentationParameters>(params.representation), "LGPAction");
    auto actionMaple =
        new Representation::MapleRepresentation(*actionLgp, std::make_unique<Representation::RepresentationParameters>(params.representation), "MAPLEAction");
    auto matpg = new Representation::ATPGRepresentation(*lgp, *actionMaple, std::make_unique<Representation::RepresentationParameters>(params.representation), "MATPG");

    auto standaloneLGP = std::make_shared<Representation::LGPRepresentation>(set, std::make_unique<Representation::RepresentationParameters>(params.representation), "LGP1");

    auto standaloneLGPforMaple = std::make_shared<Representation::LGPRepresentation>(set, std::make_unique<Representation::RepresentationParameters>(params.representation), "LGP2");
    auto standaloneMaple = new Representation::MapleRepresentation(*standaloneLGPforMaple, std::make_unique<Representation::RepresentationParameters>(params.representation), "Maple2");
    std::vector<std::reference_wrapper<Representation::Representation>> listAlgo = {*matpg, *standaloneLGP, *standaloneMaple};
    Learn::LearningAgent la(cle, listAlgo, std::make_unique<Learn::LearningParameters>(params.evaluation));
    selector = new Selector::TournamentSelector(std::make_unique<Selector::SelectionParameters>(params.selection));
    standaloneLGP->setSelector(*selector);
    selector = new Selector::TournamentSelector(std::make_unique<Selector::SelectionParameters>(params.selection));
    matpg->setSelector(*selector);
    selector = new Selector::TournamentSelector(std::make_unique<Selector::SelectionParameters>(params.selection));
    standaloneMaple->setSelector(*selector);


    trainAndTestDeterminissm(la, {162, 111, 315, 1836, 3707, 3353, 13146969190562730162U}, false);
}


// Similar to previous test, but with continuous actions and no action programs
TEST_F(LearningAgentTest, TrainContinuousMapleMAPElites)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 5;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.evaluation.nbGenerations = 20;
    params.representation.nbIndividuals = 30;
    // A root may be evaluated at most for 3 generations
    params.evaluation.maxNbEvaluationPerPolicy =
        params.evaluation.nbIterationsPerPolicyEvaluation * 3;
    params.representation.lgp.forceProgramBehaviorChangeOnMutation = false;
    params.evaluation.nbThreads = 1;
    params.selection._selectionMode = "mapElites";

    auto maple = new Representation::MapleRepresentation(*lgp, std::make_unique<Representation::RepresentationParameters>(params.representation));
    Learn::LearningAgent la(cle, *maple, std::make_unique<Learn::LearningParameters>(params.evaluation));
    selector = new Selector::MapElitesSelector(std::make_unique<Selector::SelectionParameters>(params.selection), params.evaluation.maxNbEvaluationPerPolicy);
    maple->setSelector(*selector);

    la.init();
    Selector::MapElitesSelector& mapElitesSelector =
        dynamic_cast<Selector::MapElitesSelector&>(
            *selector);
    std::shared_ptr<Selector::MapElites::MapElitesDescriptor> descriptor = std::make_shared<Selector::MapElites::DefaultDescriptors::ActionValues>();
	descriptor->initDescriptor(la.getGraph(), cle);
    mapElitesSelector.addArchiveFromDescriptor(30, descriptor, cle);


    trainAndTestDeterminissm(la, {30, 27, 66, 1048, 3293, 2100, 5387768408616106558U}, false, true);
}


// Similar to previous test, but with continuous actions and no action programs
TEST_F(LearningAgentTest, TrainContinuousMapleCvtMAPElites)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 5;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.evaluation.nbGenerations = 20;
    params.representation.nbIndividuals = 30;
    // A root may be evaluated at most for 3 generations
    params.evaluation.maxNbEvaluationPerPolicy =
        params.evaluation.nbIterationsPerPolicyEvaluation * 3;
    params.representation.lgp.forceProgramBehaviorChangeOnMutation = false;
    params.evaluation.nbThreads = 1;
    params.selection._selectionMode = "mapElites";

    auto maple = new Representation::MapleRepresentation(*lgp, std::make_unique<Representation::RepresentationParameters>(params.representation));
    Learn::LearningAgent la(cle, *maple, std::make_unique<Learn::LearningParameters>(params.evaluation));
    selector = new Selector::MapElitesSelector(std::make_unique<Selector::SelectionParameters>(params.selection), params.evaluation.maxNbEvaluationPerPolicy);
    maple->setSelector(*selector);

    la.init();
    Selector::MapElitesSelector& mapElitesSelector =
        dynamic_cast<Selector::MapElitesSelector&>(
            maple->getSelector());
    std::shared_ptr<Selector::MapElites::MapElitesDescriptor> descriptor =
        std::make_shared<
            Selector::MapElites::DefaultDescriptors::ActionValues>();
    descriptor->initDescriptor(la.getGraph(), cle);
    mapElitesSelector.addCvtArchiveFromDescriptor(30, descriptor, cle, la.getRNG());

    trainAndTestDeterminissm(la, {11, 8, 16, 751, 1980, 1497, 13259124827853609931U}, false, true);
}


/*
TEST_F(LearningAgentTest, GraphCleanProgramIntrons)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 1;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.evaluation.nbGenerations = 5;

    Learn::LearningAgent la(le, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation));
    la.init();
    bool alt = false;
    la.train(alt, false);


    EvoGraph::Graph& graph = la.getGraph();
    
    la.getRepresentationAt(0).getSelector().keepBestPolicy(la.getGraph());

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
    std::unique_ptr<Representation::ExecutionEngine> execEngine = tpg->createExecutionEngine();

    std::vector<const EvoGraph::Vertex*> pathOrigin =
        tee.executeFromRoot(*(graph.getRootVertices().at(0))).first;

    // Clear introns
    la.getRepresentationAt(0).clearUnusedIndividualParts();

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

TEST_F(LearningAgentTest, TrainOnegenerationContinuousNoActionProg)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 3;
    params.selection.truncation.ratioDeletedRoots =
        0.5; // high number to force the apparition of root action.
    params.evaluation.nbThreads = 1;
    tpg = new Representation::TPGRepresentation(params, *lgp);
    Learn::LearningAgent la(cle, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation));

    la.init();
    // Do the populate call to keep know the number of initial vertex
    Representation::TPG::TPGArchive a(0);

    tpg->getMutator()->mutatePopulation(la.getGraph(), tpg->getPopulation(), params, la.getRNG());
    size_t initialNbVertex = la.getGraph().getNbVertices();

    // Seed selected so that an action becomes a root during next generation
    ASSERT_NO_THROW(la.trainOneGeneration(4))
        << "Training for one generation failed.";
    // Check the number of vertex in the graph.
    // Must be lower or equal to initial number of vertex - number of root
    // removed (since some actions vertex are removed too)
    ASSERT_LE(la.getGraph().getNbVertices(),
              initialNbVertex -
                  floor(params.selection.truncation.ratioDeletedRoots *
                        params.representation.nbIndividuals))
        << "Number of remaining is under the number of roots from the "
           "Graph.";
    // Train a second generation, because most roots were removed, a root
    // actions have appeared and the training representation will attempt to remove
    // them.
    ASSERT_NO_THROW(la.trainOneGeneration(0))
        << "Training for one generation failed.";
}
    */

TEST_F(ParallelLearningAgentTest, Constructor)
{
    Learn::ParallelLearningAgent* pla;

    ASSERT_NO_THROW(pla = new Learn::ParallelLearningAgent(le, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation)))
        << "Construction of the LearningAgent failed.";

    ASSERT_NO_THROW(delete pla) << "Destruction of the LearningAgent failed.";
}

TEST_F(ParallelLearningAgentTest, Init)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    Learn::ParallelLearningAgent pla(le, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation));

    ASSERT_NO_THROW(pla.init())
        << "Initialization of the LearningAgent should not fail.";

}

/*
TEST_F(ParallelLearningAgentTest, EvalRootSequential)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 1.0;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 10;
    params.evaluation.nbThreads = 1;

    Learn::ParallelLearningAgent pla(le, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation));
    Representation::TPG::TPGArchive a; // For testing purposes, notmally, the archive from the
               // LearningAgent is used.

    pla.init();
    std::unique_ptr<Representation::ExecutionEngine> execEngine = tpg->getPopulation().createExecutionEngine();

    std::shared_ptr<Learn::EvaluationResult> result;
    auto job = tpg->createJob(pla.getRepresentationAt(0).getIndividuals().at(0),
                           Learn::LearningMode::TRAINING, pla.getRNG());
    pla.setCurrentRepresentation(tpg);
    ASSERT_NO_THROW(
        result = pla.evaluateJob(*execEngine, *job, 0, Learn::LearningMode::TRAINING, le))
        << "Evaluation from a root failed.";
    ASSERT_LE(result->getSelectionMetrics()->getScore(), 1.0)
        << "Average score should not exceed the score of a perfect player.";
}*/

TEST_F(ParallelLearningAgentTest, EvalAllRootsSequential)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 10;
    params.evaluation.nbThreads = 1;

    Learn::ParallelLearningAgent pla(le, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation));

    pla.init();
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  std::reference_wrapper<const Representation::Individual>>
        result;
    ASSERT_NO_THROW(result =
                        pla.evaluateAllIndividuals(0, Learn::LearningMode::TRAINING))
        << "Evaluation from a root failed.";
    ASSERT_EQ(result.size(), pla.getGraph().getNbRootVertices())
        << "Number of evaluated roots is under the number of roots from the "
           "Graph.";
}

TEST_F(ParallelLearningAgentTest, EvalAllRootsParallel)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 10;
    params.evaluation.nbThreads = 4;

    tpg = new Representation::TPGRepresentation(*lgp, std::make_unique<Representation::RepresentationParameters>(params.representation));
    Learn::ParallelLearningAgent pla(le, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation));

    pla.init();
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  std::reference_wrapper<const Representation::Individual>>
        result;
    ASSERT_NO_THROW(result =
                        pla.evaluateAllIndividuals(0, Learn::LearningMode::TRAINING))
        << "Evaluation from a root failed.";
    ASSERT_EQ(result.size(), pla.getGraph().getNbRootVertices())
        << "Number of evaluated roots is under the number of roots from the "
           "Graph.";
}

TEST_F(ParallelLearningAgentTest, EvalAllRootsParallelTrainingDeterminism)
{
    // Check that parallel execution leads to the exact same results as
    // sequential
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.1;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 10;


    auto tpgLa = new Representation::TPGRepresentation(*lgp, std::make_unique<Representation::RepresentationParameters>(params.representation));
    Learn::LearningAgent la(le, *tpgLa, std::make_unique<Learn::LearningParameters>(params.evaluation));
    la.init(0); // Reset RNG to 0
    auto results = la.evaluateAllIndividuals(0, Learn::LearningMode::TRAINING);
    auto nextInt = la.getRNG().getUnsignedInt64(0, UINT64_MAX);

    Learn::LearningParameters paramsSequential = params.evaluation;
    paramsSequential.nbThreads = 1;
    auto tpgSequential = new Representation::TPGRepresentation(*lgp, std::make_unique<Representation::RepresentationParameters>(params.representation));
    Learn::ParallelLearningAgent plaSequential(le, *tpgSequential, std::make_unique<Learn::LearningParameters>(paramsSequential));

    plaSequential.init(0); // Reset centralized RNG to 0
    auto resultsSequential =
        plaSequential.evaluateAllIndividuals(0, Learn::LearningMode::TRAINING);
    auto nextIntSequential =
        plaSequential.getRNG().getUnsignedInt64(0, UINT64_MAX);

    Learn::LearningParameters paramsParallel = params.evaluation;
    paramsParallel.nbThreads = 4;
    auto tpgParallel = new Representation::TPGRepresentation(*lgp, std::make_unique<Representation::RepresentationParameters>(params.representation));
    Learn::ParallelLearningAgent plaParallel(le, *tpgParallel, std::make_unique<Learn::LearningParameters>(paramsParallel));

    plaParallel.init(0); // Reset centralized RNG to 0
    auto resultsParallel =
        plaParallel.evaluateAllIndividuals(0, Learn::LearningMode::TRAINING);
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

    // Check determinism of bestIndividual score
    ASSERT_EQ(tpgLa->getSelector().getBestIndividual().first,
              tpgParallel->getSelector().getBestIndividual().first);

    // Check determinism of the number of RNG calls.
    ASSERT_EQ(nextInt, nextIntSequential)
        << "RNG::RNG was called a different number of time in parallel and "
           "sequential execution.";

    // Check archives
    ASSERT_GT(tpgLa->getArchive().getNbRecordings(), 0)
        << "For the archive determinism tests to be meaningful, Representation::TPG::TPGArchive should "
           "not be empty.";
    ASSERT_EQ(tpgLa->getArchive().getNbRecordings(),
              tpgSequential->getArchive().getNbRecordings())
        << "Archives have different sizes.";
    for (auto i = 0; i < tpgLa->getArchive().getNbRecordings(); i++) {
        ASSERT_EQ(tpgLa->getArchive().at(i).dataHash,
                  tpgSequential->getArchive().at(i).dataHash)
            << "Archives have different content.";
        ASSERT_EQ(tpgLa->getArchive().at(i).result,
                  tpgSequential->getArchive().at(i).result)
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

    // Check determinism of bestIndividual score
    ASSERT_EQ(tpgSequential->getSelector().getBestIndividual().first,
              tpgParallel->getSelector().getBestIndividual().first);

    // Check determinism of the number of RNG calls.
    ASSERT_EQ(nextIntSequential, nextIntParallel)
        << "RNG::RNG was called a different number of time in parallel and "
           "sequential execution.";

    // Check archives
    ASSERT_EQ(tpgParallel->getArchive().getNbRecordings(),
              tpgSequential->getArchive().getNbRecordings())
        << "Archives have different sizes.";
    for (auto i = 0; i < tpgParallel->getArchive().getNbRecordings(); i++) {
        ASSERT_EQ(tpgParallel->getArchive().at(i).dataHash,
                  tpgSequential->getArchive().at(i).dataHash)
            << "Archives have different content.";
        ASSERT_EQ(tpgParallel->getArchive().at(i).result,
                  tpgSequential->getArchive().at(i).result)
            << "Archives have different content.";
    }
}

TEST_F(ParallelLearningAgentTest, EvalAllRootsParallelValidationDeterminism)
{
    // Check that parallel execution leads to the exact same results as
    // sequential
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.1;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 10;

    auto tpgLa = new Representation::TPGRepresentation(*lgp, std::make_unique<Representation::RepresentationParameters>(params.representation));
    Learn::LearningAgent la(le, *tpgLa, std::make_unique<Learn::LearningParameters>(params.evaluation));
    la.init(0); // Reset centralized RNG to 0
    auto results = la.evaluateAllIndividuals(0, Learn::LearningMode::VALIDATION);
    auto nextInt = la.getRNG().getUnsignedInt64(0, UINT64_MAX);

    Learn::LearningParameters paramsSequential = params.evaluation;
    paramsSequential.nbThreads = 1;
    
    auto tpgSequential = new Representation::TPGRepresentation(*lgp, std::make_unique<Representation::RepresentationParameters>(params.representation));
    Learn::ParallelLearningAgent plaSequential(le, *tpgSequential, std::make_unique<Learn::LearningParameters>(paramsSequential));

    plaSequential.init(0); // Reset centralized RNG to 0
    auto resultsSequential =
        plaSequential.evaluateAllIndividuals(0, Learn::LearningMode::VALIDATION);
    auto nextIntSequential =
        plaSequential.getRNG().getUnsignedInt64(0, UINT64_MAX);

    Learn::LearningParameters paramsParallel = params.evaluation;
    paramsParallel.nbThreads = 4;
    auto tpgParallel = new Representation::TPGRepresentation(*lgp, std::make_unique<Representation::RepresentationParameters>(params.representation));
    Learn::ParallelLearningAgent plaParallel(le, *tpgParallel, std::make_unique<Learn::LearningParameters>(paramsParallel));

    plaParallel.init(0); // Reset centralized RNG to 0
    auto resultsParallel =
        plaParallel.evaluateAllIndividuals(0, Learn::LearningMode::VALIDATION);
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
    ASSERT_EQ(tpgLa->getArchive().getNbRecordings(), 0)
        << "Archives should be empty in Validation mode.";
    ASSERT_EQ(tpgSequential->getArchive().getNbRecordings(), 0)
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
    ASSERT_EQ(tpgParallel->getArchive().getNbRecordings(), 0)
        << "Archives should be empty in Validation mode.";
}
/*
TEST_F(ParallelLearningAgentTest, TrainOnegenerationSequential)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 3;
    params.selection.truncation.ratioDeletedRoots =
        0.85; // high number to force the apparition of root action.
    params.evaluation.nbThreads = 1;

    Learn::ParallelLearningAgent pla(le, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation));
    selector = new Selector::TruncationSelector(std::make_unique<Selector::SelectionParameters>(params.selection));
    pla.getRepresentations().front().get().setSelector(*selector);

    pla.init();
    // Do the populate call to keep know the number of initial vertex
    Representation::TPG::TPGArchive a(0);
    Representation::Representation& tpg = pla.getRepresentationAt(0);
    tpg.getMutator().mutatePopulation(pla.getGraph(), tpg.getPopulation(), params.representation, pla.getRNG());
    size_t initialNbVertex = pla.getGraph().getNbVertices();
    // Seed selected so that an action becomes a root during next generation
    ASSERT_NO_THROW(pla.trainOneGeneration(4, false))
        << "Training for one generation failed.";
    // Check the number of vertex in the graph.
    // Must be initial number of vertex - number of root removed
    ASSERT_EQ(pla.getGraph().getNbVertices(),
              initialNbVertex -
                  floor(params.selection.truncation.ratioDeletedRoots *
                        params.representation.nbIndividuals))
        << "Number of remaining is under the number of roots from the "
           "Graph.";

    // Check that bestRoot has been set
    ASSERT_NE(tpg.getSelector().getBestIndividual().first, std::nullopt)
        << "Best root should not be expired after training one generation.";
}*/

TEST_F(ParallelLearningAgentTest, TrainOneGenerationParallel)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 3;
    params.selection.truncation.ratioDeletedRoots =
        0.85; // high number to force the apparition of root action.
    params.evaluation.nbThreads = 4;

    tpg = new Representation::TPGRepresentation(*lgp, std::make_unique<Representation::RepresentationParameters>(params.representation));
    Learn::ParallelLearningAgent pla(le, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation));
    selector = new Selector::TruncationSelector(std::make_unique<Selector::SelectionParameters>(params.selection));
    pla.getRepresentations().front().get().setSelector(*selector);

    pla.init();
    // Do the populate call to keep know the number of initial vertex
    tpg->getMutator().mutatePopulation(pla.getGraph(), tpg->getPopulation(), params.representation, pla.getRNG());
    
    size_t initialNbVertex = pla.getGraph().getNbVertices();
    // Seed selected so that an action becomes a root during next generation
    ASSERT_NO_THROW(pla.trainOneGeneration(4, false))
        << "Training for one generation failed.";
    // Check the number of vertex in the graph.
    // Must be initial number of vertex - number of root removed
    ASSERT_EQ(pla.getGraph().getNbVertices(),
              initialNbVertex -
                  floor(params.selection.truncation.ratioDeletedRoots *
                        params.representation.nbIndividuals))
        << "Number of remaining is under the number of roots from the "
           "Graph.";
}

TEST_F(ParallelLearningAgentTest, TrainSequential)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 5;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.evaluation.nbGenerations = 3;
    params.evaluation.maxNbEvaluationPerPolicy =
        params.evaluation.nbIterationsPerPolicyEvaluation * 2;
    params.evaluation.nbThreads = 1;

    tpg = new Representation::TPGRepresentation(*lgp, std::make_unique<Representation::RepresentationParameters>(params.representation));
    Learn::ParallelLearningAgent pla(le, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation));
    selector = new Selector::TruncationSelector(std::make_unique<Selector::SelectionParameters>(params.selection));
    pla.getRepresentations().front().get().setSelector(*selector);

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
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 5;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.evaluation.nbGenerations = 3;
    params.evaluation.maxNbEvaluationPerPolicy =
        params.evaluation.nbIterationsPerPolicyEvaluation * 2;
    params.evaluation.nbThreads = std::thread::hardware_concurrency();

    Learn::ParallelLearningAgent pla(le, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation));

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
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 5;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    // Set a large number of generations and roots
    // so that the chances of something going wrong is higher.
    params.evaluation.nbGenerations = 20;
    params.representation.nbIndividuals = 30;
    params.evaluation.maxNbEvaluationPerPolicy =
        params.evaluation.nbIterationsPerPolicyEvaluation * 5;

    Learn::LearningAgent la(le, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation));
    selector = new Selector::TruncationSelector(std::make_unique<Selector::SelectionParameters>(params.selection));
    la.getRepresentations().front().get().setSelector(*selector);

    la.init();

    // Train for several generation
    bool alt = false;
    la.train(alt, false);

    params.evaluation.nbThreads = 4;
    Learn::ParallelLearningAgent pla(le, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation));
    selector = new Selector::TruncationSelector(std::make_unique<Selector::SelectionParameters>(params.selection));
    pla.getRepresentations().front().get().setSelector(*selector);

    pla.init();

    // Train for several generation
    pla.train(alt, false);

    // Check number of vertex in graphs
    // Non-zero to avoid false positive.
    // These checks guarantee determinism between sequential and parallel
    // version on a given platform. They do not guarantee portability between
    // compilers and OS
    ASSERT_GT(la.getGraph().getNbVertices(), 0)
        << "Number of vertex in the trained graph should not be 0.";
    ASSERT_EQ(la.getGraph().getNbVertices(),
              pla.getGraph().getNbVertices())
        << "LearningAgent and ParallelLearning Agent result in different "
           "Graphs.";
}

TEST_F(ParallelLearningAgentTest, TrainPortability)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 5;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.evaluation.nbGenerations = 20;
    params.representation.nbIndividuals = 30;
    // A root may be evaluated at most for 3 generations
    params.evaluation.maxNbEvaluationPerPolicy =
        params.evaluation.nbIterationsPerPolicyEvaluation * 3;
    params.evaluation.nbThreads = 3;

    tpg = new Representation::TPGRepresentation(*lgp, std::make_unique<Representation::RepresentationParameters>(params.representation));
    Learn::ParallelLearningAgent la(le, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation));
    selector = new Selector::TruncationSelector(std::make_unique<Selector::SelectionParameters>(params.selection));
    la.getRepresentations().front().get().setSelector(*selector);

    trainAndTestDeterminissm(la, {32, 25, 83, 152, 518, 366, 2465407677219726593U}, false);
}

/*
TEST_F(ParallelLearningAgentTest, KeepBestPolicy)
{
    params.representation.tpg.archiveSize = 50;
    params.representation.tpg.archivingProbability = 0.5;
    params.evaluation.maxNbActionsPerEval = 11;
    params.evaluation.nbIterationsPerPolicyEvaluation = 1;
    params.selection.truncation.ratioDeletedRoots = 0.2;
    params.evaluation.nbGenerations = 5;
    params.evaluation.maxNbEvaluationPerPolicy =
        params.evaluation.nbIterationsPerPolicyEvaluation * 2;

    Learn::ParallelLearningAgent pla(le, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation));
    pla.init();
    bool alt = false;
    pla.train(alt, true);

    ASSERT_NO_THROW(pla.getSelector().keepBestPolicy())
        << "Keeping the best policy after training should not fail.";
    ASSERT_EQ(pla.getGraph().getNbRootVertices(), 1)
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

    params.evaluation.nbIterationsPerPolicyEvaluation = 2;
    Learn::LearningAgent la(utilityEnv, set, std::make_unique<Learn::LearningParameters>(params.evaluation));
    la.init();

    Representation::TPG::TPGArchive a;
    EvoGraph::OldExecutionEngine tee(la.getGraph().getEnvironment(), &a);
    auto job = *la.makeJob(la.getGraph().getRootVertices().at(0),
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
    Learn::LearningAgent la(le, *tpg, std::make_unique<Learn::LearningParameters>(params.evaluation));
    la.init();
    EvoGraph::Team fakeTeam;
    ASSERT_THROW(
        la.evaluateOneRoot(0, Learn::LearningMode::TRAINING, &fakeTeam),
        std::runtime_error);
}
*/