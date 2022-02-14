
#include <gtest/gtest.h>

#include "data/dataHandler.h"
#include "environment.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/lambdaInstruction.h"
#include "instructions/set.h"
#include "program/program.h"

#include "tpg/instrumented/tpgActionInstrumented.h"
#include "tpg/instrumented/tpgEdgeInstrumented.h"
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

    ASSERT_EQ(edge.getNbTraversed(), 0)
        << "Number of traversal on a newly constructed TPGEdgeInstrumented "
           "should "
           "be 0.";

    ASSERT_NO_THROW(edge.incrementNbVisits())
        << "Increment of number of visits on a TPGEdgeInstrumented should not "
           "fail.";

    ASSERT_EQ(edge.getNbVisits(), 1)
        << "Number of visit of a TPGEdgeInstrumented should "
           "be 1 after an increment.";

    ASSERT_NO_THROW(edge.incrementNbTraversed())
        << "Increment of number of traversal on a TPGEdgeInstrumented should "
           "not "
           "fail.";

    ASSERT_EQ(edge.getNbTraversed(), 1)
        << "Number of traversal of a TPGEdgeInstrumented should "
           "be 1 after an increment.";

    ASSERT_NO_THROW(edge.reset()) << "Reset of instrumentation counter should "
                                     "not fail on a TPGEdgeInstrumented.";

    ASSERT_EQ(edge.getNbVisits(), 0)
        << "Number of visit of a TPGEdgeInstrumented should "
           "be 0 after a reset.";

    ASSERT_EQ(edge.getNbTraversed(), 0)
        << "Number of traversal of a TPGEdgeInstrumented should "
           "be 0 after a reset.";
}

TEST_F(TPGInstrumentedTest, TPGInstrumentedFactory)
{
    TPG::TPGInstrumentedFactory factory;

    TPG::TPGAction* action;
    TPG::TPGTeam* team;
    std::unique_ptr<TPG::TPGEdge> edge;

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