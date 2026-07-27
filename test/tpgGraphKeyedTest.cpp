#include <gtest/gtest.h>

#include "data/dataHandler.h"
#include "environment.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/lambdaInstruction.h"
#include "instructions/set.h"
#include "program/program.h"

#include "tpg/keyed/tpgEdgeKeyed.h"
#include "tpg/keyed/tpgExecutionEngineKeyed.h"
#include "tpg/keyed/tpgGraphKeyed.h"
#include "tpg/keyed/tpgKeyedFactory.h"
#include "tpg/keyed/tpgTeamKeyed.h"
#include "tpg/tpgGraph.h"
#include "util/counterReset.h"

class TPGGraphKeyedTest : public ::testing::Test
{
  protected:
    const size_t size1{24};
    const size_t size2{32};
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
    Instructions::Set set;
    Environment* e = NULL;
    Learn::LearningParameters params;
    std::shared_ptr<Program::Program> progPointer;

    virtual void SetUp()
    {
        CounterReset::counterReset();
        vect.push_back(
            *(new Data::PrimitiveTypeArray<double>((unsigned int)size1)));
        vect.push_back(
            *(new Data::PrimitiveTypeArray<float>((unsigned int)size2)));

        set.add(*(new Instructions::AddPrimitiveType<float>()));
        auto minus = [](double a, double b) -> double { return a - b; };
        set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));

        params.nbRegisters = 8;
        params.nbProgramConstant = 1;
        e = new Environment(set, params, vect);
        progPointer =
            std::shared_ptr<Program::Program>(new Program::Program(*e, false));
    }

    virtual void TearDown()
    {
        delete e;
        delete (&(vect.at(0).get()));
        delete (&(vect.at(1).get()));
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
    }
};

TEST_F(TPGGraphKeyedTest, TPGTeamKeyedConstructorsDestructors)
{
    TPG::TPGVertex* team;

    ASSERT_NO_THROW(team = new TPG::TPGTeamKeyed())
        << "Construction of TPGTeamKeyed should not fail.";

    ASSERT_NO_THROW(delete team)
        << "Destruction of TPGTeamKeyed should not fail.";
}

TEST_F(TPGGraphKeyedTest, TPGTeamKeyedSettersAndGetters)
{
    TPG::TPGTeamKeyed team;

    // Test default key
    ASSERT_EQ(team.getKey(), 1)
        << "Default key of a newly constructed TPGTeamKeyed should be 1.";

    // Test setting a prime key
    ASSERT_NO_THROW(team.setKey(2))
        << "Setting key to prime number 2 on a TPGTeamKeyed should not fail.";

    ASSERT_EQ(team.getKey(), 2)
        << "Key of TPGTeamKeyed should be 2 after setKey(2).";

    // Test setting another prime key
    ASSERT_NO_THROW(team.setKey(3))
        << "Setting key to prime number 3 on a TPGTeamKeyed should not fail.";

    ASSERT_EQ(team.getKey(), 3)
        << "Key of TPGTeamKeyed should be 3 after setKey(3).";
}

TEST_F(TPGGraphKeyedTest, TPGEdgeKeyedConstructorsDestructors)
{
    TPG::TPGTeam team;
    TPG::TPGAction action(1);
    TPG::TPGEdge* edge;

    ASSERT_NO_THROW(edge = new TPG::TPGEdgeKeyed(&team, &action, progPointer))
        << "Construction of TPGEdgeKeyed should not fail.";

    ASSERT_NO_THROW(delete edge)
        << "Destruction of TPGEdgeKeyed should not fail.";
}

TEST_F(TPGGraphKeyedTest, TPGEdgeKeyedSettersAndGetters)
{
    TPG::TPGTeam team;
    TPG::TPGAction action(1);
    TPG::TPGEdgeKeyed edge(&team, &action, progPointer);

    // Test default lock value
    ASSERT_EQ(edge.getLock(), 1)
        << "Default lock of a newly constructed TPGEdgeKeyed should be 1.";

    // Test setting a new lock value
    ASSERT_NO_THROW(edge.setLock(6))
        << "Setting lock to 6 on a TPGEdgeKeyed should not fail.";

    ASSERT_EQ(edge.getLock(), 6)
        << "Lock of TPGEdgeKeyed should be 6 after setLock(6).";
}

TEST_F(TPGGraphKeyedTest, TPGEdgeKeyedIsUnlockedByKey)
{
    TPG::TPGTeam team;
    TPG::TPGAction action(1);
    TPG::TPGEdgeKeyed edge(&team, &action, progPointer);
    // Set lock to 6 (2 * 3)
    edge.setLock(6);
    // Test keys that should unlock the edge
    ASSERT_TRUE(edge.isUnlockedByKey(2))
        << "Key 2 should unlock the edge with lock 6.";
    ASSERT_TRUE(edge.isUnlockedByKey(3))
        << "Key 3 should unlock the edge with lock 6.";
    ASSERT_TRUE(edge.isUnlockedByKey(6))
        << "Key 6 should unlock the edge with lock 6.";
    // Test keys that should not unlock the edge
    ASSERT_FALSE(edge.isUnlockedByKey(1))
        << "Key 1 should not unlock the edge with lock 6.";
    ASSERT_FALSE(edge.isUnlockedByKey(5))
        << "Key 5 should not unlock the edge with lock 6.";

    // Create an edge with default lock (1) and test that any key unlocks it
    TPG::TPGEdgeKeyed edgeDefaultLock(&team, &action, progPointer);
    ASSERT_TRUE(edgeDefaultLock.isUnlockedByKey(2))
        << "Key 2 should unlock the edge with default lock 1.";
    ASSERT_TRUE(edgeDefaultLock.isUnlockedByKey(1))
        << "Key 1 should unlock the edge with default lock 1.";
}

