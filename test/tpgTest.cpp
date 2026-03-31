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
#include "parameters.h"

class TPGTest : public ::testing::Test
{
  protected:
    const size_t size1{24};
    const size_t size2{32};
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
    Instructions::Set set;
    std::shared_ptr<const Algorithm::LGP::LGPEnvironment> e = NULL;
    Parameters params;
    std::shared_ptr<Algorithm::Agent> sharedProgramAgent;

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

        params.algorithm.lgp.nbRegisters = 8;
        params.algorithm.lgp.nbProgramConstant = 1;
        e = std::make_shared<Algorithm::LGP::LGPEnvironment>(set, params.algorithm.lgp.nbRegisters, params.algorithm.lgp.nbProgramConstant, vect);
        sharedProgramAgent =
            std::make_shared<Algorithm::LGP::LGPAgent>(*e, Output::OutputHandler(1), (uint64_t)0);
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

    ASSERT_NO_THROW(edge = new EvoGraph::Edge(*team, *action, *sharedProgramAgent));

    ASSERT_NO_THROW(delete edge);
}

TEST_F(TPGTest, VertexEdgesSettersGetters)
{
    auto team = std::make_shared<EvoGraph::Team>();
    auto action = std::make_shared<EvoGraph::Action>(0);

    auto edge = std::make_shared<EvoGraph::Edge>(*team, *action, *sharedProgramAgent);

    ASSERT_NO_THROW(team->addOutgoingEdge(*edge))
        << "Adding an outgoing edge to a Team vertex failed.";
    ASSERT_THROW(action->addOutgoingEdge(*edge), std::runtime_error)
        << "Adding an outgoing edge to an Action vertex did not fail.";
    ASSERT_NO_THROW(action->addIncomingEdge(*edge))
        << "Adding an outgoing edge to an Action vertex failed.";

    ASSERT_EQ(team->getOutgoingEdges().size(), 1)
        << "Size of the outgoing edges of the node is incorrect.";
    ASSERT_EQ(std::count(team->getOutgoingEdges().begin(),
                         team->getOutgoingEdges().end(),
                         *edge),
              1)
        << "Edge pointer contained in the outgoingEdges is incorrect.";

    // Add the same edge again.. Nothing should happen, but it should not fail.
    ASSERT_NO_THROW(team->addOutgoingEdge(*edge))
        << "Adding an outgoing edge to a Team vertex, even though it is "
           "already there, failed unexpectedly.";
    ASSERT_EQ(team->getOutgoingEdges().size(), 1)
        << "Size of the outgoing edges of the node is incorrect.";

    ASSERT_EQ(team->getOutgoingEdges().size(), 1)
        << "Size of the outgoing edges of the node is incorrect.";

    ASSERT_EQ(action->getIncomingEdges().size(), 1)
        << "Size of the outgoing edges of the node is incorrect.";
    ASSERT_EQ(std::count(action->getIncomingEdges().begin(),
                         action->getIncomingEdges().end(), *edge),
              1)
        << "Edge pointer contained in the outgoingEdges is incorrect.";

    // Remove edges
    ASSERT_NO_THROW(team->removeOutgoingEdge(*edge))
        << "Removing an outgoing edge failed.";
    ASSERT_EQ(team->getOutgoingEdges().size(), 0)
        << "Size of the outgoing edges of the node is incorrect.";

    ASSERT_NO_THROW(action->removeIncomingEdge(*edge))
        << "Removing an outgoing edge to an Action vertex failed.";
    ASSERT_EQ(action->getIncomingEdges().size(), 0)
        << "Size of the outgoing edges of the node is incorrect.";

    ASSERT_NO_THROW(action->removeIncomingEdge(*edge))
        << "Removing an edge no longer in the set should do nothing, but not "
           "fail.";
}

