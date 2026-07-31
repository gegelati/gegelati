/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2022 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2022)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
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

#include "data/dataHandler.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/lambdaInstruction.h"
#include "instructions/set.h"

#include "evoGraph/instrumented/actionInstrumented.h"
#include "evoGraph/instrumented/edgeInstrumented.h"
#include "evoGraph/instrumented/factoryInstrumented.h"
#include "evoGraph/instrumented/teamInstrumented.h"
#include "evoGraph/graph.h"
#include "util/counterReset.h"

#if 0

class TPGInstrumentedTest : public ::testing::Test
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

        params.representation.lgp.nbRegisters = 8;
        params.representation.lgp.nbProgramConstant = 1;
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

TEST_F(TPGInstrumentedTest,
       TeamInstrumentedAndActionInstrumentedConstructorsDestructors)
{
    EvoGraph::Vertex* team;
    EvoGraph::Vertex* action;

    ASSERT_NO_THROW(team = new EvoGraph::TeamInstrumented());
    ASSERT_NO_THROW(action = new EvoGraph::ActionInstrumented(0));

    ASSERT_NO_THROW(delete team);
    ASSERT_NO_THROW(delete action);
}

TEST_F(TPGInstrumentedTest, VertexInstrumentationSettersAndGetters)
{
    // Test VertexInstrumentation through its TeamInstrumented
    // specialization.
    EvoGraph::TeamInstrumented team;

    ASSERT_EQ(team.getNbVisits(), 0)
        << "Number of visit on a newly constructed TeamInstrumented should "
           "be 0.";

    ASSERT_NO_THROW(team.incrementNbVisits())
        << "Increment of number of visits on a TeamInstrumented should not "
           "fail.";

    ASSERT_EQ(team.getNbVisits(), 1)
        << "Number of visit of a TeamInstrumented should "
           "be 1 after an increment.";

    ASSERT_NO_THROW(team.reset()) << "Reset of instrumentation counter should "
                                     "not fail on a TeamInstrumented.";

    ASSERT_EQ(team.getNbVisits(), 0)
        << "Number of visit of a TeamInstrumented should "
           "be 0 after a reset.";
}

TEST_F(TPGInstrumentedTest, EdgeInstrumentedConstructorsDestructors)
{
    EvoGraph::Team team;
    EvoGraph::Action action(1);
    EvoGraph::Edge* edge;

    ASSERT_NO_THROW(
        edge = new EvoGraph::EdgeInstrumented(&team, &action, progPointer));

    ASSERT_NO_THROW(delete edge);
}

TEST_F(TPGInstrumentedTest, EdgeInstrumentedSettersAndGetters)
{
    EvoGraph::Team team;
    EvoGraph::Action action(1);
    EvoGraph::EdgeInstrumented edge(&team, &action, progPointer);

    ASSERT_EQ(edge.getNbVisits(), 0)
        << "Number of visit on a newly constructed EdgeInstrumented should "
           "be 0.";

    ASSERT_EQ(edge.getNbTraversal(), 0)
        << "Number of traversal on a newly constructed EdgeInstrumented "
           "should "
           "be 0.";

    ASSERT_NO_THROW(edge.incrementNbVisits())
        << "Increment of number of visits on a EdgeInstrumented should not "
           "fail.";

    ASSERT_EQ(edge.getNbVisits(), 1)
        << "Number of visit of a EdgeInstrumented should "
           "be 1 after an increment.";

    ASSERT_NO_THROW(edge.incrementNbTraversal())
        << "Increment of number of traversal on a EdgeInstrumented should "
           "not "
           "fail.";

    ASSERT_EQ(edge.getNbTraversal(), 1)
        << "Number of traversal of a EdgeInstrumented should "
           "be 1 after an increment.";

    ASSERT_NO_THROW(edge.reset()) << "Reset of instrumentation counter should "
                                     "not fail on a EdgeInstrumented.";

    ASSERT_EQ(edge.getNbVisits(), 0)
        << "Number of visit of a EdgeInstrumented should "
           "be 0 after a reset.";

    ASSERT_EQ(edge.getNbTraversal(), 0)
        << "Number of traversal of a EdgeInstrumented should "
           "be 0 after a reset.";
}

