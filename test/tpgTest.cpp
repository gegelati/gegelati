/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
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

#include <algorithm>
#include <gtest/gtest.h>

#include "algorithm/lgp/lgpAgent.h"

#include "data/dataHandler.h"
#include "data/primitiveTypeArray.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/lambdaInstruction.h"

#include "evoGraph/action.h"
#include "evoGraph/edge.h"
#include "evoGraph/graph.h"
#include "evoGraph/team.h"
#include "evoGraph/vertex.h"

#include "evoGraph/factory.h"

#include "util/counterReset.h"

class TPGTest : public ::testing::Test
{
  protected:
    const size_t size1{24};
    const size_t size2{32};
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
    Instructions::Set set;
    std::shared_ptr<const Environment> e = NULL;
    Learn::LearningParameters params;
    std::shared_ptr<Algorithm::Agent> programAgent;

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
        e = std::make_shared<Environment>(set, params, vect);
        programAgent =
            std::make_shared<Algorithm::LGP::LGPAgent>(e, 1, "fake");
    }

    virtual void TearDown()
    {
        delete (&(vect.at(0).get()));
        delete (&(vect.at(1).get()));
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
    }
};

TEST_F(TPGTest, TeamAndActionConstructorsDestructors)
{
    EvoGraph::Vertex* team;
    EvoGraph::Vertex* action;

    ASSERT_NO_THROW(team = new EvoGraph::Team());
    ASSERT_NO_THROW(action = new EvoGraph::Action(0));

    ASSERT_NO_THROW(delete team);
    ASSERT_NO_THROW(delete action);
}

TEST_F(TPGTest, EdgeConstructorDestructor)
{
    auto team = std::make_shared<EvoGraph::Team>();
    auto action = std::make_shared<EvoGraph::Action>(0);

    EvoGraph::Edge* edge;

    ASSERT_NO_THROW(edge = new EvoGraph::Edge(team, action, programAgent));

    ASSERT_NO_THROW(delete edge);
}

TEST_F(TPGTest, VertexEdgesSettersGetters)
{
    auto team = std::make_shared<EvoGraph::Team>();
    auto action = std::make_shared<EvoGraph::Action>(0);

    auto edge = std::make_shared<EvoGraph::Edge>(team, action, programAgent);

    ASSERT_NO_THROW(team->addOutgoingEdge(edge))
        << "Adding an outgoing edge to a Team vertex failed.";
    ASSERT_THROW(action->addOutgoingEdge(edge), std::runtime_error)
        << "Adding an outgoing edge to an Action vertex did not fail.";
    ASSERT_NO_THROW(action->addIncomingEdge(edge))
        << "Adding an outgoing edge to an Action vertex failed.";

    ASSERT_EQ(team->getOutgoingEdges().size(), 1)
        << "Size of the outgoing edges of the node is incorrect.";
    ASSERT_EQ(std::count(team->getOutgoingEdges().begin(),
                         team->getOutgoingEdges().end(),
                         edge),
              1)
        << "Edge pointer contained in the outgoingEdges is incorrect.";

    // Add the same edge again.. Nothing should happen, but it should not fail.
    ASSERT_NO_THROW(team->addOutgoingEdge(edge))
        << "Adding an outgoing edge to a Team vertex, even though it is "
           "already there, failed unexpectedly.";
    ASSERT_EQ(team->getOutgoingEdges().size(), 1)
        << "Size of the outgoing edges of the node is incorrect.";

    // Add Null pointer: Nothing should happen, but it should not fail.
    ASSERT_NO_THROW(team->addOutgoingEdge(nullptr))
        << "Adding an outgoing edge to a Team vertex, even though it is "
           "already there, failed unexpectedly.";
    ASSERT_EQ(team->getOutgoingEdges().size(), 1)
        << "Size of the outgoing edges of the node is incorrect.";

    ASSERT_EQ(action->getIncomingEdges().size(), 1)
        << "Size of the outgoing edges of the node is incorrect.";
    ASSERT_EQ(std::count(action->getIncomingEdges().begin(),
                         action->getIncomingEdges().end(), edge),
              1)
        << "Edge pointer contained in the outgoingEdges is incorrect.";

    // Remove edges
    ASSERT_NO_THROW(team->removeOutgoingEdge(edge))
        << "Removing an outgoing edge failed.";
    ASSERT_EQ(team->getOutgoingEdges().size(), 0)
        << "Size of the outgoing edges of the node is incorrect.";

    ASSERT_NO_THROW(action->removeIncomingEdge(edge))
        << "Removing an outgoing edge to an Action vertex failed.";
    ASSERT_EQ(action->getIncomingEdges().size(), 0)
        << "Size of the outgoing edges of the node is incorrect.";

    // Remove Null or non existing edge.
    ASSERT_NO_THROW(team->removeOutgoingEdge(nullptr))
        << "Removing an NULL edge failed, while it should just do nothing.";
    ASSERT_NO_THROW(action->removeIncomingEdge(edge))
        << "Removing an edge no longer in the set should do nothing, but not "
           "fail.";
}

TEST_F(TPGTest, EdgeGetSetProgram)
{
    auto team = std::make_shared<EvoGraph::Team>();
    auto action = std::make_shared<EvoGraph::Action>(0);

    std::shared_ptr<EvoGraph::Edge> constEdge = std::make_shared<EvoGraph::Edge>(team, action, programAgent);
    auto constProg = constEdge->getProgram();
    ASSERT_EQ(constProg, programAgent)
        << "Program accessor on const Edge returns a Program different from "
           "the one given at construction.";

    // program is a mutable attribute of the Edge.
    std::shared_ptr<Algorithm::Agent> programAgent2 =
            std::make_shared<Algorithm::LGP::LGPAgent>(e, 1, "fake");
    constEdge->setProgram(programAgent2);
    ASSERT_EQ(constEdge->getProgram(), programAgent2)
        << "Program accessor on Edge returns a Program different from the "
           "one set before.";
}

