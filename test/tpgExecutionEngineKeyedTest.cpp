#include <gtest/gtest.h>

#include "data/dataHandler.h"
#include "environment.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/multByConstant.h"
#include "program/program.h"

#include "tpg/keyed/tpgEdgeKeyed.h"
#include "tpg/keyed/tpgExecutionEngineKeyed.h"
#include "tpg/keyed/tpgGraphKeyed.h"
#include "tpg/keyed/tpgKeyedFactory.h"
#include "tpg/keyed/tpgTeamKeyed.h"
#include "tpg/tpgGraph.h"
#include "util/counterReset.h"

class TPGExecutionEngineKeyedTest : public ::testing::Test
{
  protected:
    const size_t size1{24};
    const size_t size2{32};
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
    Instructions::Set set;
    Environment* e = NULL;
    Learn::LearningParameters params;
    std::vector<std::shared_ptr<Program::Program>> progPointers;

    TPG::TPGGraph* tpg;
    std::vector<const TPG::TPGEdge*> edges;
    Archive a;

    /**
     * Populate the program instructions so that it returns the given value.
     *
     * \param[in] value a double value between 0 and 10.
     */
    void makeProgramReturn(Program::Program& prog, double value)
    {
        auto& line = prog.addNewLine();
        // do a multby constant with DHandler 0
        line.setInstructionIndex(1);
        line.setOperand(0, 2, 0);    // Dhandler 0 location 0
        line.setOperand(1, 1, 0);    // CHandler at location 0
        line.setDestinationIndex(0); // 0th register dest
        prog.getConstantHandler().setDataAt(typeid(Data::Constant), 0,
                                            {static_cast<double>(value)});
    }

    virtual void SetUp()
    {
        CounterReset::counterReset();

        // Setup environment
        vect.push_back(
            *(new Data::PrimitiveTypeArray<double>((unsigned int)size1)));
        vect.push_back(
            *(new Data::PrimitiveTypeArray<int>((unsigned int)size2)));

        // Put a 1 in the dataHandler to make it easy to have non-zero return in
        // Programs.
        ((Data::PrimitiveTypeArray<double>&)vect.at(0).get())
            .setDataAt(typeid(double), 0, 1.0);

        set.add(*(new Instructions::AddPrimitiveType<double>()));
        set.add(*(new Instructions::MultByConstant<double>()));

        params.nbRegisters = 8;
        params.nbProgramConstant = 1;
        e = new Environment(set, params, vect);
        tpg = new TPG::TPGGraphKeyed(*e,
                                     std::make_unique<TPG::TPGKeyedFactory>());

        // Create 7 programs
        for (int i = 0; i < 7; i++) {
            progPointers.push_back(std::shared_ptr<Program::Program>(
                new Program::Program(*e, false)));
        }

        // Create a TPG with keyed teams and edges
        // (T= Team, A= Action)
        // Keys and locks next to vertices and edges respectively
        //
        // T0(2)          T1(3)
        // |\             /|
        // | \2         1/ |
        // |  '->T2(1)<-`  |3
        // |1   /|\        |
        // V  3/ | \6      V
        // A0<'  |2 '----->A1
        //       V
        //       A2

        // With four action and four teams
        uint64_t keys[] = {2, 3, 1};
        for (int i = 0; i < 3; i++) {
            auto& team = tpg->addNewTeam();
            dynamic_cast<TPG::TPGGraphKeyed*>(tpg)->setNewTeamKey(
                dynamic_cast<const TPG::TPGTeamKeyed&>(team), keys[i]);
        }

        uint64_t locks[] = {1, 1, 2};
        for (int i = 0; i < 3; i++) {
            // Each action is linked to a team (and vice-versa)
            tpg->addNewAction(i);
            auto edge = &tpg->addNewEdge(*tpg->getVertices().at(i),
                                         *tpg->getVertices().back(),
                                         progPointers.at(i));
            // Set the lock for the edge
            dynamic_cast<TPG::TPGGraphKeyed*>(tpg)->setNewEdgeLock(
                dynamic_cast<const TPG::TPGEdgeKeyed&>(*edge), locks[i]);

            edges.push_back(edge);
        }

        // Add missing Edges between teams
        // T0 -(2)-> T2
        auto edge =
            &tpg->addNewEdge(*tpg->getVertices().at(0),
                             *tpg->getVertices().at(2), progPointers.at(3));
        dynamic_cast<TPG::TPGGraphKeyed*>(tpg)->setNewEdgeLock(
            dynamic_cast<const TPG::TPGEdgeKeyed&>(*edge), 2);
        edges.push_back(edge);

        // T1 -(3)-> T2
        edge = &tpg->addNewEdge(*tpg->getVertices().at(1),
                                *tpg->getVertices().at(2), progPointers.at(4));
        dynamic_cast<TPG::TPGGraphKeyed*>(tpg)->setNewEdgeLock(
            dynamic_cast<const TPG::TPGEdgeKeyed&>(*edge), 3);
        edges.push_back(edge);

        // T2 -(1)-> A0
        edge = &tpg->addNewEdge(*tpg->getVertices().at(2),
                                *tpg->getVertices().at(3), progPointers.at(5));
        dynamic_cast<TPG::TPGGraphKeyed*>(tpg)->setNewEdgeLock(
            dynamic_cast<const TPG::TPGEdgeKeyed&>(*edge), 3);
        edges.push_back(edge);

        // T2 -(6)-> A1
        edge = &tpg->addNewEdge(*tpg->getVertices().at(2),
                                *tpg->getVertices().at(4), progPointers.at(6));
        dynamic_cast<TPG::TPGGraphKeyed*>(tpg)->setNewEdgeLock(
            dynamic_cast<const TPG::TPGEdgeKeyed&>(*edge), 6);
        edges.push_back(edge);

        // Put a weight on edges
        makeProgramReturn(*progPointers.at(0), 0); // T0->A0
        makeProgramReturn(*progPointers.at(1), 5); // T1->A1
        makeProgramReturn(*progPointers.at(2), 3); // T2->A2
        makeProgramReturn(*progPointers.at(3), 8); // T0->T2
        makeProgramReturn(*progPointers.at(4), 8); // T1->T2
        makeProgramReturn(*progPointers.at(5), 9); // T2->A0
        makeProgramReturn(*progPointers.at(6), 6); // T2->A1

        // Check the characteristics
        ASSERT_EQ(tpg->getNbVertices(), 6);
        ASSERT_EQ(tpg->getEdges().size(), 7);
        ASSERT_EQ(tpg->getRootVertices().size(), 2);
    }

