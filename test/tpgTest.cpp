/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019)
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

#include "data/dataHandler.h"
#include "data/primitiveTypeArray.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/lambdaInstruction.h"
#include "program/program.h"

#include "tpg/tpgAction.h"
#include "tpg/tpgEdge.h"
#include "tpg/tpgGraph.h"
#include "tpg/tpgTeam.h"
#include "tpg/tpgVertex.h"

class TPGTest : public ::testing::Test
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
        auto minus = [](double a, double b)->double {return a - b; };
        set.add(*(new Instructions::LambdaInstruction<double,double>(minus)));
        
        e = new Environment(set, vect, 8);
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

TEST_F(TPGTest, TPGTeamAndTPGActionConstructorsDestructors)
{
    TPG::TPGVertex* team;
    TPG::TPGVertex* action;

    ASSERT_NO_THROW(team = new TPG::TPGTeam());
    ASSERT_NO_THROW(action = new TPG::TPGAction(0));

    ASSERT_NO_THROW(delete team);
    ASSERT_NO_THROW(delete action);
}

TEST_F(TPGTest, TPGEdgeConstructorDestructor)
{
    TPG::TPGTeam team;
    TPG::TPGAction action(0);

    TPG::TPGEdge* edge;

    ASSERT_NO_THROW(edge = new TPG::TPGEdge(&team, &action, progPointer));

    ASSERT_NO_THROW(delete edge);
}

TEST_F(TPGTest, TPGVertexEdgesSettersGetters)
{
    TPG::TPGTeam team;
    TPG::TPGAction action(0);

    TPG::TPGEdge edge(&team, &action, progPointer);

    ASSERT_NO_THROW(team.addOutgoingEdge(&edge))
        << "Adding an outgoing edge to a Team vertex failed.";
    ASSERT_THROW(action.addOutgoingEdge(&edge), std::runtime_error)
        << "Adding an outgoing edge to an Action vertex did not fail.";
    ASSERT_NO_THROW(action.addIncomingEdge(&edge))
        << "Adding an outgoing edge to an Action vertex failed.";

    ASSERT_EQ(team.getOutgoingEdges().size(), 1)
        << "Size of the outgoing edges of the node is incorrect.";
    ASSERT_EQ(std::count(team.getOutgoingEdges().begin(),
                         team.getOutgoingEdges().end(),
                         (const TPG::TPGEdge*)&edge),
              1)
        << "TPGEdge pointer contained in the outgoingEdges is incorrect.";

    // Add the same edge again.. Nothing should happen, but it should not fail.
    ASSERT_NO_THROW(team.addOutgoingEdge(&edge))
        << "Adding an outgoing edge to a Team vertex, even though it is "
           "already there, failed unexpectedly.";
    ASSERT_EQ(team.getOutgoingEdges().size(), 1)
        << "Size of the outgoing edges of the node is incorrect.";

    // Add Null pointer: Nothing should happen, but it should not fail.
    ASSERT_NO_THROW(team.addOutgoingEdge(NULL))
        << "Adding an outgoing edge to a Team vertex, even though it is "
           "already there, failed unexpectedly.";
    ASSERT_EQ(team.getOutgoingEdges().size(), 1)
        << "Size of the outgoing edges of the node is incorrect.";

    ASSERT_EQ(action.getIncomingEdges().size(), 1)
        << "Size of the outgoing edges of the node is incorrect.";
    ASSERT_EQ(std::count(action.getIncomingEdges().begin(),
                         action.getIncomingEdges().end(), &edge),
              1)
        << "TPGEdge pointer contained in the outgoingEdges is incorrect.";

    // Remove edges
    ASSERT_NO_THROW(team.removeOutgoingEdge(&edge))
        << "Removing an outgoing edge failed.";
    ASSERT_EQ(team.getOutgoingEdges().size(), 0)
        << "Size of the outgoing edges of the node is incorrect.";

    ASSERT_NO_THROW(action.removeIncomingEdge(&edge))
        << "Removing an outgoing edge to an Action vertex failed.";
    ASSERT_EQ(action.getIncomingEdges().size(), 0)
        << "Size of the outgoing edges of the node is incorrect.";

    // Remove Null or non existing edge.
    ASSERT_NO_THROW(team.removeOutgoingEdge(NULL))
        << "Removing an NULL edge failed, while it should just do nothing.";
    ASSERT_NO_THROW(action.removeIncomingEdge(&edge))
        << "Removing an edge no longer in the set should do nothing, but not "
           "fail.";
}