TEST_F(TPGTest, EdgeGetSetSourceAndDestination)
{
    auto team0 = std::make_shared<EvoGraph::Team>();
    auto team1 = std::make_shared<EvoGraph::Team>();
    auto action0 = std::make_shared<EvoGraph::Action>(1);
    auto action1 = std::make_shared<EvoGraph::Action>(0);

    auto edge = std::make_shared<EvoGraph::Edge>(team0, action0, programAgent);

    ASSERT_EQ(team0, edge->getSource())
        << "Source of the Edge differs from the one given at construction.";
    ASSERT_EQ(action0, edge->getDestination())
        << "Destination of the Edge differs from the one given at "
           "construction.";

    edge->setSource(team1);
    ASSERT_EQ(team1, edge->getSource())
        << "Source of the Edge differs from the one set right before.";

    edge->setDestination(action1);
    ASSERT_EQ(action1, edge->getDestination())
        << "Destination of the Edge differs from the one set right before.";
}

TEST_F(TPGTest, ActionEdgeGetSet)
{
    auto action0 = std::make_shared<EvoGraph::Action>(1);

    auto actionEdge = std::make_shared<EvoGraph::ActionEdge>(action0, programAgent, 0);

    ASSERT_THROW(actionEdge->getDestination(), std::runtime_error)
        << "ActionEdge does not have destination.";
    ASSERT_THROW(actionEdge->setDestination(action0), std::runtime_error)
        << "ActionEdge does not have destination.";

    ASSERT_EQ(actionEdge->getActionClass(), 0)
        << "Action class of the ActionEdge is wrong";

    actionEdge->setActionClass(1);
    ASSERT_EQ(actionEdge->getActionClass(), 1)
        << "Action class of the ActionEdge has not been changed successfuly";
}

TEST_F(TPGTest, GraphFactory)
{
    EvoGraph::GraphFactory factory;

    std::shared_ptr<EvoGraph::Action> action;
    std::shared_ptr<EvoGraph::Team> team;
    std::shared_ptr<EvoGraph::Edge> edge;
    std::shared_ptr<EvoGraph::Edge> actionEdge;

    ASSERT_NO_THROW(action = factory.createAction(0))
        << "GraphELementFactory could not build a action->";
    ASSERT_NE(action, nullptr) << "Created Action should not be null.";

    ASSERT_NO_THROW(team = factory.createTeam())
        << "GraphELementFactory could not build a team->";
    ASSERT_NE(team, nullptr) << "Created Team should not be null.";

    ASSERT_NO_THROW(
        edge = factory.createEdge(team, action, programAgent))
        << "GraphELementFactory could not build a Edge.";
    ASSERT_NE(edge.get(), nullptr) << "Created Edge should not be null.";

    ASSERT_NO_THROW(
        actionEdge = factory.createActionEdge(action, programAgent, 0))
        << "GraphELementFactory could not build a ActionEdge.";
    ASSERT_NE(actionEdge.get(), nullptr)
        << "Created ActionEdge should not be null.";
}

TEST_F(TPGTest, GraphAddVertex)
{
    EvoGraph::Graph tpg;
    std::shared_ptr<const EvoGraph::Action> action;
    std::shared_ptr<const EvoGraph::Team> team;
    ASSERT_NO_THROW(team = tpg.addNewTeam())
        << "Adding a new Team to a Graph failed.";
    ASSERT_NO_THROW(action = tpg.addNewAction(0))
        << "Adding a new Action to a Graph failed.";
}

TEST_F(TPGTest, GraphConstructorDestructor)
{
    EvoGraph::Graph* tpg;

    ASSERT_NO_THROW(tpg = new EvoGraph::Graph())
        << "Error while calling a Graph constructor.";

    ASSERT_NE(tpg, nullptr)
        << "Graph construction succeded but returned a null pointer.";

    ASSERT_NO_THROW(delete tpg) << "Destruction of a Graph failed.";
}

TEST_F(TPGTest, GraphHasVertex)
{
    EvoGraph::Graph tpg;
    std::shared_ptr<const EvoGraph::Action> action;
    std::shared_ptr<const EvoGraph::Team> team;
    ASSERT_NO_THROW(team = tpg.addNewTeam())
        << "Adding a new Team to a Graph failed.";
    ASSERT_NO_THROW(action = tpg.addNewAction(0))
        << "Adding a new Action to a Graph failed.";

    EvoGraph::Action external(12);

    ASSERT_TRUE(tpg.hasVertex(*team))
        << "A Vertex from the Graph was not detected as such.";
    ASSERT_FALSE(tpg.hasVertex(external))
        << "A Vertex from the Graph was wrongfully detected as such.";
}

TEST_F(TPGTest, GraphGetNbVertices)
{
    EvoGraph::Graph tpg;
    tpg.addNewTeam();
    tpg.addNewAction(0);
    ASSERT_EQ(tpg.getNbVertices(), 2)
        << "Number of vertices in the Graph is incorrect.";
}

TEST_F(TPGTest, GraphGetVertices)
{
    EvoGraph::Graph tpg;
    std::shared_ptr<const EvoGraph::Team> vertex = tpg.addNewTeam();
    const std::vector<std::shared_ptr<const EvoGraph::Vertex>> vertices = tpg.getVertices();
    ASSERT_EQ(vertices.size(), 1)
        << "Size of the retrievd std::vector<Vertex> is incorrect.";
    ASSERT_EQ(vertices.front(), vertex)
        << "Vertex in the retrieved vertices list does not correspond to the "
           "one added to the TPGGrapg (pointer comparison)";
}