TEST_F(TPGTest, EdgeGetSetProgram)
{
    auto team = std::make_shared<EvoGraph::Team>();
    auto action = std::make_shared<EvoGraph::Action>(0);

    std::shared_ptr<EvoGraph::Edge> constEdge = std::make_shared<EvoGraph::Edge>(*team, *action, *sharedProgramAgent);
    auto& constProg = constEdge->getProgram();
    ASSERT_EQ(constProg, *sharedProgramAgent)
        << "Program accessor on const Edge returns a Program different from "
           "the one given at construction.";

    // program is a mutable attribute of the Edge.
    std::shared_ptr<Algorithm::Agent> programAgent2 =
            std::make_shared<Algorithm::LGP::LGPAgent>(*e, Output::OutputHandler(1), 0);
    
    constEdge->setProgram(*programAgent2);
    ASSERT_EQ(constEdge->getProgram(), *programAgent2)
        << "Program accessor on Edge returns a Program different from the "
           "one set before.";
}

TEST_F(TPGTest, EdgeGetSetSourceAndDestination)
{
    EvoGraph::Team team0;
    EvoGraph::Team team1;
    EvoGraph::Action action0(0);
    EvoGraph::Action action1(1);

    auto edge = std::make_shared<EvoGraph::Edge>(team0, action0, *sharedProgramAgent);

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


TEST_F(TPGTest, GraphFactory)
{
    EvoGraph::GraphFactory factory;

    std::unique_ptr<EvoGraph::Action> action;
    std::unique_ptr<EvoGraph::Team> team;
    std::unique_ptr<EvoGraph::Edge> edge;

    ASSERT_NO_THROW(action = std::move(factory.createAction(0)))
        << "GraphELementFactory could not build a action->";
    ASSERT_NE(action, nullptr) << "Created Action should not be null.";

    ASSERT_NO_THROW(team = std::move(factory.createTeam()))
        << "GraphELementFactory could not build a team->";
    ASSERT_NE(team, nullptr) << "Created Team should not be null.";

    ASSERT_NO_THROW(
        edge = std::move(factory.createEdge(*team, *action, *sharedProgramAgent)))
        << "GraphELementFactory could not build a Edge.";
    ASSERT_NE(edge, nullptr) << "Created Edge should not be null.";

}

TEST_F(TPGTest, GraphAddVertex)
{
    EvoGraph::Graph tpg;
    ASSERT_NO_THROW(tpg.addNewTeam())
        << "Adding a new Team to a Graph failed.";
    ASSERT_NO_THROW(tpg.addNewAction(0))
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
    const EvoGraph::Team* team;
    ASSERT_NO_THROW(team = &tpg.addNewTeam())
        << "Adding a new Team to a Graph failed.";
    ASSERT_NO_THROW(tpg.addNewAction(0))
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
    const EvoGraph::Team* vertex = &tpg.addNewTeam();
    const std::vector<std::reference_wrapper<const EvoGraph::Vertex>> vertices = tpg.getVertices();
    ASSERT_EQ(vertices.size(), 1)
        << "Size of the retrievd std::vector<Vertex> is incorrect.";
    ASSERT_EQ(vertices.front(), *vertex)
        << "Vertex in the retrieved vertices list does not correspond to the "
           "one added to the TPGGrapg (pointer comparison)";
}

TEST_F(TPGTest, GraphAddEdge)
{
    EvoGraph::Graph tpg;
    const EvoGraph::Team & vertex0 = tpg.addNewTeam();
    const EvoGraph::Action & vertex1 = tpg.addNewAction(0);

    ASSERT_NO_THROW(tpg.addNewEdge(vertex0, vertex1, *sharedProgramAgent))
        << "Adding an edge between a team and an action failed.";
    // Add with a vertex not in the graph.
    EvoGraph::Action vertex2(2);
    ASSERT_THROW(tpg.addNewEdge(vertex0, vertex2, *sharedProgramAgent),
                 std::runtime_error)
        << "Adding an edge with a vertex not from the graph should have "
           "failed.";

    // Add the edge from the action
    ASSERT_THROW(tpg.addNewEdge(vertex1, vertex0, *sharedProgramAgent),
                 std::runtime_error)
        << "Adding an edge from an Action should have failed.";

}


TEST_F(TPGTest, GraphGetEdges)
{
    EvoGraph::Graph tpg;
    const EvoGraph::Team & vertex0 = tpg.addNewTeam();;
    const EvoGraph::Action & vertex1 = tpg.addNewAction(0);

    const EvoGraph::Edge& edge = tpg.addNewEdge(vertex0, vertex1, *sharedProgramAgent);
    ASSERT_EQ(tpg.getEdges().size(), 1)
        << "Edges of the graph have incorrect size after successful add.";

    // Check that connection were added
    // To the source
    ASSERT_EQ(vertex0.getOutgoingEdges().size(), 1);
    ASSERT_EQ(std::count_if(vertex0.getOutgoingEdges().begin(),
                            vertex0.getOutgoingEdges().end(),
                            [&edge](const EvoGraph::Edge& other) {
                                return other == edge;
                            }),
              1);

    // To the destination
    ASSERT_EQ(vertex1.getIncomingEdges().size(), 1);
    ASSERT_EQ(std::count_if(vertex1.getIncomingEdges().begin(),
                            vertex1.getIncomingEdges().end(),
                            [&edge](const EvoGraph::Edge& other) {
                                return other == edge;
                            }),
              1);

    // Attempt an impossible add.
    ASSERT_THROW(tpg.addNewEdge(vertex1, vertex0, *sharedProgramAgent),
                 std::runtime_error)
        << "An exception should be thrown when adding an impossible edge.";

    ASSERT_EQ(tpg.getEdges().size(), 1)
        << "Edges of the graph have incorrect size after unsuccessful add.";
}

TEST_F(TPGTest, GraphRemoveEdge)
{
    EvoGraph::Graph tpg;
    const EvoGraph::Team & vertex0 = tpg.addNewTeam();;
    const EvoGraph::Action & vertex1 = tpg.addNewAction(0);

    const auto& edge = tpg.addNewEdge(vertex0, vertex1, *sharedProgramAgent);


    // Remove the edge
    ASSERT_NO_THROW(tpg.removeEdge(edge))
        << "Edge from the graph could not be removed successfully.";
    // Check that the edge is no longer in the graph
    ASSERT_EQ(tpg.getEdges().size(), 0)
        << "Edge was not effectively removed from the graph.";
    // Check that vertices were disconnected from the removed edge.
    ASSERT_EQ(vertex0.getOutgoingEdges().size(), 0)
        << "Source vertex was not disconnected from the removed Edge.";
    // and from the destination
    ASSERT_EQ(vertex1.getIncomingEdges().size(), 0)
        << "Destination vertex was not disconnected from the removed Edge.";
    // Check that the edge was successfully deleted
    ASSERT_EQ(sharedProgramAgent.use_count(), 1)
        << "Edge was not properly deleted, its shared pointer is still active.";
    // Remove an edge that does not exist anymore
    ASSERT_THROW(tpg.removeEdge(edge), std::runtime_error)
        << "Edge not in the graph should not be removable";
}

TEST_F(TPGTest, GraphRemoveVertex)
{
    EvoGraph::Graph tpg;
    const EvoGraph::Team & vertex0 = tpg.addNewTeam();;
    const EvoGraph::Action & vertex1 = tpg.addNewAction(0);
    const EvoGraph::Team & vertex2 = tpg.addNewTeam();

    ASSERT_NO_THROW(tpg.removeVertex(vertex0))
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
    tpg.addNewEdge(vertex2, vertex1, *sharedProgramAgent);
    ASSERT_NO_THROW(tpg.removeVertex(vertex2))
        << "Removing a vertex from the graph failed.";
    // Check that edge was removed from the graph
    ASSERT_EQ(tpg.getEdges().size(), 0)
        << "Edge connected to the removed vertex was not removed from the "
           "graph.";
    // And disconnected from vertex1
    ASSERT_EQ(vertex1.getIncomingEdges().size(), 0)
        << "Edge connected to the vertex removed from the graph was not "
           "disconnected from its destination.";

    // For code coverage, test when the destination vertex of an edge is removed
    // Add a new edge to test removal of vertex connectet to an edge.
    const EvoGraph::Team & vertex4 = tpg.addNewTeam();
    tpg.addNewEdge(vertex4, vertex1, *sharedProgramAgent);
    ASSERT_NO_THROW(tpg.removeVertex(vertex1))
        << "Removing a vertex from the graph failed.";
    // Check that edge was removed from the graph
    ASSERT_EQ(tpg.getEdges().size(), 0)
        << "Edge connected to the removed vertex was not removed from the "
           "graph.";
    // And disconnected from vertex1
    ASSERT_EQ(vertex4.getOutgoingEdges().size(), 0)
        << "Edge connected to the vertex removed from the graph was not "
           "disconnected from its destination.";
}

TEST_F(TPGTest, GraphClear)
{
    EvoGraph::Graph tpg;
    const EvoGraph::Team & vertex0 = tpg.addNewTeam();;
    const EvoGraph::Action & vertex1 = tpg.addNewAction(0);
    const EvoGraph::Edge& edge = tpg.addNewEdge(vertex0, vertex1, *sharedProgramAgent);

    ASSERT_NO_THROW(tpg.clear()) << "Clearing a non empty graph failed.";
    ASSERT_EQ(tpg.getNbVertices(), 0)
        << "Cleared graph is not empty of vertices as expected.";
    ASSERT_EQ(tpg.getEdges().size(), 0)
        << "Cleared graph is not empty of edges as expected.";
}

TEST_F(TPGTest, GraphGetNbRootVertices)
{
    EvoGraph::Graph tpg;
    const EvoGraph::Team & vertex0 = tpg.addNewTeam();;
    const EvoGraph::Action & vertex1 = tpg.addNewAction(0);
    const EvoGraph::Edge& edge = tpg.addNewEdge(vertex0, vertex1, *sharedProgramAgent);

    ASSERT_EQ(tpg.getNbRootVertices(), 1)
        << "Number of roots of the TPG is incorrect.";
}

TEST_F(TPGTest, GraphGetRootVertices)
{
    EvoGraph::Graph tpg;
    const EvoGraph::Team & vertex0 = tpg.addNewTeam();;
    const EvoGraph::Action & vertex1 = tpg.addNewAction(0);

    const EvoGraph::Edge& edge = tpg.addNewEdge(vertex0, vertex1, *sharedProgramAgent);
    ASSERT_EQ(tpg.getRootVertices().size(), 1)
        << "Number of roots of the TPG is incorrect.";
    ASSERT_EQ(tpg.getRootVertices().at(0), vertex0)
        << "Vertex classified as root is incorrect.";

    const EvoGraph::Team & vertex2 = tpg.addNewTeam();
    const EvoGraph::Action & vertex3 = tpg.addNewAction(1);
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
    const EvoGraph::Team & vertex0 = tpg.addNewTeam();;
    const EvoGraph::Action & vertex1 = tpg.addNewAction(4);

    const auto& edge0 = tpg.addNewEdge(vertex0, vertex1, *sharedProgramAgent);

    // Clone the team
    const EvoGraph::Vertex* cloneVertex;
    ASSERT_NO_THROW(cloneVertex = &tpg.cloneVertex(vertex0))
        << "Cloning a TeamVertex of the Graph failed.";
    // Check that the clone vertex is in the graph
    ASSERT_EQ(tpg.getNbVertices(), 3)
        << "Number of vertices of the graph after clone is incorrect.";
    ASSERT_EQ(tpg.getVertices().at(2), *cloneVertex)
        << "CloneVertex is not the last of the graph vertices as it should be.";
    cloneVertex = &tpg.getVertices().at(2).get(); // to remove a compilation warning.
    // Check that the type is correct
    ASSERT_EQ(typeid(&vertex0), typeid(dynamic_cast<const EvoGraph::Team*>(cloneVertex)));
    ASSERT_EQ(tpg.getEdges().size(), 2)
        << "Number of edges of the graph after clone is incorrect.";
    const EvoGraph::Vertex& destinationVertex =
        ((*cloneVertex->getOutgoingEdges().begin()).get().getDestination());
    ASSERT_EQ(destinationVertex, vertex1)
        << "Cloned vertex is not connected to the correct other vertex in the "
           "Graph.";

    // Duplicate the action (to increase code coverage)
    ASSERT_NO_THROW(tpg.cloneVertex(vertex1));
    // Check that the type is correct
    auto vertex = tpg.getVertices().at(3);
    ASSERT_EQ(typeid(vertex1).name(), typeid(vertex.get()).name());
    ASSERT_EQ(vertex1.getActionID(),
              dynamic_cast<const EvoGraph::Action&>(tpg.getVertices().at(3).get()).getActionID());
    ASSERT_EQ(tpg.getEdges().size(), 2)
        << "Number of edges of the graph after clone is incorrect.";

    // Clone a vertex not from the graph
    EvoGraph::Vertex* vertex2 = new EvoGraph::Action(1);
    ASSERT_THROW(tpg.cloneVertex(*vertex2), std::runtime_error)
        << "Cloning a vertex that does not belong to the Graph should not "
           "be possible.";
}

TEST_F(TPGTest, GraphCloneEdge)
{
    EvoGraph::Graph tpg;
    const EvoGraph::Team & vertex0 = tpg.addNewTeam();;
    auto& vertex1 = tpg.addNewAction(4);
    const EvoGraph::Edge& edge = tpg.addNewEdge(vertex0, vertex1, *sharedProgramAgent);

    const EvoGraph::Edge* clone;
    ASSERT_NO_THROW(clone = &tpg.cloneEdge(edge))
        << "Cloning an existing edge failed.";
    // Check that the new edge is correctly added to the graph
    ASSERT_EQ(tpg.getEdges().size(), 2)
        << "Incorrect number of edges in the graph after clone.";

    // Check the edge source and destination
    ASSERT_EQ(clone->getSource(), vertex0)
        << "Clone edge has an incorrect source.";
    ASSERT_EQ(clone->getDestination(), vertex1)
        << "Clone edge has an incorrect destination.";
    // Check that the edge was correctly registered
    ASSERT_EQ(std::count_if(vertex0.getOutgoingEdges().begin(),
                            vertex0.getOutgoingEdges().end(),
                            [clone](const EvoGraph::Edge& other) {
                                return other == *clone;
                            }),
              1)
        << "Clone edge is not registered within its source vertex outgoing "
           "edges.";
    ASSERT_EQ(std::count_if(vertex1.getIncomingEdges().begin(),
                            vertex1.getIncomingEdges().end(),
                            [clone](const EvoGraph::Edge& other) {
                                return other == *clone;
                            }),
              1)
        << "Clone edge is not registered within its destination vertex "
           "incoming edges.";


    // Check throw behavior
    EvoGraph::Edge newEdge(vertex0, vertex1, *sharedProgramAgent);
    ASSERT_THROW(tpg.cloneEdge(newEdge), std::runtime_error)
        << "Cloning an edge not from the graph should not succeed.";

}

TEST_F(TPGTest, GraphSetEdgeDestination)
{
    EvoGraph::Graph tpg;
    const EvoGraph::Team & vertex0 = tpg.addNewTeam();;
    auto& vertex1 = tpg.addNewAction(4);
    auto& vertex2 = tpg.addNewAction(4);
    const EvoGraph::Edge& edge = tpg.addNewEdge(vertex0, vertex1, *sharedProgramAgent);

    // Change the destination of the edge
    ASSERT_TRUE(tpg.setEdgeDestination(edge, vertex2))
        << "Changing the destination of an Edge to a valid new destination "
           "should not fail.";
    // Check the graph size
    ASSERT_EQ(tpg.getEdges().size(), 1)
        << "Incorrect number of edges in the graph after edge setDestination.";
    // Check the edge source and destination
    ASSERT_EQ(edge.getSource(), vertex0)
        << "Updated edge has an incorrect source.";
    ASSERT_EQ(edge.getDestination(), vertex2)
        << "Updated edge has an incorrect destination.";
    // Check that the edge was correctly registered
    ASSERT_EQ(std::count_if(vertex0.getOutgoingEdges().begin(),
                            vertex0.getOutgoingEdges().end(),
                            [&edge](const EvoGraph::Edge& other) {
                                return other == edge;
                            }),
              1)
        << "Updated edge is no longer registered within its source vertex "
           "outgoing edges.";
    ASSERT_EQ(std::count_if(vertex2.getIncomingEdges().begin(),
                            vertex2.getIncomingEdges().end(),
                            [&edge](const EvoGraph::Edge& other) {
                                return other == edge;
                            }),
              1)
        << "Updated edge is not registered within its new destination vertex "
           "incoming edges.";
    // Check that the edge was unregistered
    ASSERT_EQ(vertex1.getIncomingEdges().size(), 0)
        << "This vertex should not have incomingEdge after destination change.";

    // Check failure
    EvoGraph::Edge newEdge(vertex0, vertex1, *sharedProgramAgent);
    ASSERT_FALSE(tpg.setEdgeDestination(newEdge, vertex2))
        << "Changing destination of an edge not within the graph should not "
           "succeed.";
}

TEST_F(TPGTest, GraphSetEdgeSource)
{
    EvoGraph::Graph tpg;
    const EvoGraph::Team & vertex0 = tpg.addNewTeam();;
    auto& vertex1 = tpg.addNewAction(4);
    auto& vertex2 = tpg.addNewTeam();
    const EvoGraph::Edge& edge = tpg.addNewEdge(vertex0, vertex1, *sharedProgramAgent);

    // Change the destination of the edge
    ASSERT_TRUE(tpg.setEdgeSource(edge, vertex2))
        << "Changing the destination of an Edge to a valid new destination "
           "should not fail.";
    // Check the graph size
    ASSERT_EQ(tpg.getEdges().size(), 1)
        << "Incorrect number of edges in the graph after edge setDestination.";
    // Check the edge source and destination
    ASSERT_EQ(edge.getSource(), vertex2)
        << "Updated edge has an incorrect source.";
    ASSERT_EQ(edge.getDestination(), vertex1)
        << "Updated edge has an incorrect destination.";
    // Check that the edge was correctly registered
    ASSERT_EQ(std::count_if(vertex2.getOutgoingEdges().begin(),
                            vertex2.getOutgoingEdges().end(),
                            [&edge](const EvoGraph::Edge& other) {
                                return other == edge;
                            }),
              1)
        << "Updated edge is no longer registered within its new source vertex "
           "outgoing edges.";
    ASSERT_EQ(std::count_if(vertex1.getIncomingEdges().begin(),
                            vertex1.getIncomingEdges().end(),
                            [&edge](const EvoGraph::Edge& other) {
                                return other == edge;
                            }),
              1)
        << "Updated edge is not registered within its destination vertex "
           "incoming edges.";
    // Check that the edge was unregistered
    ASSERT_EQ(vertex0.getOutgoingEdges().size(), 0)
        << "This vertex should not have incomingEdge after source change.";

    // Check failure
    EvoGraph::Edge newEdge(vertex0, vertex1, *sharedProgramAgent);
    ASSERT_FALSE(tpg.setEdgeSource(newEdge, vertex2))
        << "Changing source of an edge not within the graph should not "
           "succeed.";
}

TEST_F(TPGTest, TPGMoveOperator)
{
    EvoGraph::Graph source;
    EvoGraph::Graph* destination =
        new EvoGraph::Graph(); // creates an empty tpg graph

    auto& vertex0 = source.addNewTeam();
    auto& vertex1 = source.addNewAction(4);
    auto& vertex2 = source.addNewTeam();
    auto& edge = source.addNewEdge(vertex0, vertex1, *sharedProgramAgent);
    auto& edge2 =
        source.addNewEdge(vertex2, vertex1, *sharedProgramAgent);
    auto& edge3 =
        source.addNewEdge(vertex0, vertex2, *sharedProgramAgent);

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

TEST_F(TPGTest, VertexHasSameAssessedActions)
{
    EvoGraph::Graph tpg;
    auto& team = tpg.addNewTeam();
    auto& action1 = tpg.addNewAction(1);
    auto& action2 = tpg.addNewAction(2);
    auto& action3 = tpg.addNewAction(3);
    tpg.addNewEdge(team, action1, *sharedProgramAgent);
    tpg.addNewEdge(team, action2, *sharedProgramAgent);
    tpg.addNewEdge(team, action3, *sharedProgramAgent);
    tpg.updateAssessedActions(team);

    // Case 1: Intersection is not empty (should return true)
    std::set<uint64_t> testSet1 = {2, 4, 5};
    ASSERT_TRUE(team.hasSameAssessedActions(testSet1))
        << "hasSameAssessedActions should return true when intersection is not "
           "empty.";

    // Case 2: Intersection is empty (should return false)
    std::set<uint64_t> testSet2 = {4, 5, 6};
    ASSERT_FALSE(team.hasSameAssessedActions(testSet2))
        << "hasSameAssessedActions should return false when intersection is "
           "empty.";

    // Case 3: Exact match (should return true)
    std::set<uint64_t> testSet3 = {1, 2, 3};
    ASSERT_TRUE(team.hasSameAssessedActions(testSet3))
        << "hasSameAssessedActions should return true when sets are identical.";

    // Case 4: Partial overlap (should return true)
    std::set<uint64_t> testSet4 = {3};
    ASSERT_TRUE(team.hasSameAssessedActions(testSet4))
        << "hasSameAssessedActions should return true when there is a single "
           "common element.";

    // Case 5: Empty input set (should return false)
    std::set<uint64_t> testSet5;
    ASSERT_FALSE(team.hasSameAssessedActions(testSet5))
        << "hasSameAssessedActions should return false when input set is "
           "empty.";

    // Case 6: Empty assessedActions (should return false)
    EvoGraph::Team emptyTeam;
    std::set<uint64_t> testSet6 = {1, 2};
    ASSERT_FALSE(emptyTeam.hasSameAssessedActions(testSet6))
        << "hasSameAssessedActions should return false when assessedActions is "
           "empty.";
}


TEST_F(TPGTest, GraphUpdateAssessedActions)
{
    EvoGraph::Graph tpg;
    auto& team1 = tpg.addNewTeam(); 
    auto& action1 = tpg.addNewAction(1);
    auto& action2 = tpg.addNewAction(2);
    tpg.addNewEdge(team1, action1, *sharedProgramAgent);
    tpg.addNewEdge(team1, action2, *sharedProgramAgent);

    // Should update without throwing
    ASSERT_NO_THROW(tpg.updateAssessedActions(team1));
    // Check that assessedActions contains the correct action classes: 1 and 2
    const auto& assessed = team1.getAssessedActions();
    ASSERT_TRUE(assessed.find(1) != assessed.end());
    ASSERT_TRUE(assessed.find(2) != assessed.end());
    ASSERT_FALSE(assessed.find(0) != assessed.end());

    auto& team2 = tpg.addNewTeam(); 
    auto& action3 = tpg.addNewAction(1);
    auto& action4 = tpg.addNewAction(3);
    tpg.addNewEdge(team2, action3, *sharedProgramAgent);
    tpg.addNewEdge(team2, action4, *sharedProgramAgent);

    // Vertex should contain action 1, 2 and 3 now.
    auto& vertex = tpg.addNewTeam();
    tpg.addNewEdge(vertex, team1, *sharedProgramAgent);
    tpg.addNewEdge(vertex, team2, *sharedProgramAgent);

    // Should update without throwing
    ASSERT_NO_THROW(tpg.updateAssessedActions(team2));
    // Check that assessedActions contains the correct action classes
    const auto& assessed2 = vertex.getAssessedActions();
    ASSERT_TRUE(assessed2.find(1) != assessed2.end());
    ASSERT_TRUE(assessed2.find(2) != assessed2.end());
    ASSERT_TRUE(assessed2.find(3) != assessed2.end());

    // Try with a vertex not in the graph (should throw)
    auto fakeAction = std::make_shared<EvoGraph::Action>(99);
    ASSERT_THROW(tpg.updateAssessedActions(*fakeAction), std::runtime_error);
}

TEST_F(TPGTest, GraphUpdateAllAssessedActions)
{
    EvoGraph::Graph tpg;
    auto& team1 = tpg.addNewTeam(); 
    auto& action1 = tpg.addNewAction(1);
    auto& action2 = tpg.addNewAction(2);
    tpg.addNewEdge(team1, action1, *sharedProgramAgent);
    tpg.addNewEdge(team1, action2, *sharedProgramAgent);

    // Should update all actions without throwing
    ASSERT_NO_THROW(tpg.updateAllAssessedActions());
    // Check that both actions have their assessedActions updated
    ASSERT_TRUE(team1.getAssessedActions().find(1) !=
                team1.getAssessedActions().end());
    ASSERT_TRUE(team1.getAssessedActions().find(2) !=
                team1.getAssessedActions().end());
}

TEST_F(TPGTest, GraphOrderActionEdges)
{
    EvoGraph::Graph tpg;
    auto& team1 = tpg.addNewTeam(); 
    auto& action1 = tpg.addNewAction(5);
    auto& action2 = tpg.addNewAction(2);
    auto& action3 = tpg.addNewAction(9);
    tpg.addNewEdge(team1, action1, *sharedProgramAgent);
    tpg.addNewEdge(team1, action2, *sharedProgramAgent);
    tpg.addNewEdge(team1, action3, *sharedProgramAgent);

    // Call orderActionEdges (should not throw)
    ASSERT_NO_THROW(tpg.orderActionEdges(team1));

    // Check that the outgoing edges are now ordered by actionClass
    std::vector<uint64_t> actionClasses;
    for (auto& edge : team1.getOutgoingEdges()) {
        auto action = dynamic_cast<const EvoGraph::Action*>(&edge.get().getDestination());
        ASSERT_NE(action, nullptr);
        actionClasses.push_back(action->getActionID());
    }
    ASSERT_TRUE(std::is_sorted(actionClasses.begin(), actionClasses.end()));

    // Try with an action not in the graph (should throw)
    auto fakeTeam = std::make_shared<EvoGraph::Team>();
    ASSERT_THROW(tpg.orderActionEdges(*fakeTeam), std::runtime_error);
}

TEST_F(TPGTest, GraphVertexID)
{
    EvoGraph::Graph tpg;
    auto& team0 = tpg.addNewTeam();
    auto& team1 = tpg.addNewTeam();
    auto& action0 = tpg.addNewAction(0);

    ASSERT_EQ(team0.getVertexID(), 0) << "ID of vertex is incorrect.";
    ASSERT_EQ(team1.getVertexID(), 1) << "ID of vertex is incorrect.";
    ASSERT_EQ(action0.getVertexID(), 2) << "ID of vertex is incorrect.";
    ASSERT_EQ(EvoGraph::Vertex::getVertexIDCounter(), 3)
        << "ID counter is incorrect.";

    CounterReset::counterReset();

    ASSERT_EQ(EvoGraph::Vertex::getVertexIDCounter(), 0)
        << "ID counter is incorrect.";

    ASSERT_NO_THROW(tpg.setNewVertexID(action0, 5))
        << "Setting a correct value for id should not throw";
    ASSERT_EQ(EvoGraph::Vertex::getVertexIDCounter(), 6)
        << "ID counter is incorrect.";

    ASSERT_THROW(tpg.setNewVertexID(action0, 0), std::runtime_error)
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
    auto& team0 = tpg.addNewTeam();
    auto& team1 = tpg.addNewTeam();
    auto& action0 = tpg.addNewAction(0);

    auto& edge0 = tpg.addNewEdge(team0, team1, *sharedProgramAgent);
    auto& edge1 = tpg.addNewEdge(team1, action0, *sharedProgramAgent);
    auto& edge2 = tpg.addNewEdge(team1, action0, *sharedProgramAgent);

    ASSERT_EQ(edge0.getEdgeID(), 0) << "ID of edge is incorrect.";
    ASSERT_EQ(edge1.getEdgeID(), 1) << "ID of edge is incorrect.";
    ASSERT_EQ(edge2.getEdgeID(), 2) << "ID of edge is incorrect.";
    ASSERT_EQ(EvoGraph::Edge::getEdgeIDCounter(), 3)
        << "ID counter is incorrect.";

    CounterReset::counterReset();

    ASSERT_EQ(EvoGraph::Edge::getEdgeIDCounter(), 0)
        << "ID counter is incorrect.";

    ASSERT_NO_THROW(tpg.setNewEdgeID(edge1, 5))
        << "Setting a correct value for id should not throw";
    ASSERT_EQ(EvoGraph::Edge::getEdgeIDCounter(), 6)
        << "ID counter is incorrect.";

    ASSERT_THROW(tpg.setNewEdgeID(edge2, 0), std::runtime_error)
        << "Setting an incorrect value for id should throw";

    EvoGraph::Team fakeTeam1;
    EvoGraph::Team fakeTeam2;
    EvoGraph::Edge fakeEdge(fakeTeam1, fakeTeam2, std::nullopt);
    ASSERT_NO_THROW(fakeEdge.setEdgeID(10))
        << "Setting a correct value for id should not throw";
    ASSERT_EQ(EvoGraph::Edge::getEdgeIDCounter(), 11)
        << "ID counter is incorrect.";

    ASSERT_THROW(tpg.setNewEdgeID(fakeEdge, 12), std::runtime_error)
        << "Setting a new ID for an inexisting vertex should throw";
}