    virtual void TearDown()
    {
        delete tpg;
        delete e;
        delete (&(vect.at(0).get()));
        delete (&(vect.at(1).get()));
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
    }
};

TEST_F(TPGExecutionEngineKeyedTest, TPGGraphKeyedGetSubtree)
{
    // Because it requires a TPGGraphKeyed to be built, this test is here
    // insread of in the TPGGraphKeyedTest file.

    // Get the subtree from T0
    const TPG::TPGGraphKeyed* tpgKeyed =
        dynamic_cast<const TPG::TPGGraphKeyed*>(tpg);
    auto subtree = tpgKeyed->getSubtree(*tpg->getVertices().at(0));

    // Check that the subtree contains the expected teams and edges
    ASSERT_EQ(subtree.first.size(), 2)
        << "Subtree should contain 2 teams (T0 and T2).";
    ASSERT_EQ(subtree.second.size(), 4)
        << "Subtree should contain 4 edges (T0->A0, T0->T2, T2->A2, T2->A1).";
    // Check that the subtree contains the expected teams and edges
    auto verticesToCheck = {
        dynamic_cast<const TPG::TPGTeamKeyed*>(tpg->getVertices().at(0)),
        dynamic_cast<const TPG::TPGTeamKeyed*>(tpg->getVertices().at(2))};
    for (const auto& vertex : verticesToCheck) {
        ASSERT_TRUE(subtree.first.find(vertex) != subtree.first.end())
            << "Subtree should contain team with ID: " << vertex->getVertexID();
    }
    auto edgesToCheck = {dynamic_cast<const TPG::TPGEdgeKeyed*>(edges.at(0)),
                         dynamic_cast<const TPG::TPGEdgeKeyed*>(edges.at(2)),
                         dynamic_cast<const TPG::TPGEdgeKeyed*>(edges.at(3)),
                         dynamic_cast<const TPG::TPGEdgeKeyed*>(edges.at(6))};
    for (const auto& edge : edgesToCheck) {
        ASSERT_TRUE(subtree.second.find(edge) != subtree.second.end())
            << "Subtree should contain edge with ID: " << edge->getEdgeID();
    }

    // Get subtree from T1
    subtree = tpgKeyed->getSubtree(*tpg->getVertices().at(1));

    // Check that the subtree contains the expected teams and edges
    ASSERT_EQ(subtree.first.size(), 2)
        << "Subtree should contain 2 teams (T1 and T2).";
    ASSERT_EQ(subtree.second.size(), 4)
        << "Subtree should contain 4 edges (T1->A1, T1->T2, T2->A0, T2->A1).";
    // Check that the subtree contains the expected teams and edges
    auto verticesToCheck2 = {
        dynamic_cast<const TPG::TPGTeamKeyed*>(tpg->getVertices().at(1)),
        dynamic_cast<const TPG::TPGTeamKeyed*>(tpg->getVertices().at(2))};
    for (const auto& vertex : verticesToCheck2) {
        ASSERT_TRUE(subtree.first.find(vertex) != subtree.first.end())
            << "Subtree should contain team with ID: " << vertex->getVertexID();
    }
    auto edgesToCheck2 = {dynamic_cast<const TPG::TPGEdgeKeyed*>(edges.at(1)),
                          dynamic_cast<const TPG::TPGEdgeKeyed*>(edges.at(4)),
                          dynamic_cast<const TPG::TPGEdgeKeyed*>(edges.at(5)),
                          dynamic_cast<const TPG::TPGEdgeKeyed*>(edges.at(6))};
    for (const auto& edge : edgesToCheck2) {
        ASSERT_TRUE(subtree.second.find(edge) != subtree.second.end())
            << "Subtree should contain edge with ID: " << edge->getEdgeID();
    }

    // Check the subtree from T0 again, but give key 3 at the start
    std::set<uint64_t> keys = {3};
    subtree = tpgKeyed->getSubtree(*tpg->getVertices().at(0), keys);
    // Check that the subtree contains the expected teams and edges
    ASSERT_EQ(subtree.first.size(), 2)
        << "Subtree should contain 2 teams (T0 and T2).";
    ASSERT_EQ(subtree.second.size(), 5)
        << "Subtree should contain 5 edges (T0->A0, T0->T2, T2->A2, T2->A1, "
           "T1->A0).";
}