TEST_F(TPGTest, GraphAddEdge)
{
    EvoGraph::Graph tpg;
    auto vertex0 = tpg.addNewTeam();
    auto vertex1 = tpg.addNewAction(0);

    ASSERT_NO_THROW(tpg.addNewEdge(*vertex0, *vertex1, programAgent))
        << "Adding an edge between a team and an action failed.";
    // Add with a vertex not in the graph.
    EvoGraph::Action vertex2(2);
    ASSERT_THROW(tpg.addNewEdge(*vertex0, vertex2, programAgent),
                 std::runtime_error)
        << "Adding an edge with a vertex not from the graph should have "
           "failed.";

    // Add the edge from the action
    ASSERT_THROW(tpg.addNewEdge(*vertex1, *vertex0, programAgent),
                 std::runtime_error)
        << "Adding an edge from an Action should have failed.";

    auto vertex3 = tpg.getFactory().createTeam();
    auto edge = tpg.getFactory().createActionEdge(vertex1, programAgent, 0);
    ASSERT_THROW(vertex3->addOutgoingEdge(edge), std::runtime_error)
        << "Adding an action edge from a Vertex that is not an action "
           "should have failed.";
}

TEST_F(TPGTest, GraphAddActionEdge)
{
    EvoGraph::Graph tpg;
    auto vertex0 = tpg.addNewTeam();
    auto vertex1 = tpg.addNewAction(0);

    ASSERT_NO_THROW(tpg.addNewActionEdge(*vertex1, programAgent, 0))
        << "Adding an action edge from an action failed.";
    // Add with a vertex not in the graph.
    EvoGraph::Action vertex2(2);
    ASSERT_THROW(tpg.addNewActionEdge(vertex2, programAgent, 0),
                 std::runtime_error)
        << "Adding an edge with a vertex not from the graph should have "
           "failed.";

    // Add the edge from a team
    ASSERT_THROW(tpg.addNewActionEdge(*vertex0, programAgent, 0),
                 std::runtime_error)
        << "Adding an edge from an Action should have failed.";
}

TEST_F(TPGTest, GraphGetEdges)
{
    EvoGraph::Graph tpg;
    auto vertex0 = tpg.addNewTeam();
    auto vertex1 = tpg.addNewAction(0);

    auto edge = tpg.addNewEdge(*vertex0, *vertex1, programAgent);
    ASSERT_EQ(tpg.getEdges().size(), 1)
        << "Edges of the graph have incorrect size after successful add.";

    // Check that connection were added
    // To the source
    ASSERT_EQ(vertex0->getOutgoingEdges().size(), 1);
    ASSERT_EQ(std::count_if(vertex0->getOutgoingEdges().begin(),
                            vertex0->getOutgoingEdges().end(),
                            [edge](std::shared_ptr<const EvoGraph::Edge> other) {
                                return other == edge;
                            }),
              1);

    // To the destination
    ASSERT_EQ(vertex1->getIncomingEdges().size(), 1);
    ASSERT_EQ(std::count_if(vertex1->getIncomingEdges().begin(),
                            vertex1->getIncomingEdges().end(),
                            [edge](std::shared_ptr<const EvoGraph::Edge> other) {
                                return other == edge;
                            }),
              1);

    // Attempt an impossible add.
    ASSERT_THROW(tpg.addNewEdge(*vertex1, *vertex0, programAgent),
                 std::runtime_error)
        << "An exception should be thrown when adding an impossible edge.";

    ASSERT_EQ(tpg.getEdges().size(), 1)
        << "Edges of the graph have incorrect size after unsuccessful add.";
}

TEST_F(TPGTest, GraphRemoveEdge)
{
    EvoGraph::Graph tpg;
    auto vertex0 = tpg.addNewTeam();
    auto vertex1 = tpg.addNewAction(0);

    const auto& edge = *tpg.addNewEdge(*vertex0, *vertex1, programAgent);


    // Remove the edge
    ASSERT_NO_THROW(tpg.removeEdge(edge))
        << "Edge from the graph could not be removed successfully.";
    // Check that the edge is no longer in the graph
    ASSERT_EQ(tpg.getEdges().size(), 0)
        << "Edge was not effectively removed from the graph.";
    // Check that vertices were disconnected from the removed edge.
    ASSERT_EQ(vertex0->getOutgoingEdges().size(), 0)
        << "Source vertex was not disconnected from the removed Edge.";
    // and from the destination
    ASSERT_EQ(vertex1->getIncomingEdges().size(), 0)
        << "Destination vertex was not disconnected from the removed Edge.";
    // Check that the edge was successfully deleted
    ASSERT_EQ(programAgent.use_count(), 1)
        << "Edge was not properly deleted, its shared pointer is still active.";
    // Remove an edge that does not exist anymore
    ASSERT_THROW(tpg.removeEdge(edge), std::runtime_error)
        << "Edge not in the graph should not be removable";
}

TEST_F(TPGTest, GraphRemoveActionEdge)
{
    EvoGraph::Graph tpg;
    auto vertex = tpg.addNewAction(0);

    const auto& edge0 = *tpg.addNewActionEdge(*vertex, programAgent, 0);
    const auto& edge1 = *tpg.addNewActionEdge(*vertex, programAgent, 1);

    // Remove the edge
    ASSERT_NO_THROW(tpg.removeEdge(edge0))
        << "Edge from the graph could not be removed successfully with "
           "removeEdge.";
    // Check that the edge is no longer in the graph
    ASSERT_EQ(tpg.getEdges().size(), 1)
        << "Edge was not effectively removed from the graph.";
    // Remove the edge
    ASSERT_NO_THROW(tpg.removeActionEdge(edge1))
        << "Edge from the graph could not be removed successfully with "
           "removeActionEdge.";
    // Check that the edge is no longer in the graph
    ASSERT_EQ(tpg.getEdges().size(), 0)
        << "Edge was not effectively removed from the graph.";
    // Check that the edge was successfully deleted
    ASSERT_EQ(programAgent.use_count(), 1)
        << "Edge was not properly deleted, its shared pointer is still active.";
    // Remove an edge that does not exist anymore
    ASSERT_THROW(tpg.removeActionEdge(edge1), std::runtime_error)
        << "Edge not in the graph should not be removable";
}