TEST_F(TPGTest, TPGEdgeGetSetProgram)
{
    TPG::TPGTeam team;
    TPG::TPGAction action(0);

    const TPG::TPGEdge constEdge(&team, &action, progPointer);
    const Program::Program& constProg = constEdge.getProgram();
    ASSERT_EQ(&constProg, progPointer.get())
        << "Program accessor on const TPGEdge returns a Program different from "
           "the one given at construction.";

    // program is a mutable attribute of the Edge.
    std::shared_ptr<Program::Program> progPointer2(new Program::Program(*e));
    constEdge.setProgram(progPointer2);
    ASSERT_EQ(&constEdge.getProgram(), progPointer2.get())
        << "Program accessor on TPGEdge returns a Program different from the "
           "one set before.";
}

TEST_F(TPGTest, TPGEdgeGetSetSourceAndDestination)
{
    TPG::TPGTeam team0, team1;
    TPG::TPGAction action0(0), action1(1);

    TPG::TPGEdge edge(&team0, &action0, progPointer);

    ASSERT_EQ(&team0, edge.getSource())
        << "Source of the TPGEdge differs from the one given at construction.";
    ASSERT_EQ(&action0, edge.getDestination())
        << "Source of the TPGEdge differs from the one given at construction.";

    edge.setSource(&team1);
    ASSERT_EQ(&team1, edge.getSource())
        << "Source of the TPGEdge differs from the one set right before.";

    edge.setDestination(&action1);
    ASSERT_EQ(&action1, edge.getDestination())
        << "Destination of the TPGEdge differs from the one set right before.";
}

TEST_F(TPGTest, TPGGraphAddTPGVertex)
{
    TPG::TPGGraph tpg(*e);
    const TPG::TPGTeam* t;
    const TPG::TPGAction* a;
    ASSERT_NO_THROW(t = &tpg.addNewTeam())
        << "Adding a new Team to a TPGGraph failed.";
    ASSERT_NO_THROW(a = &tpg.addNewAction(0))
        << "Adding a new Action to a TPGGraph failed.";
}

TEST_F(TPGTest, TPGGraphHasVertex)
{
    TPG::TPGGraph tpg(*e);
    const TPG::TPGTeam* t;
    const TPG::TPGAction* a;
    ASSERT_NO_THROW(t = &tpg.addNewTeam())
        << "Adding a new Team to a TPGGraph failed.";
    ASSERT_NO_THROW(a = &tpg.addNewAction(0))
        << "Adding a new Action to a TPGGraph failed.";

    TPG::TPGAction external(12);

    ASSERT_TRUE(tpg.hasVertex(*t))
        << "A TPGVertex from the TPGGraph was not detected as such.";
    ASSERT_FALSE(tpg.hasVertex(external))
        << "A TPGVertex from the TPGGraph was wrongfully detected as such.";
}

TEST_F(TPGTest, TPGGraphGetNbVertices)
{
    TPG::TPGGraph tpg(*e);
    tpg.addNewTeam();
    tpg.addNewAction(0);
    ASSERT_EQ(tpg.getNbVertices(), 2)
        << "Number of vertices in the TPGGraph is incorrect.";
}

TEST_F(TPGTest, TPGGraphGetVertices)
{
    TPG::TPGGraph tpg(*e);
    const TPG::TPGVertex& vertex = tpg.addNewTeam();
    const std::vector<const TPG::TPGVertex*> vertices = tpg.getVertices();
    ASSERT_EQ(vertices.size(), 1)
        << "Size of the retrievd std::vector<TPGVertex> is incorrect.";
    ASSERT_EQ(vertices.front(), &vertex)
        << "Vertex in the retrieved vertices list does not correspond to the "
           "one added to the TPGGrapg (pointer comparison)";
}