TEST_F(TPGGraphKeyedTest, TPGKeyedFactoryConstructorsDestructors)
{
    TPG::TPGKeyedFactory* factory;

    ASSERT_NO_THROW(factory = new TPG::TPGKeyedFactory())
        << "Construction of TPGKeyedFactory should not fail.";

    ASSERT_NO_THROW(delete factory)
        << "Destruction of TPGKeyedFactory should not fail.";
}

TEST_F(TPGGraphKeyedTest, TPGKeyedFactoryCreateTeam)
{
    TPG::TPGKeyedFactory factory;

    std::unique_ptr<TPG::TPGTeam> team;

    ASSERT_NO_THROW(team = factory.createTPGTeam())
        << "TPGKeyedFactory could not build a TPGTeam.";
    ASSERT_NE(team, nullptr) << "Created TPGTeam should not be null.";
    ASSERT_EQ(typeid(*team), typeid(TPG::TPGTeamKeyed))
        << "Team built by the TPGKeyedFactory has an incorrect type.";
}

TEST_F(TPGGraphKeyedTest, TPGKeyedFactoryCreateEdge)
{
    TPG::TPGKeyedFactory factory;

    std::unique_ptr<TPG::TPGTeam> team;
    std::unique_ptr<TPG::TPGAction> action;
    std::unique_ptr<TPG::TPGEdge> edge;

    ASSERT_NO_THROW(team = factory.createTPGTeam());
    ASSERT_NO_THROW(action = factory.createTPGAction(0));

    ASSERT_NO_THROW(
        edge = factory.createTPGEdge(team.get(), action.get(), progPointer))
        << "TPGKeyedFactory could not build a TPGEdge.";
    ASSERT_NE(edge.get(), nullptr) << "Created TPGEdge should not be null.";
    ASSERT_EQ(typeid(*edge), typeid(TPG::TPGEdgeKeyed))
        << "Edge built by the TPGKeyedFactory has an incorrect type.";
}

TEST_F(TPGGraphKeyedTest, TPGKeyedFactoryCreateExecutionEngine)
{
    TPG::TPGKeyedFactory factory;

    std::unique_ptr<TPG::TPGExecutionEngine> tee;

    ASSERT_NO_THROW(tee = factory.createTPGExecutionEngine(*e, nullptr))
        << "TPGKeyedFactory could not build a TPGExecutionEngine.";
    ASSERT_NE(tee.get(), nullptr)
        << "Created TPGExecutionEngine should not be null.";
    ASSERT_EQ(typeid(*tee), typeid(TPG::TPGExecutionEngineKeyed))
        << "ExecutionEngine built by the TPGKeyedFactory has an incorrect "
           "type.";
}

TEST_F(TPGGraphKeyedTest, TPGKeyedFactoryCreateTPGGraph)
{
    TPG::TPGKeyedFactory factory;

    std::shared_ptr<TPG::TPGGraph> tpg;

    ASSERT_NO_THROW(tpg = factory.createTPGGraph(*e))
        << "TPGKeyedFactory could not build a TPGGraph.";
    ASSERT_NE(tpg, nullptr) << "Created TPGGraph should not be null.";
}

TEST_F(TPGGraphKeyedTest, TPGGraphAddTPGKeyedVertexAndEdge)
{
    TPG::TPGGraph tpg(*e, std::make_unique<TPG::TPGKeyedFactory>());
    const TPG::TPGTeam* t;
    const TPG::TPGAction* a;
    const TPG::TPGEdge* edge;

    ASSERT_NO_THROW(t = &tpg.addNewTeam())
        << "Adding a new Team to a TPGGraph failed.";
    ASSERT_EQ(typeid(*t), typeid(TPG::TPGTeamKeyed))
        << "Team built by the TPGKeyedFactory has an incorrect type.";

    ASSERT_NO_THROW(a = &tpg.addNewAction(0))
        << "Adding a new Action to a TPGGraph failed.";

    ASSERT_NO_THROW(edge = &tpg.addNewEdge(*t, *a, progPointer));
    ASSERT_EQ(typeid(*edge), typeid(TPG::TPGEdgeKeyed))
        << "Edge built by the TPGKeyedFactory has an incorrect type.";
}