TEST_F(TPGInstrumentedTest, TPGInstrumentedFactory)
{
    EvoGraph::TPGInstrumentedFactory factory;

    std::unique_ptr<EvoGraph::Action> action;
    std::unique_ptr<EvoGraph::Team> team;
    std::unique_ptr<EvoGraph::Edge> edge;
    std::unique_ptr<EvoGraph::OldExecutionEngine> tee;

    ASSERT_NO_THROW(action = factory.createAction(0))
        << "GraphFactory could not build a Action.";
    ASSERT_NE(action, nullptr) << "Created Action should not be null.";
    ASSERT_EQ(typeid(*action), typeid(EvoGraph::ActionInstrumented))
        << "Action built by the TPGInstrumentedFactory has an incorrect type.";

    ASSERT_NO_THROW(team = factory.createTeam())
        << "GraphELementFactory could not build a Action.";
    ASSERT_NE(team, nullptr) << "Created Team should not be null.";
    ASSERT_EQ(typeid(*team), typeid(EvoGraph::TeamInstrumented))
        << "Team built by the TPGInstrumentedFactory has an incorrect type.";

    ASSERT_NO_THROW(
        edge = factory.createEdge(team.get(), action.get(), progPointer))
        << "GraphELementFactory could not build a Action.";
    ASSERT_NE(edge.get(), nullptr) << "Created Edge should not be null.";
    ASSERT_EQ(typeid(*edge), typeid(EvoGraph::EdgeInstrumented))
        << "Edge built by the TPGInstrumentedFactory has an incorrect type.";

    ASSERT_NO_THROW(tee = factory.createExecutionEngine(*e, nullptr))
        << "GraphELementFactory could not build a OldExecutionEngine.";
    ASSERT_NE(tee.get(), nullptr) << "Created Edge should not be null.";
    ASSERT_EQ(typeid(*tee), typeid(EvoGraph::ExecutionEngineInstrumented))
        << "Edge built by the TPGInstrumentedFactory has an incorrect type.";
}

TEST_F(TPGInstrumentedTest, GraphAddVertexAndEdge)
{
    EvoGraph::Graph tpg(*e, std::make_unique<EvoGraph::TPGInstrumentedFactory>());
    const EvoGraph::Team* t;
    const EvoGraph::Action* a;
    const EvoGraph::Edge* e;

    ASSERT_NO_THROW(t = &tpg.addNewTeam())
        << "Adding a new Team to a Graph failed.";
    ASSERT_EQ(typeid(*t), typeid(EvoGraph::TeamInstrumented))
        << "Team built by the TPGInstrumentedFactory has an incorrect type.";

    ASSERT_NO_THROW(a = &tpg.addNewAction(0))
        << "Adding a new Action to a Graph failed.";
    ASSERT_EQ(typeid(*a), typeid(EvoGraph::ActionInstrumented))
        << "Action built by the TPGInstrumentedFactory has an incorrect type.";

    ASSERT_NO_THROW(e = &tpg.addNewEdge(*t, *a, progPointer));
    ASSERT_EQ(typeid(*e), typeid(EvoGraph::EdgeInstrumented))
        << "Edge built by the TPGInstrumentedFactory has an incorrect type.";
}