TEST_F(TPGTest, GraphRemoveVertex)
{
    EvoGraph::Graph tpg;
    auto vertex0 = tpg.addNewTeam();
    auto vertex1 = tpg.addNewAction(0);
    auto vertex2 = tpg.addNewTeam();

    ASSERT_NO_THROW(tpg.removeVertex(*vertex0))
        << "Removing a vertex from the graph failed.";
    ASSERT_EQ(tpg.getNbVertices(), 2)
        << "Number of vertices of the TPG is incorrect after removing a "
           "Vertex.";
    ASSERT_EQ(tpg.getVertices().front(), vertex1)
        << "Remaining vertex after removal is not correct.";
    ASSERT_EQ(tpg.getVertices().back(), vertex2)
        << "Remaining vertex after removal is not correct.";

    // Try to remove a vertex not from the graph
    EvoGraph::Action vertex3(3);
    ASSERT_NO_THROW(tpg.removeVertex(vertex3))
        << "Removing a vertex from the graph (although it is not inside) "
           "throwed an exception.";
    ASSERT_EQ(tpg.getNbVertices(), 2)
        << "Number of vertices of the TPG is incorrect after removing a "
           "Vertex not from the graph.";

    // Add a new edge to test removal of vertex connectet to an edge.
    tpg.addNewEdge(*vertex2, *vertex1, programAgent);
    ASSERT_NO_THROW(tpg.removeVertex(*vertex2))
        << "Removing a vertex from the graph failed.";
    // Check that edge was removed from the graph
    ASSERT_EQ(tpg.getEdges().size(), 0)
        << "Edge connected to the removed vertex was not removed from the "
           "graph.";
    // And disconnected from vertex1
    ASSERT_EQ(vertex1->getIncomingEdges().size(), 0)
        << "Edge connected to the vertex removed from the graph was not "
           "disconnected from its destination.";

    // For code coverage, test when the destination vertex of an edge is removed
    // Add a new edge to test removal of vertex connectet to an edge.
    auto vertex4 = tpg.addNewTeam();
    tpg.addNewEdge(*vertex4, *vertex1, programAgent);
    ASSERT_NO_THROW(tpg.removeVertex(*vertex1))
        << "Removing a vertex from the graph failed.";
    // Check that edge was removed from the graph
    ASSERT_EQ(tpg.getEdges().size(), 0)
        << "Edge connected to the removed vertex was not removed from the "
           "graph.";
    // And disconnected from vertex1
    ASSERT_EQ(vertex4->getOutgoingEdges().size(), 0)
        << "Edge connected to the vertex removed from the graph was not "
           "disconnected from its destination.";
}

TEST_F(TPGTest, GraphClear)
{
    EvoGraph::Graph tpg;
    auto vertex0 = tpg.addNewTeam();
    auto vertex1 = tpg.addNewAction(0);
    auto edge = tpg.addNewEdge(*vertex0, *vertex1, programAgent);

    ASSERT_NO_THROW(tpg.clear()) << "Clearing a non empty graph failed.";
    ASSERT_EQ(tpg.getNbVertices(), 0)
        << "Cleared graph is not empty of vertices as expected.";
    ASSERT_EQ(tpg.getEdges().size(), 0)
        << "Cleared graph is not empty of edges as expected.";
}

TEST_F(TPGTest, GraphGetNbRootVertices)
{
    EvoGraph::Graph tpg;
    auto vertex0 = tpg.addNewTeam();
    auto vertex1 = tpg.addNewAction(0);
    auto edge = tpg.addNewEdge(*vertex0, *vertex1, programAgent);

    ASSERT_EQ(tpg.getNbRootVertices(), 1)
        << "Number of roots of the TPG is incorrect.";
}

TEST_F(TPGTest, GraphGetRootVertices)
{
    EvoGraph::Graph tpg;
    auto vertex0 = tpg.addNewTeam();
    auto vertex1 = tpg.addNewAction(0);

    auto edge = tpg.addNewEdge(*vertex0, *vertex1, programAgent);
    ASSERT_EQ(tpg.getRootVertices().size(), 1)
        << "Number of roots of the TPG is incorrect.";
    ASSERT_EQ(tpg.getRootVertices().at(0), vertex0)
        << "Vertex classified as root is incorrect.";

    auto vertex2 = tpg.addNewTeam();
    auto vertex3 = tpg.addNewAction(1);
    ASSERT_EQ(tpg.getRootVertices().size(), 3)
        << "Number of roots of the TPG is incorrect.";
    ASSERT_EQ(tpg.getRootTeams().size(), 2)
        << "Number of roots teams of the TPG is incorrect.";
    ASSERT_EQ(tpg.getRootActions().size(), 1)
        << "Number of roots actions of the TPG is incorrect.";
}