TEST_F(TPGTest, TPGGraphAddEdge)
{
    TPG::TPGGraph tpg(*e);
    const TPG::TPGVertex& vertex0 = tpg.addNewTeam();
    const TPG::TPGAction& vertex1 = tpg.addNewAction(0);

    ASSERT_NO_THROW(tpg.addNewEdge(vertex0, vertex1, progPointer))
        << "Adding an edge between a team and an action failed.";
    // Add with a vertex not in the graph.
    TPG::TPGAction vertex2(2);
    ASSERT_THROW(tpg.addNewEdge(vertex0, vertex2, progPointer),
                 std::runtime_error)
        << "Adding an edge with a vertex not from the graph should have "
           "failed.";

    // Add the edge from the action
    ASSERT_THROW(tpg.addNewEdge(vertex1, vertex0, progPointer),
                 std::runtime_error)
        << "Adding an edge from an Action should have failed.";
}

TEST_F(TPGTest, TPGGraphGetEdges)
{
    TPG::TPGGraph tpg(*e);
    const TPG::TPGVertex& vertex0 = tpg.addNewTeam();
    const TPG::TPGAction& vertex1 = tpg.addNewAction(0);

    const TPG::TPGEdge& edge = tpg.addNewEdge(vertex0, vertex1, progPointer);
    ASSERT_EQ(tpg.getEdges().size(), 1)
        << "Edges of the graph have incorrect size after successful add.";

    // Check that connection were added
    // To the source
    ASSERT_EQ(vertex0.getOutgoingEdges().size(), 1);
    ASSERT_EQ(std::count_if(vertex0.getOutgoingEdges().begin(),
                            vertex0.getOutgoingEdges().end(),
                            [&edge](const TPG::TPGEdge* other) {
                                return other == &edge;
                            }),
              1);

    // To the destination
    ASSERT_EQ(vertex1.getIncomingEdges().size(), 1);
    ASSERT_EQ(std::count_if(vertex1.getIncomingEdges().begin(),
                            vertex1.getIncomingEdges().end(),
                            [&edge](const TPG::TPGEdge* other) {
                                return other == &edge;
                            }),
              1);

    // Attempt an impossible add.
    ASSERT_THROW(tpg.addNewEdge(vertex1, vertex0, progPointer),
                 std::runtime_error)
        << "An exception should be thrown when adding an impossible edge.";

    ASSERT_EQ(tpg.getEdges().size(), 1)
        << "Edges of the graph have incorrect size after unsuccessful add.";
}

TEST_F(TPGTest, TPGGraphRemoveEdge)
{
    TPG::TPGGraph tpg(*e);
    const TPG::TPGVertex& vertex0 = tpg.addNewTeam();
    const TPG::TPGAction& vertex1 = tpg.addNewAction(0);

    const TPG::TPGEdge& edge = tpg.addNewEdge(vertex0, vertex1, progPointer);

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
    ASSERT_EQ(progPointer.use_count(), 1)
        << "Edge was not properly deleted, its shared pointer is still active.";
    // Remove an edge that does not exist anymore
    ASSERT_THROW(tpg.removeEdge(edge), std::runtime_error)
        << "Edge not in the graph should not be removable";
}

