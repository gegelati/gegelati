
#include <gtest/gtest.h>

#include "data/dataHandler.h"
#include "environment.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/lambdaInstruction.h"
#include "instructions/set.h"
#include "program/program.h"

#include "tpg/instrumented/tpgActionInstrumented.h"
#include "tpg/instrumented/tpgEdgeInstrumented.h"
#include "tpg/instrumented/tpgExecutionEngineInstrumented.h"
#include "tpg/instrumented/tpgInstrumentedFactory.h"
#include "tpg/instrumented/tpgTeamInstrumented.h"
#include "tpg/tpgGraph.h"

class TPGInstrumentedTest : public ::testing::Test
{
  protected:
    const size_t size1{24};
    const size_t size2{32};
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
    Instructions::Set set;
    Environment* e = NULL;
    std::shared_ptr<Program::Program> progPointer;

    virtual void SetUp()
    {
        vect.push_back(
            *(new Data::PrimitiveTypeArray<double>((unsigned int)size1)));
        vect.push_back(
            *(new Data::PrimitiveTypeArray<float>((unsigned int)size2)));

        set.add(*(new Instructions::AddPrimitiveType<float>()));
        auto minus = [](double a, double b) -> double { return a - b; };
        set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));

        e = new Environment(set, vect, 8, 5);
        progPointer =
            std::shared_ptr<Program::Program>(new Program::Program(*e));
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

TEST_F(TPGInstrumentedTest,
       TPGTeamInstrumentedAndTPGActionInstrumentedConstructorsDestructors)
{
    TPG::TPGVertex* team;
    TPG::TPGVertex* action;

    ASSERT_NO_THROW(team = new TPG::TPGTeamInstrumented());
    ASSERT_NO_THROW(action = new TPG::TPGActionInstrumented(0));

    ASSERT_NO_THROW(delete team);
    ASSERT_NO_THROW(delete action);
}

TEST_F(TPGInstrumentedTest, TPGVertexInstrumentationSettersAndGetters)
{
    // Test TPGVertexInstrumentation through its TPGTeamInstrumented
    // specialization.
    TPG::TPGTeamInstrumented team;

    ASSERT_EQ(team.getNbVisits(), 0)
        << "Number of visit on a newly constructed TPGTeamInstrumented should "
           "be 0.";

    ASSERT_NO_THROW(team.incrementNbVisits())
        << "Increment of number of visits on a TPGTeamInstrumented should not "
           "fail.";

    ASSERT_EQ(team.getNbVisits(), 1)
        << "Number of visit of a TPGTeamInstrumented should "
           "be 1 after an increment.";

    ASSERT_NO_THROW(team.reset()) << "Reset of instrumentation counter should "
                                     "not fail on a TPGTeamInstrumented.";

    ASSERT_EQ(team.getNbVisits(), 0)
        << "Number of visit of a TPGTeamInstrumented should "
           "be 0 after a reset.";
}

TEST_F(TPGInstrumentedTest, TPGEdgeInstrumentedConstructorsDestructors)
{
    TPG::TPGTeam team;
    TPG::TPGAction action(1);
    TPG::TPGEdge* edge;

    ASSERT_NO_THROW(
        edge = new TPG::TPGEdgeInstrumented(&team, &action, progPointer));

    ASSERT_NO_THROW(delete edge);
}

TEST_F(TPGInstrumentedTest, TPGEdgeInstrumentedSettersAndGetters)
{
    TPG::TPGTeam team;
    TPG::TPGAction action(1);
    TPG::TPGEdgeInstrumented edge(&team, &action, progPointer);

    ASSERT_EQ(edge.getNbVisits(), 0)
        << "Number of visit on a newly constructed TPGEdgeInstrumented should "
           "be 0.";

    ASSERT_EQ(edge.getNbTraversal(), 0)
        << "Number of traversal on a newly constructed TPGEdgeInstrumented "
           "should "
           "be 0.";

    ASSERT_NO_THROW(edge.incrementNbVisits())
        << "Increment of number of visits on a TPGEdgeInstrumented should not "
           "fail.";

    ASSERT_EQ(edge.getNbVisits(), 1)
        << "Number of visit of a TPGEdgeInstrumented should "
           "be 1 after an increment.";

    ASSERT_NO_THROW(edge.incrementNbTraversal())
        << "Increment of number of traversal on a TPGEdgeInstrumented should "
           "not "
           "fail.";

    ASSERT_EQ(edge.getNbTraversal(), 1)
        << "Number of traversal of a TPGEdgeInstrumented should "
           "be 1 after an increment.";

    ASSERT_NO_THROW(edge.reset()) << "Reset of instrumentation counter should "
                                     "not fail on a TPGEdgeInstrumented.";

    ASSERT_EQ(edge.getNbVisits(), 0)
        << "Number of visit of a TPGEdgeInstrumented should "
           "be 0 after a reset.";

    ASSERT_EQ(edge.getNbTraversal(), 0)
        << "Number of traversal of a TPGEdgeInstrumented should "
           "be 0 after a reset.";
}