TEST_F(TPGTest, GraphCloneVertex)
{
    EvoGraph::Graph tpg;
    auto vertex0 = tpg.addNewTeam();
    auto vertex1 = tpg.addNewAction(4);

    auto edge0 = tpg.addNewEdge(*vertex0, *vertex1, programAgent);
    auto edge1 = tpg.addNewActionEdge(*vertex1, programAgent, 0);

    // Clone the team
    std::shared_ptr<const EvoGraph::Vertex> cloneVertex;
    ASSERT_NO_THROW(cloneVertex = tpg.cloneVertex(*vertex0))
        << "Cloning a TeamVertex of the Graph failed.";
    // Check that the clone vertex is in the graph
    ASSERT_EQ(tpg.getNbVertices(), 3)
        << "Number of vertices of the graph after clone is incorrect.";
    ASSERT_EQ(tpg.getVertices().at(2), cloneVertex)
        << "CloneVertex is not the last of the graph vertices as it should be.";
    cloneVertex = tpg.getVertices().at(2); // to remove a compilation warning.
    // Check that the type is correct
    ASSERT_EQ(typeid(*vertex0), typeid(*cloneVertex));
    ASSERT_EQ(tpg.getEdges().size(), 3)
        << "Number of edges of the graph after clone is incorrect.";
    auto destinationVertex =
        ((*cloneVertex->getOutgoingEdges().begin())->getDestination());
    ASSERT_EQ(destinationVertex, vertex1)
        << "Cloned vertex is not connected to the correct other vertex in the "
           "Graph.";
    // Check pointer usage was increased.
    ASSERT_EQ(programAgent.use_count(), 4)
        << "Shared pointer use count should increase after cloning a vertex "
           "connected with an edge using it.";

    // Duplicate the action (to increase code coverage)
    ASSERT_NO_THROW(tpg.cloneVertex(*vertex1));
    // Check that the type is correct
    auto vertex = tpg.getVertices().at(3);
    ASSERT_EQ(typeid(*vertex1).name(), typeid(*vertex).name());
    ASSERT_EQ(vertex1->getActionID(),
              std::dynamic_pointer_cast<const EvoGraph::Action>(tpg.getVertices().at(3))->getActionID());
    ASSERT_EQ(tpg.getEdges().size(), 4)
        << "Number of edges of the graph after clone is incorrect.";
    // Check pointer usage was increased.
    ASSERT_EQ(programAgent.use_count(), 5)
        << "Shared pointer use count should increase after cloning a vertex "
           "connected with an edge using it.";

    // Clone a vertex not from the graph
    EvoGraph::Vertex* vertex2 = new EvoGraph::Action(1);
    ASSERT_THROW(tpg.cloneVertex(*vertex2), std::runtime_error)
        << "Cloning a vertex that does not belong to the Graph should not "
           "be possible.";
}

TEST_F(TPGTest, GraphCloneEdge)
{
    EvoGraph::Graph tpg;
    auto vertex0 = tpg.addNewTeam();
    auto vertex1 = tpg.addNewAction(4);
    auto edge = tpg.addNewEdge(*vertex0, *vertex1, programAgent);
    auto actionEdge =
        tpg.addNewActionEdge(*vertex1, programAgent, 0);

    std::shared_ptr<const EvoGraph::Edge> clone;
    ASSERT_NO_THROW(clone = tpg.cloneEdge(*edge))
        << "Cloning an existing edge failed.";
    // Check that the new edge is correctly added to the graph
    ASSERT_EQ(tpg.getEdges().size(), 3)
        << "Incorrect number of edges in the graph after clone.";
    // Check the program use
    ASSERT_EQ(programAgent.use_count(), 4)
        << "Program pointer was not correctly registered to the edge clone.";
    // Check the edge source and destination
    ASSERT_EQ(clone->getSource(), vertex0)
        << "Clone edge has an incorrect source.";
    ASSERT_EQ(clone->getDestination(), vertex1)
        << "Clone edge has an incorrect destination.";
    // Check that the edge was correctly registered
    ASSERT_EQ(std::count_if(vertex0->getOutgoingEdges().begin(),
                            vertex0->getOutgoingEdges().end(),
                            [clone](std::shared_ptr<const EvoGraph::Edge> other) {
                                return other == clone;
                            }),
              1)
        << "Clone edge is not registered within its source vertex outgoing "
           "edges.";
    ASSERT_EQ(std::count_if(vertex1->getIncomingEdges().begin(),
                            vertex1->getIncomingEdges().end(),
                            [clone](std::shared_ptr<const EvoGraph::Edge> other) {
                                return other == clone;
                            }),
              1)
        << "Clone edge is not registered within its destination vertex "
           "incoming edges.";

    // Check clone from action edge
    std::shared_ptr<const EvoGraph::Edge> cloneActionEdge;
    ASSERT_NO_THROW(cloneActionEdge = tpg.cloneEdge(*actionEdge))
        << "Cloning an existing action edge failed.";
    // Check that the new edge is correctly added to the graph
    ASSERT_EQ(tpg.getEdges().size(), 4)
        << "Incorrect number of edges in the graph after clone.";
    // Check the program use
    ASSERT_EQ(programAgent.use_count(), 5)
        << "Program pointer was not correctly registered to the edge clone.";
    ASSERT_TRUE(std::dynamic_pointer_cast<const EvoGraph::ActionEdge>(cloneActionEdge) !=
                nullptr)
        << "Cloning the action edge did not create a action edge";

    auto cloneActionEdgeCast =
        std::dynamic_pointer_cast<const EvoGraph::ActionEdge>(cloneActionEdge);
    // Check the edge source and destination
    ASSERT_EQ(cloneActionEdgeCast->getSource(), vertex1)
        << "Clone action edge has an incorrect source.";
    ASSERT_EQ(cloneActionEdgeCast->getActionClass(), 0)
        << "Clone action edge has an incorrect action Class.";

    // Check throw behavior
    EvoGraph::Edge newEdge(vertex0, vertex1, programAgent);
    ASSERT_THROW(tpg.cloneEdge(newEdge), std::runtime_error)
        << "Cloning an edge not from the graph should not succeed.";

    EvoGraph::ActionEdge newActionEdge(vertex1, programAgent, 0);
    ASSERT_THROW(tpg.cloneEdge(newActionEdge), std::runtime_error)
        << "Cloning an action edge not from the graph should not succeed.";
}