TEST_F(TPGExecutionEngineKeyedTest, TPGExecutionEngineKeyed)
{
    TPG::TPGExecutionEngineKeyed* engine;

    ASSERT_NO_THROW(engine = new TPG::TPGExecutionEngineKeyed(*e, &a))
        << "Construction of TPGExecutionEngineKeyed with Archive should "
           "not "
           "fail.";

    ASSERT_NO_THROW(delete engine)
        << "Destruction of TPGExecutionEngineKeyed with Archive should not "
           "fail.";
}

TEST_F(TPGExecutionEngineKeyedTest, TPGExecutionEngineKeyedCollectedKeysInitial)
{
    TPG::TPGExecutionEngineKeyed engine(*e, nullptr);

    auto keys = engine.getCollectedKeys();
    ASSERT_EQ(keys.size(), 0)
        << "Collected keys should be empty after construction.";
}

TEST_F(TPGExecutionEngineKeyedTest, TPGExecutionEngineKeyedEvaluateTeamWithKey)
{
    TPG::TPGExecutionEngineKeyed engine(*e, nullptr);

    // Get first team and set its key
    const TPG::TPGTeamKeyed* team =
        dynamic_cast<const TPG::TPGTeamKeyed*>(tpg->getVertices().at(0));
    ASSERT_NE(team, nullptr);

    // Evaluate the team T0
    const TPG::TPGEdge* result;
    ASSERT_NO_THROW(result = &engine.evaluateTeam(*team))
        << "Evaluating a TPGTeamKeyed should not fail.";

    // Check that the result is as expected (edge from T0 to T2)
    ASSERT_EQ(result, edges.at(3))
        << "Edge selected during team evaluation is incorrect.";

    // Check that the key was collected
    auto keys = engine.getCollectedKeys();
    ASSERT_TRUE(keys.find(2) != keys.end())
        << "Key 2 should be in the collected keys after evaluating team.";

    // reset collected keys
    engine.clearCollectedKeys();

    // Evaluate the team T1
    const TPG::TPGTeamKeyed* team1 =
        dynamic_cast<const TPG::TPGTeamKeyed*>(tpg->getVertices().at(1));
    ASSERT_NE(team1, nullptr);

    ASSERT_NO_THROW(result = &engine.evaluateTeam(*team1))
        << "Evaluating a TPGTeamKeyed should not fail.";

    // Check that the result is as expected (edge from T1 to T2) with no
    // lock(lock=1)
    ASSERT_EQ(result, edges.at(4))
        << "Edge selected during team evaluation is incorrect.";
}