TEST_F(TPGInstrumentedTest, TPGInstrumentedFactory)
{
    TPG::TPGInstrumentedFactory factory;

    TPG::TPGAction* action;
    TPG::TPGTeam* team;
    std::unique_ptr<TPG::TPGEdge> edge;
    std::unique_ptr<TPG::TPGExecutionEngine> tee;

    ASSERT_NO_THROW(action = factory.createTPGAction(0))
        << "TPGFactory could not build a TPGAction.";
    ASSERT_NE(action, nullptr) << "Created TPGAction should not be null.";
    ASSERT_EQ(typeid(*action), typeid(TPG::TPGActionInstrumented))
        << "Action built by the TPGInstrumentedFactory has an incorrect type.";

    ASSERT_NO_THROW(team = factory.createTPGTeam())
        << "TPGGraphELementFactory could not build a TPGAction.";
    ASSERT_NE(team, nullptr) << "Created TPGTeam should not be null.";
    ASSERT_EQ(typeid(*team), typeid(TPG::TPGTeamInstrumented))
        << "Team built by the TPGInstrumentedFactory has an incorrect type.";

    ASSERT_NO_THROW(edge = factory.createTPGEdge(team, action, progPointer))
        << "TPGGraphELementFactory could not build a TPGAction.";
    ASSERT_NE(edge.get(), nullptr) << "Created TPGEdge should not be null.";
    ASSERT_EQ(typeid(*edge), typeid(TPG::TPGEdgeInstrumented))
        << "Edge built by the TPGInstrumentedFactory has an incorrect type.";

    ASSERT_NO_THROW(tee = factory.createTPGExecutionEngine(*e, nullptr))
        << "TPGGraphELementFactory could not build a TPGExecutionEngine.";
    ASSERT_NE(tee.get(), nullptr) << "Created TPGEdge should not be null.";
    ASSERT_EQ(typeid(*tee), typeid(TPG::TPGExecutionEngineInstrumented))
        << "Edge built by the TPGInstrumentedFactory has an incorrect type.";

    delete team;
    delete action;
}

TEST_F(TPGInstrumentedTest, TPGGraphAddTPGVertexAndEdge)
{
    TPG::TPGGraph tpg(*e, std::make_unique<TPG::TPGInstrumentedFactory>());
    const TPG::TPGTeam* t;
    const TPG::TPGAction* a;
    const TPG::TPGEdge* e;

    ASSERT_NO_THROW(t = &tpg.addNewTeam())
        << "Adding a new Team to a TPGGraph failed.";
    ASSERT_EQ(typeid(*t), typeid(TPG::TPGTeamInstrumented))
        << "Team built by the TPGInstrumentedFactory has an incorrect type.";

    ASSERT_NO_THROW(a = &tpg.addNewAction(0))
        << "Adding a new Action to a TPGGraph failed.";
    ASSERT_EQ(typeid(*a), typeid(TPG::TPGActionInstrumented))
        << "Action built by the TPGInstrumentedFactory has an incorrect type.";

    ASSERT_NO_THROW(e = &tpg.addNewEdge(*t, *a, progPointer));
    ASSERT_EQ(typeid(*e), typeid(TPG::TPGEdgeInstrumented))
        << "Edge built by the TPGInstrumentedFactory has an incorrect type.";
}

TEST_F(TPGInstrumentedTest, TPGInstrumentedFactoryReset)
{
    // Add to the TPG
    TPG::TPGGraph tpg(*e, std::make_unique<TPG::TPGInstrumentedFactory>());
    const TPG::TPGTeamInstrumented& t =
        dynamic_cast<const TPG::TPGTeamInstrumented&>(tpg.addNewTeam());
    const TPG::TPGActionInstrumented& a =
        dynamic_cast<const TPG::TPGActionInstrumented&>(tpg.addNewAction(0));
    const TPG::TPGEdgeInstrumented& e =
        dynamic_cast<const TPG::TPGEdgeInstrumented&>(
            tpg.addNewEdge(t, a, progPointer));

    // Increment counters
    t.incrementNbVisits();
    a.incrementNbVisits();
    e.incrementNbVisits();
    e.incrementNbTraversal();

    // Check increment
    ASSERT_EQ(t.getNbVisits(), 1);
    ASSERT_EQ(a.getNbVisits(), 1);
    ASSERT_EQ(e.getNbVisits(), 1);
    ASSERT_EQ(e.getNbTraversal(), 1);

    // Do the reset
    ASSERT_NO_THROW(
        dynamic_cast<const TPG::TPGInstrumentedFactory&>(tpg.getFactory())
            .resetTPGGraphCounters(tpg));

    // Check result
    ASSERT_EQ(t.getNbVisits(), 0);
    ASSERT_EQ(a.getNbVisits(), 0);
    ASSERT_EQ(e.getNbVisits(), 0);
    ASSERT_EQ(e.getNbTraversal(), 0);
}