TEST_F(TPGTest, GraphSetEdgeDestination)
{
    EvoGraph::Graph tpg;
    auto vertex0 = tpg.addNewTeam();
    auto vertex1 = tpg.addNewAction(4);
    auto vertex2 = tpg.addNewAction(4);
    auto edge = tpg.addNewEdge(*vertex0, *vertex1, programAgent);

    // Change the destination of the edge
    ASSERT_TRUE(tpg.setEdgeDestination(*edge, *vertex2))
        << "Changing the destination of an Edge to a valid new destination "
           "should not fail.";
    // Check the graph size
    ASSERT_EQ(tpg.getEdges().size(), 1)
        << "Incorrect number of edges in the graph after edge setDestination.";
    // Check the program use
    ASSERT_EQ(programAgent.use_count(), 2)
        << "Program pointer use should not be affected by edge destination "
           "change.";
    // Check the edge source and destination
    ASSERT_EQ(edge->getSource(), vertex0)
        << "Updated edge has an incorrect source.";
    ASSERT_EQ(edge->getDestination(), vertex2)
        << "Updated edge has an incorrect destination.";
    // Check that the edge was correctly registered
    ASSERT_EQ(std::count_if(vertex0->getOutgoingEdges().begin(),
                            vertex0->getOutgoingEdges().end(),
                            [edge](std::shared_ptr<const EvoGraph::Edge> other) {
                                return other == edge;
                            }),
              1)
        << "Updated edge is no longer registered within its source vertex "
           "outgoing edges.";
    ASSERT_EQ(std::count_if(vertex2->getIncomingEdges().begin(),
                            vertex2->getIncomingEdges().end(),
                            [edge](std::shared_ptr<const EvoGraph::Edge> other) {
                                return other == edge;
                            }),
              1)
        << "Updated edge is not registered within its new destination vertex "
           "incoming edges.";
    // Check that the edge was unregistered
    ASSERT_EQ(vertex1->getIncomingEdges().size(), 0)
        << "This vertex should not have incomingEdge after destination change.";

    // Check failure
    EvoGraph::Edge newEdge(vertex0, vertex1, programAgent);
    ASSERT_FALSE(tpg.setEdgeDestination(newEdge, *vertex2))
        << "Changing destination of an edge not within the graph should not "
           "succeed.";
}

TEST_F(TPGTest, GraphSetEdgeSource)
{
    EvoGraph::Graph tpg;
    auto vertex0 = tpg.addNewTeam();
    auto vertex1 = tpg.addNewAction(4);
    auto vertex2 = tpg.addNewTeam();
    auto edge = tpg.addNewEdge(*vertex0, *vertex1, programAgent);

    // Change the destination of the edge
    ASSERT_TRUE(tpg.setEdgeSource(*edge, *vertex2))
        << "Changing the destination of an Edge to a valid new destination "
           "should not fail.";
    // Check the graph size
    ASSERT_EQ(tpg.getEdges().size(), 1)
        << "Incorrect number of edges in the graph after edge setDestination.";
    // Check the program use
    ASSERT_EQ(programAgent.use_count(), 2)
        << "Program pointer use should not be affected by edge source change.";
    // Check the edge source and destination
    ASSERT_EQ(edge->getSource(), vertex2)
        << "Updated edge has an incorrect source.";
    ASSERT_EQ(edge->getDestination(), vertex1)
        << "Updated edge has an incorrect destination.";
    // Check that the edge was correctly registered
    ASSERT_EQ(std::count_if(vertex2->getOutgoingEdges().begin(),
                            vertex2->getOutgoingEdges().end(),
                            [edge](std::shared_ptr<const EvoGraph::Edge> other) {
                                return other == edge;
                            }),
              1)
        << "Updated edge is no longer registered within its new source vertex "
           "outgoing edges.";
    ASSERT_EQ(std::count_if(vertex1->getIncomingEdges().begin(),
                            vertex1->getIncomingEdges().end(),
                            [edge](std::shared_ptr<const EvoGraph::Edge> other) {
                                return other == edge;
                            }),
              1)
        << "Updated edge is not registered within its destination vertex "
           "incoming edges.";
    // Check that the edge was unregistered
    ASSERT_EQ(vertex0->getOutgoingEdges().size(), 0)
        << "This vertex should not have incomingEdge after source change.";

    // Check failure
    EvoGraph::Edge newEdge(vertex0, vertex1, programAgent);
    ASSERT_FALSE(tpg.setEdgeSource(newEdge, *vertex2))
        << "Changing source of an edge not within the graph should not "
           "succeed.";
}

TEST_F(TPGTest, TPGMoveOperator)
{
    EvoGraph::Graph source;
    EvoGraph::Graph* destination =
        new EvoGraph::Graph(); // creates an empty tpg graph

    auto vertex0 = source.addNewTeam();
    auto vertex1 = source.addNewAction(4);
    auto vertex2 = source.addNewTeam();
    auto edge = source.addNewEdge(*vertex0, *vertex1, programAgent);
    auto edge2 =
        source.addNewEdge(*vertex2, *vertex1, programAgent);
    auto edge3 =
        source.addNewEdge(*vertex0, *vertex2, programAgent);

    /*
     *	 T2
     *	  ^	\
     *    |	  A4
     *	 T0	/
     */

    ASSERT_NO_THROW(*destination = std::move(source))
        << "The move operator is never supposed to fail";
    ASSERT_EQ(destination->getNbVertices(), 3)
        << "All verticies were not moved";
    ASSERT_EQ(source.getNbVertices(), 0)
        << "Some verticies are still present in the source graph";
    ASSERT_EQ(destination->getEdges().size(), 3) << "All edges were not moved";
    ASSERT_EQ(source.getEdges().size(), 0)
        << "Some edges are still present in the source graph";
    delete (destination);
}
TEST_F(TPGTest, TPGAffectationOperator)
{
    EvoGraph::Graph source;

    ASSERT_NO_THROW(EvoGraph::Graph& destination = source)
        << "The affectation operator is never supposed to fail";
}