TEST_F(TPGGraphKeyedTest, TPGKeyedFactoryIntegration)
{
    // Create a graph with keyed factory
    TPG::TPGGraph tpg(*e, std::make_unique<TPG::TPGKeyedFactory>());

    // Add vertices
    const TPG::TPGTeamKeyed* t1 =
        dynamic_cast<const TPG::TPGTeamKeyed*>(&tpg.addNewTeam());
    const TPG::TPGTeamKeyed* t2 =
        dynamic_cast<const TPG::TPGTeamKeyed*>(&tpg.addNewTeam());
    const TPG::TPGAction* a =
        dynamic_cast<const TPG::TPGAction*>(&tpg.addNewAction(0));

    ASSERT_NE(t1, nullptr);
    ASSERT_NE(t2, nullptr);
    ASSERT_NE(a, nullptr);

    // Add edges
    const TPG::TPGEdgeKeyed* e1 = dynamic_cast<const TPG::TPGEdgeKeyed*>(
        &tpg.addNewEdge(*t1, *t2, progPointer));
    const TPG::TPGEdgeKeyed* e2 = dynamic_cast<const TPG::TPGEdgeKeyed*>(
        &tpg.addNewEdge(*t2, *a, progPointer));

    ASSERT_NE(e1, nullptr);
    ASSERT_NE(e2, nullptr);

    // Verify edges have correct default locks
    ASSERT_EQ(e1->getLock(), 1);
    ASSERT_EQ(e2->getLock(), 1);

    // Verify teams have correct default keys
    ASSERT_EQ(t1->getKey(), 1);
    ASSERT_EQ(t2->getKey(), 1);

    // Verify graph structure
    ASSERT_EQ(tpg.getNbVertices(), 3)
        << "Graph should have 3 vertices (2 teams + 1 action).";
    ASSERT_EQ(tpg.getEdges().size(), 2) << "Graph should have 2 edges.";
}

TEST_F(TPGGraphKeyedTest, TPGGraphKeyedConstructor)
{
    TPG::TPGGraphKeyed* graphKeyed;

    ASSERT_NO_THROW(graphKeyed = new TPG::TPGGraphKeyed(*e))
        << "Construction of TPGGraphKeyed with default factory should not "
           "fail.";

    // Check used factory type
    ASSERT_EQ(typeid(graphKeyed->getFactory()), typeid(TPG::TPGKeyedFactory))
        << "TPGGraph Keyed should use TPGKeyedFactory by default.";

    ASSERT_NO_THROW(delete graphKeyed)
        << "Destruction of TPGGraphKeyed should not fail.";
}

TEST_F(TPGGraphKeyedTest, TPGGraphKeyedSetNewTeamKey)
{
    TPG::TPGGraphKeyed graphKeyed(*e);

    // Add a team
    const TPG::TPGTeamKeyed* team =
        dynamic_cast<const TPG::TPGTeamKeyed*>(&graphKeyed.addNewTeam());

    ASSERT_NE(team, nullptr) << "Added team should be TPGTeamKeyed.";
    ASSERT_EQ(team->getKey(), 1) << "Default key should be 1.";

    // Test setting a new key
    ASSERT_NO_THROW(graphKeyed.setNewTeamKey(*team, 2))
        << "Setting team key to 2 should not fail.";

    ASSERT_EQ(team->getKey(), 2)
        << "Team key should be 2 after setNewTeamKey(team, 2).";

    // Test setting key on a non-existent team (should throw)
    TPG::TPGTeamKeyed fakeTeam;
    ASSERT_THROW(graphKeyed.setNewTeamKey(fakeTeam, 3), std::runtime_error)
        << "Setting key on a non-existent team should throw an exception.";
}

TEST_F(TPGGraphKeyedTest, TPGGraphKeyedSetNewEdgeLock)
{
    TPG::TPGGraphKeyed graphKeyed(*e);

    // Add vertices
    const TPG::TPGTeamKeyed* team =
        dynamic_cast<const TPG::TPGTeamKeyed*>(&graphKeyed.addNewTeam());
    const TPG::TPGAction* action =
        dynamic_cast<const TPG::TPGAction*>(&graphKeyed.addNewAction(0));

    ASSERT_NE(team, nullptr);
    ASSERT_NE(action, nullptr);

    // Add an edge
    const TPG::TPGEdgeKeyed* edge = dynamic_cast<const TPG::TPGEdgeKeyed*>(
        &graphKeyed.addNewEdge(*team, *action, progPointer));

    ASSERT_NE(edge, nullptr) << "Added edge should be TPGEdgeKeyed.";
    ASSERT_EQ(edge->getLock(), 1) << "Default lock should be 1.";

    // Test setting a new lock
    ASSERT_NO_THROW(graphKeyed.setNewEdgeLock(*edge, 3))
        << "Setting edge lock to 3 should not fail.";

    ASSERT_EQ(edge->getLock(), 3)
        << "Edge lock should be 3 after setNewEdgeLock(edge, 3).";

    // Test setting lock on a non-existent edge (should throw)
    TPG::TPGEdgeKeyed fakeEdge(team, action, progPointer);
    ASSERT_THROW(graphKeyed.setNewEdgeLock(fakeEdge, 4), std::runtime_error)
        << "Setting lock on a non-existent edge should throw an exception.";
}