TEST_F(TPGExecutionEngineKeyedTest,
       TPGExecutionEngineKeyedEvaluateTeamMultipleKeys)
{
    TPG::TPGExecutionEngineKeyed engine(*e, nullptr);

    // Get first and second teams
    const TPG::TPGTeamKeyed* team0 =
        dynamic_cast<const TPG::TPGTeamKeyed*>(tpg->getVertices().at(0));
    const TPG::TPGTeamKeyed* team1 =
        dynamic_cast<const TPG::TPGTeamKeyed*>(tpg->getVertices().at(1));

    ASSERT_NE(team0, nullptr);
    ASSERT_NE(team1, nullptr);

    // Evaluate both teams
    ASSERT_NO_THROW(engine.evaluateTeam(*team0));
    ASSERT_NO_THROW(engine.evaluateTeam(*team1));

    // Check that both keys were collected
    auto keys = engine.getCollectedKeys();
    ASSERT_EQ(keys.size(), 2) << "Should have collected 2 different keys.";
    ASSERT_TRUE(keys.find(2) != keys.end());
    ASSERT_TRUE(keys.find(3) != keys.end());

    // Evaluate team T2 which has no special key
    // All tree output TPGEdges should be accessible since T2 has no key and
    // the locks on edges are 2, 3, and 6
    const TPG::TPGTeamKeyed* team2 =
        dynamic_cast<const TPG::TPGTeamKeyed*>(tpg->getVertices().at(2));

    const TPG::TPGEdge* result;

    // Activate path to A2 by setting the program return values accordingly
    makeProgramReturn(*progPointers.at(2), 1); // T2->A2
    makeProgramReturn(*progPointers.at(5), 0); // T2->A0
    makeProgramReturn(*progPointers.at(6), 0); // T2->A1
    ASSERT_NO_THROW(result = &engine.evaluateTeam(*team2))
        << "Evaluating a TPGTeamKeyed should not fail.";

    // Check that the result is as expected (edge from T2 to A2)
    ASSERT_EQ(result, edges.at(2))
        << "Edge selected during team evaluation is incorrect.";

    // Set path to A0 by changing the program return values
    makeProgramReturn(*progPointers.at(2), 0); // T2->A2
    makeProgramReturn(*progPointers.at(5), 1); // T2->A0
    makeProgramReturn(*progPointers.at(6), 0); // T2->A1

    ASSERT_NO_THROW(result = &engine.evaluateTeam(*team2))
        << "Evaluating a TPGTeamKeyed should not fail.";

    // Check that the result is as expected (edge from T2 to A0)
    ASSERT_EQ(result, edges.at(5))
        << "Edge selected during team evaluation is incorrect.";

    // Set path to A1 by changing the program return values
    makeProgramReturn(*progPointers.at(2), 0); // T2->A2
    makeProgramReturn(*progPointers.at(5), 0); // T2->A0
    makeProgramReturn(*progPointers.at(6), 1); // T2->A1

    ASSERT_NO_THROW(result = &engine.evaluateTeam(*team2))
        << "Evaluating a TPGTeamKeyed should not fail.";

    // Check that the result is as expected (edge from T2 to A1)
    ASSERT_EQ(result, edges.at(6))
        << "Edge selected during team evaluation is incorrect.";

    // Clear collected keys and check that they are empty
    engine.clearCollectedKeys();

    // Collect only key 2 and evaluate team T2 again
    ASSERT_NO_THROW(engine.evaluateTeam(*team0));

    // Set path to A1 by changing the program return values
    makeProgramReturn(*progPointers.at(2), 0);  // T2->A2
    makeProgramReturn(*progPointers.at(5), 10); // T2->A0
    makeProgramReturn(*progPointers.at(6), 1);  // T2->A1

    ASSERT_NO_THROW(result = &engine.evaluateTeam(*team2))
        << "Evaluating a TPGTeamKeyed should not fail.";

    // Check that the result is as expected (edge from T2 to A1) since
    // lock=2 and key=2, so edge should be accessible.
    ASSERT_EQ(result, edges.at(6))
        << "Edge selected during team evaluation is incorrect.";
}