TEST_F(TPGTest, ActionOutgoingEdge)
{
    // Create a Action
    auto action = std::make_shared<EvoGraph::Action>(42);

    // Try to add a non-ActionEdge outgoing edge (should throw)
    auto team = std::make_shared<EvoGraph::Team>();
    auto edge = std::make_shared<EvoGraph::Edge>(team, action, programAgent);
    ASSERT_THROW(action->addOutgoingEdge(edge), std::runtime_error);

    // Add valid ActionEdges with different actionClass
    auto edge0 = std::make_shared<EvoGraph::ActionEdge>(action, programAgent, 2);
    auto edge1 = std::make_shared<EvoGraph::ActionEdge>(action, programAgent, 1);
    auto edge2 = std::make_shared<EvoGraph::ActionEdge>(action, programAgent, 3);

    // Add them as outgoing edges (should not throw)
    ASSERT_NO_THROW(action->addOutgoingEdge(edge0));
    ASSERT_NO_THROW(action->addOutgoingEdge(edge1));
    ASSERT_NO_THROW(action->addOutgoingEdge(edge2));

    // Test orderActionEdges (should sort by actionClass)
    //action->orderActionEdges();
    auto it = action->getOutgoingEdges().begin();
    ASSERT_EQ(std::dynamic_pointer_cast<const EvoGraph::ActionEdge>(*it)->getActionClass(), 1);
    ++it;
    ASSERT_EQ(std::dynamic_pointer_cast<const EvoGraph::ActionEdge>(*it)->getActionClass(), 2);
    ++it;
    ASSERT_EQ(std::dynamic_pointer_cast<const EvoGraph::ActionEdge>(*it)->getActionClass(), 3);

    // Test getEdgeOfAction for existing and non-existing actionClass
    ASSERT_EQ(action->getEdgeOfAction(2), edge0);
    ASSERT_EQ(action->getEdgeOfAction(1), edge1);
    ASSERT_EQ(action->getEdgeOfAction(3), edge2);
    ASSERT_EQ(action->getEdgeOfAction(99), nullptr);

}

TEST_F(TPGTest, VertexHasSameAssessedActions)
{
    EvoGraph::Graph tpg;
    auto action = tpg.addNewAction(0);
    tpg.addNewActionEdge(*action, programAgent, 1);
    tpg.addNewActionEdge(*action, programAgent, 2);
    tpg.addNewActionEdge(*action, programAgent, 3);

    // Case 1: Intersection is not empty (should return true)
    std::set<uint64_t> testSet1 = {2, 4, 5};
    ASSERT_TRUE(action->hasSameAssessedActions(testSet1))
        << "hasSameAssessedActions should return true when intersection is not "
           "empty.";

    // Case 2: Intersection is empty (should return false)
    std::set<uint64_t> testSet2 = {4, 5, 6};
    ASSERT_FALSE(action->hasSameAssessedActions(testSet2))
        << "hasSameAssessedActions should return false when intersection is "
           "empty.";

    // Case 3: Exact match (should return true)
    std::set<uint64_t> testSet3 = {1, 2, 3};
    ASSERT_TRUE(action->hasSameAssessedActions(testSet3))
        << "hasSameAssessedActions should return true when sets are identical.";

    // Case 4: Partial overlap (should return true)
    std::set<uint64_t> testSet4 = {3};
    ASSERT_TRUE(action->hasSameAssessedActions(testSet4))
        << "hasSameAssessedActions should return true when there is a single "
           "common element.";

    // Case 5: Empty input set (should return false)
    std::set<uint64_t> testSet5;
    ASSERT_FALSE(action->hasSameAssessedActions(testSet5))
        << "hasSameAssessedActions should return false when input set is "
           "empty.";

    // Case 6: Empty assessedActions (should return false)
    EvoGraph::Team emptyTeam;
    std::set<uint64_t> testSet6 = {1, 2};
    ASSERT_FALSE(emptyTeam.hasSameAssessedActions(testSet6))
        << "hasSameAssessedActions should return false when assessedActions is "
           "empty.";
}

TEST_F(TPGTest, GraphSetActionClassEdge)
{
    EvoGraph::Graph tpg;
    auto action = tpg.addNewAction(0);

    // Add an action edge
    auto edge = tpg.addNewActionEdge(*action, programAgent, 1);
    // Change the action class
    ASSERT_NO_THROW(tpg.setActionClassEdge(edge, 42));
    // Check that the action class was updated
    auto actionEdge = std::dynamic_pointer_cast<const EvoGraph::ActionEdge>(edge);
    ASSERT_NE(actionEdge, nullptr);
    ASSERT_EQ(actionEdge->getActionClass(), 42);

    // Try to set action class on a non-action edge (should throw)
    auto team = tpg.addNewTeam();
    auto normalEdge = tpg.addNewEdge(*team, *action, programAgent);
    ASSERT_THROW(tpg.setActionClassEdge(normalEdge, 5), std::runtime_error);

    // Try to set action class on an edge not in the graph (should throw)
    auto fakeEdge = std::make_shared<EvoGraph::ActionEdge>(action, programAgent, 0);
    ASSERT_THROW(tpg.setActionClassEdge(fakeEdge, 7), std::runtime_error);
}

TEST_F(TPGTest, GraphUpdateAssessedActions)
{
    EvoGraph::Graph tpg;
    auto action1 = tpg.addNewAction(0);
    auto edge1 = tpg.addNewActionEdge(*action1, programAgent, 1);
    auto edge2 = tpg.addNewActionEdge(*action1, programAgent, 2);

    // Should update without throwing
    ASSERT_NO_THROW(tpg.updateAssessedActions(action1));
    // Check that assessedActions contains the correct action classes: 1 and 2
    const auto& assessed = action1->getAssessedActions();
    ASSERT_TRUE(assessed.find(1) != assessed.end());
    ASSERT_TRUE(assessed.find(2) != assessed.end());
    ASSERT_FALSE(assessed.find(0) != assessed.end());

    auto action2 = tpg.addNewAction(0);
    auto edge3 = tpg.addNewActionEdge(*action2, programAgent, 1);
    auto edge4 = tpg.addNewActionEdge(*action2, programAgent, 3);

    // Vertex should contain action 1, 2 and 3 now.
    auto vertex = tpg.addNewTeam();
    tpg.addNewEdge(*vertex, *action2, programAgent);
    tpg.addNewEdge(*vertex, *action1, programAgent);

    // Should update without throwing
    ASSERT_NO_THROW(tpg.updateAssessedActions(action2));
    // Check that assessedActions contains the correct action classes
    const auto& assessed2 = vertex->getAssessedActions();
    ASSERT_TRUE(assessed2.find(1) != assessed2.end());
    ASSERT_TRUE(assessed2.find(2) != assessed2.end());
    ASSERT_TRUE(assessed2.find(3) != assessed2.end());

    // Try with a vertex not in the graph (should throw)
    auto fakeAction = std::make_shared<EvoGraph::Action>(99);
    ASSERT_THROW(tpg.updateAssessedActions(fakeAction), std::runtime_error);
}