TEST_F(TPGTest, TPGGraphRemoveVertex)
{
    TPG::TPGGraph tpg(*e);
    const TPG::TPGVertex& vertex0 = tpg.addNewTeam();
    const TPG::TPGAction& vertex1 = tpg.addNewAction(0);
    const TPG::TPGTeam& vertex2 = tpg.addNewTeam();

    ASSERT_NO_THROW(tpg.removeVertex(vertex0))
        << "Removing a vertex from the graph failed.";
    ASSERT_EQ(tpg.getNbVertices(), 2)
        << "Number of vertices of the TPG is incorrect after removing a "
           "TPGVertex.";
    ASSERT_EQ(tpg.getVertices().front(), &vertex1)
        << "Remaining vertex after removal is not correct.";
    ASSERT_EQ(tpg.getVertices().back(), &vertex2)
        << "Remaining vertex after removal is not correct.";

    // Try to remove a vertex not from the graph
    TPG::TPGAction vertex3(3);
    ASSERT_NO_THROW(tpg.removeVertex(vertex3))
        << "Removing a vertex from the graph (although it is not inside) "
           "throwed an exception.";
    ASSERT_EQ(tpg.getNbVertices(), 2)
        << "Number of vertices of the TPG is incorrect after removing a "
           "TPGVertex not from the graph.";

    // Add a new edge to test removal of vertex connectet to an edge.
    tpg.addNewEdge(vertex2, vertex1, progPointer);
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
    const TPG::TPGTeam& vertex4 = tpg.addNewTeam();
    tpg.addNewEdge(vertex4, vertex1, progPointer);
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

TEST_F(TPGTest, TPGGraphClear)
{
    TPG::TPGGraph tpg(*e);
    const TPG::TPGVertex& vertex0 = tpg.addNewTeam();
    const TPG::TPGAction& vertex1 = tpg.addNewAction(0);
    const TPG::TPGEdge& edge = tpg.addNewEdge(vertex0, vertex1, progPointer);

    ASSERT_NO_THROW(tpg.clear()) << "Clearing a non empty graph failed.";
    ASSERT_EQ(tpg.getNbVertices(), 0)
        << "Cleared graph is not empty of vertices as expected.";
    ASSERT_EQ(tpg.getEdges().size(), 0)
        << "Cleared graph is not empty of edges as expected.";
}

TEST_F(TPGTest, TPGGraphGetNbRootVertices)
{
    TPG::TPGGraph tpg(*e);
    const TPG::TPGVertex& vertex0 = tpg.addNewTeam();
    const TPG::TPGAction& vertex1 = tpg.addNewAction(0);
    const TPG::TPGEdge& edge = tpg.addNewEdge(vertex0, vertex1, progPointer);

    ASSERT_EQ(tpg.getNbRootVertices(), 1)
        << "Number of roots of the TPG is incorrect.";
}

TEST_F(TPGTest, TPGGraphGetRootVertices)
{
    TPG::TPGGraph tpg(*e);
    const TPG::TPGVertex& vertex0 = tpg.addNewTeam();
    const TPG::TPGAction& vertex1 = tpg.addNewAction(0);

    const TPG::TPGEdge& edge = tpg.addNewEdge(vertex0, vertex1, progPointer);
    ASSERT_EQ(tpg.getRootVertices().size(), 1)
        << "Number of roots of the TPG is incorrect.";
    ASSERT_EQ(tpg.getRootVertices().at(0), &vertex0)
        << "Vertex classified as root is incorrect.";
}

TEST_F(TPGTest, TPGGraphCloneVertex)
{
    TPG::TPGGraph tpg(*e);
    const TPG::TPGTeam& vertex0 = tpg.addNewTeam();
    const TPG::TPGAction& vertex1 = tpg.addNewAction(4);

    const TPG::TPGEdge& edge = tpg.addNewEdge(vertex0, vertex1, progPointer);

    // Clone the team
    const TPG::TPGVertex* cloneVertex;
    ASSERT_NO_THROW(cloneVertex = &tpg.cloneVertex(vertex0))
        << "Cloning a TPGTeamVertex of the TPGGraph failed.";
    // Check that the clone vertex is in the graph
    ASSERT_EQ(tpg.getNbVertices(), 3)
        << "Number of vertices of the graph after clone is incorrect.";
    ASSERT_EQ(tpg.getVertices().at(2), cloneVertex)
        << "CloneVertex is not the last of the graph vertices as it should be.";
    cloneVertex = tpg.getVertices().at(2); // to remove a compilation warning.
    // Check that the type is correct
    ASSERT_EQ(typeid(vertex0), typeid(*cloneVertex));
    ASSERT_EQ(tpg.getEdges().size(), 2)
        << "Number of edges of the graph after clone is incorrect.";
    auto destinationVertex =
        ((*cloneVertex->getOutgoingEdges().begin())->getDestination());
    ASSERT_EQ(destinationVertex, &vertex1)
        << "Cloned vertex is not connected to the correct other vertex in the "
           "Graph.";
    // Check pointer usage was increased.
    ASSERT_EQ(progPointer.use_count(), 3)
        << "Shared pointer use count should increase after cloning a vertex "
           "connected with an edge using it.";

    // Duplicate the action (to increase code coverage)
    ASSERT_NO_THROW(tpg.cloneVertex(vertex1));
    // Check that the type is correct
    ASSERT_EQ(typeid(vertex1).name(), typeid(*tpg.getVertices().at(3)).name());
    ASSERT_EQ(vertex1.getActionID(),
              ((TPG::TPGAction*)tpg.getVertices().at(3))->getActionID());

    // Clone a vertex not from the graph
    TPG::TPGVertex* vertex2 = new TPG::TPGAction(1);
    ASSERT_THROW(tpg.cloneVertex(*vertex2), std::runtime_error)
        << "Cloning a vertex that does not belong to the TPGGraph should not "
           "be possible.";
}

TEST_F(TPGTest, TPGGraphCloneEdge)
{
    TPG::TPGGraph tpg(*e);
    const TPG::TPGTeam& vertex0 = tpg.addNewTeam();
    const TPG::TPGAction& vertex1 = tpg.addNewAction(4);
    const TPG::TPGEdge& edge = tpg.addNewEdge(vertex0, vertex1, progPointer);

    const TPG::TPGEdge* clone = NULL;
    ASSERT_NO_THROW(clone = &tpg.cloneEdge(edge))
        << "Cloning an existing edge failed.";
    // Check that the new edge is correctly added to the graph
    ASSERT_EQ(tpg.getEdges().size(), 2)
        << "Incorrect number of edges in the graph after clone.";
    // Check the program use
    ASSERT_EQ(progPointer.use_count(), 3)
        << "Program pointer was not correctly registered to the edge clone.";
    // Check the edge source and destination
    ASSERT_EQ(clone->getSource(), &vertex0)
        << "Clone edge has an incorrect source.";
    ASSERT_EQ(clone->getDestination(), &vertex1)
        << "Clone edge has an incorrect destination.";
    // Check that the edge was correctly registered
    ASSERT_EQ(std::count_if(vertex0.getOutgoingEdges().begin(),
                            vertex0.getOutgoingEdges().end(),
                            [&clone](const TPG::TPGEdge* other) {
                                return other == clone;
                            }),
              1)
        << "Clone edge is not registered within its source vertex outgoing "
           "edges.";
    ASSERT_EQ(std::count_if(vertex1.getIncomingEdges().begin(),
                            vertex1.getIncomingEdges().end(),
                            [&clone](const TPG::TPGEdge* other) {
                                return other == clone;
                            }),
              1)
        << "Clone edge is not registered within its destination vertex "
           "incoming edges.";

    // Check throw behavior
    TPG::TPGEdge newEdge(&vertex0, &vertex1, progPointer);
    ASSERT_THROW(tpg.cloneEdge(newEdge), std::runtime_error)
        << "Cloning an edge not from the graph should not succeed.";
}

TEST_F(TPGTest, TPGGraphSetEdgeDestination)
{
    TPG::TPGGraph tpg(*e);
    const TPG::TPGTeam& vertex0 = tpg.addNewTeam();
    const TPG::TPGAction& vertex1 = tpg.addNewAction(4);
    const TPG::TPGAction& vertex2 = tpg.addNewAction(4);
    const TPG::TPGEdge& edge = tpg.addNewEdge(vertex0, vertex1, progPointer);

    // Change the destination of the edge
    ASSERT_TRUE(tpg.setEdgeDestination(edge, vertex2))
        << "Changing the destination of an Edge to a valid new destination "
           "should not fail.";
    // Check the graph size
    ASSERT_EQ(tpg.getEdges().size(), 1)
        << "Incorrect number of edges in the graph after edge setDestination.";
    // Check the program use
    ASSERT_EQ(progPointer.use_count(), 2)
        << "Program pointer use should not be affected by edge destination "
           "change.";
    // Check the edge source and destination
    ASSERT_EQ(edge.getSource(), &vertex0)
        << "Updated edge has an incorrect source.";
    ASSERT_EQ(edge.getDestination(), &vertex2)
        << "Updated edge has an incorrect destination.";
    // Check that the edge was correctly registered
    ASSERT_EQ(std::count_if(vertex0.getOutgoingEdges().begin(),
                            vertex0.getOutgoingEdges().end(),
                            [&edge](const TPG::TPGEdge* other) {
                                return other == &edge;
                            }),
              1)
        << "Updated edge is no longer registered within its source vertex "
           "outgoing edges.";
    ASSERT_EQ(std::count_if(vertex2.getIncomingEdges().begin(),
                            vertex2.getIncomingEdges().end(),
                            [&edge](const TPG::TPGEdge* other) {
                                return other == &edge;
                            }),
              1)
        << "Updated edge is not registered within its new destination vertex "
           "incoming edges.";
    // Check that the edge was unregistered
    ASSERT_EQ(vertex1.getIncomingEdges().size(), 0)
        << "This vertex should not have incomingEdge after destination change.";

    // Check failure
    TPG::TPGEdge newEdge(&vertex0, &vertex1, progPointer);
    ASSERT_FALSE(tpg.setEdgeDestination(newEdge, vertex2))
        << "Changing destination of an edge not within the graph should not "
           "succeed.";
}

TEST_F(TPGTest, TPGGraphSetEdgeSource)
{
    TPG::TPGGraph tpg(*e);
    const TPG::TPGTeam& vertex0 = tpg.addNewTeam();
    const TPG::TPGAction& vertex1 = tpg.addNewAction(4);
    const TPG::TPGTeam& vertex2 = tpg.addNewTeam();
    const TPG::TPGEdge& edge = tpg.addNewEdge(vertex0, vertex1, progPointer);

    // Change the destination of the edge
    ASSERT_TRUE(tpg.setEdgeSource(edge, vertex2))
        << "Changing the destination of an Edge to a valid new destination "
           "should not fail.";
    // Check the graph size
    ASSERT_EQ(tpg.getEdges().size(), 1)
        << "Incorrect number of edges in the graph after edge setDestination.";
    // Check the program use
    ASSERT_EQ(progPointer.use_count(), 2)
        << "Program pointer use should not be affected by edge source change.";
    // Check the edge source and destination
    ASSERT_EQ(edge.getSource(), &vertex2)
        << "Updated edge has an incorrect source.";
    ASSERT_EQ(edge.getDestination(), &vertex1)
        << "Updated edge has an incorrect destination.";
    // Check that the edge was correctly registered
    ASSERT_EQ(std::count_if(vertex2.getOutgoingEdges().begin(),
                            vertex2.getOutgoingEdges().end(),
                            [&edge](const TPG::TPGEdge* other) {
                                return other == &edge;
                            }),
              1)
        << "Updated edge is no longer registered within its new source vertex "
           "outgoing edges.";
    ASSERT_EQ(std::count_if(vertex1.getIncomingEdges().begin(),
                            vertex1.getIncomingEdges().end(),
                            [&edge](const TPG::TPGEdge* other) {
                                return other == &edge;
                            }),
              1)
        << "Updated edge is not registered within its destination vertex "
           "incoming edges.";
    // Check that the edge was unregistered
    ASSERT_EQ(vertex0.getOutgoingEdges().size(), 0)
        << "This vertex should not have incomingEdge after source change.";

    // Check failure
    TPG::TPGEdge newEdge(&vertex0, &vertex1, progPointer);
    ASSERT_FALSE(tpg.setEdgeSource(newEdge, vertex2))
        << "Changing source of an edge not within the graph should not "
           "succeed.";
}

TEST_F(TPGTest, TPGMoveOperator)
{
    TPG::TPGGraph source(*e);
    TPG::TPGGraph* destination =
        new TPG::TPGGraph(*e); // creates an empty tpg graph

    const TPG::TPGTeam& vertex0 = source.addNewTeam();
    const TPG::TPGAction& vertex1 = source.addNewAction(4);
    const TPG::TPGTeam& vertex2 = source.addNewTeam();
    const TPG::TPGEdge& edge = source.addNewEdge(vertex0, vertex1, progPointer);
    const TPG::TPGEdge& edge2 =
        source.addNewEdge(vertex2, vertex1, progPointer);
    const TPG::TPGEdge& edge3 =
        source.addNewEdge(vertex0, vertex2, progPointer);

    /*
     *	 T2
     *	 ^	\
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
    TPG::TPGGraph source(*e);

    ASSERT_NO_THROW(TPG::TPGGraph& destination = source)
        << "The affectation operator is never supposed to fail";
}