TEST_F(TPGExecutionEngineKeyedTest, TPGExecutionEngineKeyedExecuteFromRoot)
{
    TPG::TPGExecutionEngineKeyed engine(*e, nullptr);

    // Get root team and set its key
    const TPG::TPGTeamKeyed* rootTeam =
        dynamic_cast<const TPG::TPGTeamKeyed*>(tpg->getRootVertices().at(0));

    // Execute from root
    std::vector<const TPG::TPGVertex*> result;
    ASSERT_NO_THROW(result = engine.executeFromRoot(*rootTeam).first);

    // Check that the result is a non-empty path
    ASSERT_EQ(result.size(), 3) << "Execution path should not be empty.";

    // Path is T0-T2-A1
    ASSERT_EQ(result.at(0), rootTeam)
        << "First vertex in path should be root team.";
    ASSERT_EQ(result.at(1), tpg->getVertices().at(2))
        << "Second vertex in path should be T2.";
    ASSERT_EQ(result.at(2), tpg->getVertices().at(4))
        << "Third vertex in path should be A1.";

    // Now do the same from T1 root team
    rootTeam =
        dynamic_cast<const TPG::TPGTeamKeyed*>(tpg->getRootVertices().at(1));

    // Execute from root
    ASSERT_NO_THROW(result = engine.executeFromRoot(*rootTeam).first);
    ASSERT_EQ(result.size(), 3) << "Execution path should not be empty.";

    // Path is T1-T2-A0
    ASSERT_EQ(result.at(0), rootTeam)
        << "First vertex in path should be root team.";
    ASSERT_EQ(result.at(1), tpg->getVertices().at(2))
        << "Second vertex in path should be T2.";
    ASSERT_EQ(result.at(2), tpg->getVertices().at(3))
        << "Third vertex in path should be A0.";
}

TEST_F(TPGExecutionEngineKeyedTest,
       TPGExecutionEngineKeyedEvaluateTeamWithoutKey)
{
    TPG::TPGExecutionEngineKeyed engine(*e, nullptr);

    // Get a team that is not a TPGTeamKeyed (should fail)
    TPG::TPGTeam regularTeam;

    ASSERT_THROW(engine.evaluateTeam(regularTeam), std::runtime_error)
        << "Evaluating a non-keyed team should throw an exception.";
}

TEST_F(TPGExecutionEngineKeyedTest,
       TPGExecutionEngineKeyedCollectedKeysMultipleEvaluations)
{
    TPG::TPGExecutionEngineKeyed engine(*e, nullptr);

    // Get multiple teams
    const TPG::TPGTeamKeyed* team0 =
        dynamic_cast<const TPG::TPGTeamKeyed*>(tpg->getVertices().at(0));

    ASSERT_NE(team0, nullptr);

    // Evaluate team 0 multiple times
    ASSERT_NO_THROW(engine.evaluateTeam(*team0));
    ASSERT_NO_THROW(engine.evaluateTeam(*team0));

    auto keys = engine.getCollectedKeys();
    ASSERT_EQ(keys.size(), 1) << "Evaluating the same key twice should not "
                                 "duplicate it in the set.";
}

TEST_F(TPGExecutionEngineKeyedTest,
       TPGExecutionEngineKeyedEmptyEvaluateTeamEmptyFilteredEdgeSet)
{
    TPG::TPGExecutionEngineKeyed engine(*e, nullptr);
    // Get a team that has no outgoing edges (create a new one)
    auto& emptyTeam = tpg->addNewTeam();
    dynamic_cast<TPG::TPGGraphKeyed*>(tpg)->setNewTeamKey(
        dynamic_cast<const TPG::TPGTeamKeyed&>(emptyTeam), 5);
    // Evaluate the empty team to collect its key
    ASSERT_THROW(engine.evaluateTeam(emptyTeam), std::runtime_error)
        << "Evaluating a team with no outgoing edges should throw an "
           "exception.";

    // Check that key 5 was collected (nevertheless)
    auto keys = engine.getCollectedKeys();
    ASSERT_TRUE(keys.find(5) != keys.end())
        << "Key 5 should be in the collected keys after evaluating empty "
           "team.";

    // Evaluate T2 which has outgoing edges but none of them should be
    // accessible with key 5
    const TPG::TPGTeamKeyed* team2 =
        dynamic_cast<const TPG::TPGTeamKeyed*>(tpg->getVertices().at(2));

    ASSERT_THROW(engine.evaluateTeam(*team2), std::runtime_error)
        << "Evaluating a team with no accessible outgoing edges should "
           "throw "
           "an exception.";
}