TEST_F(TPGTest, GraphUpdateAllAssessedActions)
{
    EvoGraph::Graph tpg;
    auto action1 = tpg.addNewAction(0);
    auto action2 = tpg.addNewAction(1);
    tpg.addNewActionEdge(*action1, programAgent, 1);
    tpg.addNewActionEdge(*action2, programAgent, 2);

    // Should update all actions without throwing
    ASSERT_NO_THROW(tpg.updateAllAssessedActions());
    // Check that both actions have their assessedActions updated
    ASSERT_TRUE(action1->getAssessedActions().find(1) !=
                action1->getAssessedActions().end());
    ASSERT_TRUE(action2->getAssessedActions().find(2) !=
                action2->getAssessedActions().end());
}

TEST_F(TPGTest, GraphOrderActionEdges)
{
    EvoGraph::Graph tpg;
    auto action = tpg.addNewAction(0);

    // Add several action edges with different actionClass values
    tpg.addNewActionEdge(*action, programAgent, 5);
    tpg.addNewActionEdge(*action, programAgent, 2);
    tpg.addNewActionEdge(*action, programAgent, 9);

    // Call orderActionEdges (should not throw)
    //ASSERT_NO_THROW(tpg.orderActionEdges(action));

    // Check that the outgoing edges are now ordered by actionClass
    std::vector<uint64_t> actionClasses;
    for (auto edge : action->getOutgoingEdges()) {
        auto actionEdge = std::dynamic_pointer_cast<const EvoGraph::ActionEdge>(edge);
        ASSERT_NE(actionEdge, nullptr);
        actionClasses.push_back(actionEdge->getActionClass());
    }
    ASSERT_TRUE(std::is_sorted(actionClasses.begin(), actionClasses.end()));

    // Try with an action not in the graph (should throw)
    auto fakeAction = std::make_shared<EvoGraph::Action>(99);
    //ASSERT_THROW(tpg.orderActionEdges(fakeAction), std::runtime_error);
}

TEST_F(TPGTest, GraphVertexID)
{
    EvoGraph::Graph tpg;
    auto team0 = tpg.addNewTeam();
    auto team1 = tpg.addNewTeam();
    auto action0 = tpg.addNewAction(0);

    ASSERT_EQ(team0->getVertexID(), 0) << "ID of vertex is incorrect.";
    ASSERT_EQ(team1->getVertexID(), 1) << "ID of vertex is incorrect.";
    ASSERT_EQ(action0->getVertexID(), 2) << "ID of vertex is incorrect.";
    ASSERT_EQ(EvoGraph::Vertex::getVertexIDCounter(), 3)
        << "ID counter is incorrect.";

    CounterReset::counterReset();

    ASSERT_EQ(EvoGraph::Vertex::getVertexIDCounter(), 0)
        << "ID counter is incorrect.";

    ASSERT_NO_THROW(tpg.setNewVertexID(*action0, 5))
        << "Setting a correct value for id should not throw";
    ASSERT_EQ(EvoGraph::Vertex::getVertexIDCounter(), 6)
        << "ID counter is incorrect.";

    ASSERT_THROW(tpg.setNewVertexID(*action0, 0), std::runtime_error)
        << "Setting an incorrect value for id should throw";

    EvoGraph::Team fakeTeam;
    ASSERT_NO_THROW(fakeTeam.setVertexID(10))
        << "Setting a correct value for id should not throw";
    ASSERT_EQ(EvoGraph::Vertex::getVertexIDCounter(), 11)
        << "ID counter is incorrect.";

    ASSERT_THROW(tpg.setNewVertexID(fakeTeam, 12), std::runtime_error)
        << "Setting a new ID for an inexisting vertex should throw";
}

TEST_F(TPGTest, GraphEdgeID)
{
    EvoGraph::Graph tpg;
    auto team0 = tpg.addNewTeam();
    auto team1 = tpg.addNewTeam();
    auto action0 = tpg.addNewAction(0);

    auto edge0 = tpg.addNewEdge(*team0, *team1, programAgent);
    auto edge1 = tpg.addNewEdge(*team1, *action0, programAgent);
    auto edge2 = tpg.addNewActionEdge(*action0, programAgent, 0);

    ASSERT_EQ(edge0->getEdgeID(), 0) << "ID of edge is incorrect.";
    ASSERT_EQ(edge1->getEdgeID(), 1) << "ID of edge is incorrect.";
    ASSERT_EQ(edge2->getEdgeID(), 2) << "ID of edge is incorrect.";
    ASSERT_EQ(EvoGraph::Edge::getEdgeIDCounter(), 3)
        << "ID counter is incorrect.";

    CounterReset::counterReset();

    ASSERT_EQ(EvoGraph::Edge::getEdgeIDCounter(), 0)
        << "ID counter is incorrect.";

    ASSERT_NO_THROW(tpg.setNewEdgeID(*edge1, 5))
        << "Setting a correct value for id should not throw";
    ASSERT_EQ(EvoGraph::Edge::getEdgeIDCounter(), 6)
        << "ID counter is incorrect.";

    ASSERT_THROW(tpg.setNewEdgeID(*edge2, 0), std::runtime_error)
        << "Setting an incorrect value for id should throw";

    EvoGraph::Edge fakeEdge(NULL, NULL, nullptr);
    ASSERT_NO_THROW(fakeEdge.setEdgeID(10))
        << "Setting a correct value for id should not throw";
    ASSERT_EQ(EvoGraph::Edge::getEdgeIDCounter(), 11)
        << "ID counter is incorrect.";

    ASSERT_THROW(tpg.setNewEdgeID(fakeEdge, 12), std::runtime_error)
        << "Setting a new ID for an inexisting vertex should throw";
}