TEST_F(TPGInstrumentedTest, TPGInstrumentedFactoryCleanTPG)
{
    // Add to the TPG

    /**
     *  Numbers next to edges teams represent the number of visits/traversal.
     *        T0:1-------.
     *       /   \        \
     *      /:1   \:1     /:0
     *     T1:1   A0:1---'
     *    /    \
     *   /:1    \:0
     *  T2:1     T3:0
     *  |        |
     *  |:1      |:0
     * A1:1      A2:0
     *
     */

    TPG::TPGGraph tpg(*e, std::make_unique<TPG::TPGInstrumentedFactory>());
    const TPG::TPGTeamInstrumented* t[4];
    for (auto i = 0; i < 4; i++) {
        t[i] = dynamic_cast<const TPG::TPGTeamInstrumented*>(&tpg.addNewTeam());
    }

    const TPG::TPGActionInstrumented* a[3];
    for (auto i = 0; i < 3; i++) {
        a[i] = dynamic_cast<const TPG::TPGActionInstrumented*>(
            &tpg.addNewAction(i));
    }

    const TPG::TPGEdgeInstrumented* e[7];
    // T0->T1
    e[0] = dynamic_cast<const TPG::TPGEdgeInstrumented*>(
        &tpg.addNewEdge(*t[0], *t[1], progPointer));
    t[0]->incrementNbVisits();
    e[0]->incrementNbVisits();
    e[0]->incrementNbTraversal();
    // T0->A0 (traversed)
    e[1] = dynamic_cast<const TPG::TPGEdgeInstrumented*>(
        &tpg.addNewEdge(*t[0], *a[0], progPointer));
    e[1]->incrementNbVisits();
    e[1]->incrementNbTraversal();
    a[0]->incrementNbVisits();
    // T1->T2
    e[2] = dynamic_cast<const TPG::TPGEdgeInstrumented*>(
        &tpg.addNewEdge(*t[1], *t[2], progPointer));
    t[1]->incrementNbVisits();
    e[2]->incrementNbVisits();
    e[2]->incrementNbTraversal();
    // T2->A1
    e[3] = dynamic_cast<const TPG::TPGEdgeInstrumented*>(
        &tpg.addNewEdge(*t[2], *a[1], progPointer));
    t[2]->incrementNbVisits();
    e[3]->incrementNbVisits();
    e[3]->incrementNbTraversal();
    a[1]->incrementNbVisits();
    // T1->T3
    e[4] = dynamic_cast<const TPG::TPGEdgeInstrumented*>(
        &tpg.addNewEdge(*t[1], *t[3], progPointer));
    e[4]->incrementNbVisits();
    // T3->A2
    e[5] = dynamic_cast<const TPG::TPGEdgeInstrumented*>(
        &tpg.addNewEdge(*t[3], *a[2], progPointer));
    // T0->A0 (traversed)
    e[6] = dynamic_cast<const TPG::TPGEdgeInstrumented*>(
        &tpg.addNewEdge(*t[0], *a[0], progPointer));
    e[1]->incrementNbVisits();

    ASSERT_EQ(tpg.getNbVertices(), 7)
        << "Number of vertices of the TPGGraph before being cleaned is not as "
           "expected.";

    ASSERT_EQ(tpg.getEdges().size(), 7)
        << "Number of edges of the TPGGraph before being cleaned is not as "
           "expected.";

    ASSERT_NO_THROW(
        dynamic_cast<const TPG::TPGInstrumentedFactory&>(tpg.getFactory())
            .clearUnusedTPGGraphElements(tpg));

    ASSERT_EQ(tpg.getNbVertices(), 5)
        << "Number of vertices of the TPGGraph after being cleaned is not as "
           "expected.";

    ASSERT_EQ(tpg.getEdges().size(), 4)
        << "Number of edges of the TPGGraph after being cleaned is not as "
           "expected.";
}