TEST_F(TPGInstrumentedTest, TPGInstrumentedFactoryReset)
{
    // Add to the TPG
    EvoGraph::Graph tpg(*e, std::make_unique<EvoGraph::TPGInstrumentedFactory>());
    const EvoGraph::TeamInstrumented& t =
        dynamic_cast<const EvoGraph::TeamInstrumented&>(tpg.addNewTeam());
    const EvoGraph::ActionInstrumented& a =
        dynamic_cast<const EvoGraph::ActionInstrumented&>(tpg.addNewAction(0));
    const EvoGraph::EdgeInstrumented& e =
        dynamic_cast<const EvoGraph::EdgeInstrumented&>(
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
        dynamic_cast<const EvoGraph::TPGInstrumentedFactory&>(tpg.getFactory())
            .resetGraphCounters(tpg));

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

    EvoGraph::Graph tpg(*e, std::make_unique<EvoGraph::TPGInstrumentedFactory>());
    const EvoGraph::TeamInstrumented* t[4];
    for (auto i = 0; i < 4; i++) {
        t[i] = dynamic_cast<const EvoGraph::TeamInstrumented*>(&tpg.addNewTeam());
    }

    const EvoGraph::ActionInstrumented* a[3];
    for (auto i = 0; i < 3; i++) {
        a[i] = dynamic_cast<const EvoGraph::ActionInstrumented*>(
            &tpg.addNewAction(i));
    }

    const EvoGraph::EdgeInstrumented* e[7];
    // T0->T1
    e[0] = dynamic_cast<const EvoGraph::EdgeInstrumented*>(
        &tpg.addNewEdge(*t[0], *t[1], progPointer));
    t[0]->incrementNbVisits();
    e[0]->incrementNbVisits();
    e[0]->incrementNbTraversal();
    // T0->A0 (traversed)
    e[1] = dynamic_cast<const EvoGraph::EdgeInstrumented*>(
        &tpg.addNewEdge(*t[0], *a[0], progPointer));
    e[1]->incrementNbVisits();
    e[1]->incrementNbTraversal();
    a[0]->incrementNbVisits();
    // T1->T2
    e[2] = dynamic_cast<const EvoGraph::EdgeInstrumented*>(
        &tpg.addNewEdge(*t[1], *t[2], progPointer));
    t[1]->incrementNbVisits();
    e[2]->incrementNbVisits();
    e[2]->incrementNbTraversal();
    // T2->A1
    e[3] = dynamic_cast<const EvoGraph::EdgeInstrumented*>(
        &tpg.addNewEdge(*t[2], *a[1], progPointer));
    t[2]->incrementNbVisits();
    e[3]->incrementNbVisits();
    e[3]->incrementNbTraversal();
    a[1]->incrementNbVisits();
    // T1->T3
    e[4] = dynamic_cast<const EvoGraph::EdgeInstrumented*>(
        &tpg.addNewEdge(*t[1], *t[3], progPointer));
    e[4]->incrementNbVisits();
    // T3->A2
    e[5] = dynamic_cast<const EvoGraph::EdgeInstrumented*>(
        &tpg.addNewEdge(*t[3], *a[2], progPointer));
    // T0->A0 (traversed)
    e[6] = dynamic_cast<const EvoGraph::EdgeInstrumented*>(
        &tpg.addNewEdge(*t[0], *a[0], progPointer));
    e[1]->incrementNbVisits();

    ASSERT_EQ(tpg.getNbVertices(), 7)
        << "Number of vertices of the Graph before being cleaned is not as "
           "expected.";

    ASSERT_EQ(tpg.getEdges().size(), 7)
        << "Number of edges of the Graph before being cleaned is not as "
           "expected.";

    ASSERT_NO_THROW(
        dynamic_cast<const EvoGraph::TPGInstrumentedFactory&>(tpg.getFactory())
            .clearUnusedGraphElements(tpg));

    ASSERT_EQ(tpg.getNbVertices(), 5)
        << "Number of vertices of the Graph after being cleaned is not as "
           "expected.";

    ASSERT_EQ(tpg.getEdges().size(), 4)
        << "Number of edges of the Graph after being cleaned is not as "
           "expected.";
}

#endif