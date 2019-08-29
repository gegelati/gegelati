#include <gtest/gtest.h>

#include "dataHandlers/dataHandler.h"
#include "dataHandlers/primitiveTypeArray.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/multByConstParam.h"
#include "program/program.h"

#include "tpg/tpgVertex.h"
#include "tpg/tpgTeam.h"
#include "tpg/tpgAction.h"
#include "tpg/tpgEdge.h"
#include "tpg/tpgGraph.h"

class TPGTest : public ::testing::Test {
protected:
	const size_t size1{ 24 };
	const size_t size2{ 32 };
	std::vector<std::reference_wrapper<DataHandlers::DataHandler>> vect;
	Instructions::Set set;
	Environment* e = NULL;
	std::shared_ptr<Program::Program> progPointer;

	virtual void SetUp() {
		vect.push_back(*(new DataHandlers::PrimitiveTypeArray<double>((unsigned int)size1)));
		vect.push_back(*(new DataHandlers::PrimitiveTypeArray<int>((unsigned int)size2)));

		set.add(*(new Instructions::AddPrimitiveType<float>()));
		set.add(*(new Instructions::MultByConstParam<double, float>()));

		e = new Environment(set, vect, 8);
		progPointer = std::shared_ptr<Program::Program>(new Program::Program(*e));
	}

	virtual void TearDown() {
		delete e;
		delete (&(vect.at(0).get()));
		delete (&(vect.at(1).get()));
		delete (&set.getInstruction(0));
		delete (&set.getInstruction(1));
	}
};

TEST_F(TPGTest, TPGTeamAndTPGActionConstructorsDestructors) {
	TPG::TPGVertex* team;
	TPG::TPGVertex* action;

	ASSERT_NO_THROW(team = new TPG::TPGTeam());
	ASSERT_NO_THROW(action = new TPG::TPGAction());

	ASSERT_NO_THROW(delete team);
	ASSERT_NO_THROW(delete action);
}

TEST_F(TPGTest, TPGEdgeConstructorDestructor) {
	TPG::TPGTeam team;
	TPG::TPGAction action;

	TPG::TPGEdge* edge;

	ASSERT_NO_THROW(edge = new TPG::TPGEdge(&team, &action, progPointer));

	ASSERT_NO_THROW(delete edge);
}

TEST_F(TPGTest, TPGVertexEdgesSettersGetters) {
	TPG::TPGTeam team;
	TPG::TPGAction action;

	TPG::TPGEdge edge(&team, &action, progPointer);

	ASSERT_NO_THROW(team.addOutgoingEdge(&edge)) << "Adding an outgoing edge to a Team vertex failed.";
	ASSERT_THROW(action.addOutgoingEdge(&edge), std::runtime_error) << "Adding an outgoing edge to an Action vertex did not fail.";
	ASSERT_NO_THROW(action.addIncomingEdge(&edge)) << "Adding an outgoing edge to an Action vertex failed.";

	ASSERT_EQ(team.getOutgoingEdges().size(), 1) << "Size of the outgoing edges of the node is incorrect.";
	ASSERT_EQ(team.getOutgoingEdges().count(&edge), 1) << "TPGEdge pointer contained in the outgoingEdges is incorrect.";

	// Add the same edge again.. Nothing should happen, but it should not fail.
	ASSERT_NO_THROW(team.addOutgoingEdge(&edge)) << "Adding an outgoing edge to a Team vertex, even though it is already there, failed unexpectedly.";
	ASSERT_EQ(team.getOutgoingEdges().size(), 1) << "Size of the outgoing edges of the node is incorrect.";

	// Add Null pointer: Nothing should happen, but it should not fail.
	ASSERT_NO_THROW(team.addOutgoingEdge(NULL)) << "Adding an outgoing edge to a Team vertex, even though it is already there, failed unexpectedly.";
	ASSERT_EQ(team.getOutgoingEdges().size(), 1) << "Size of the outgoing edges of the node is incorrect.";

	ASSERT_EQ(action.getIncomingEdges().size(), 1) << "Size of the outgoing edges of the node is incorrect.";
	ASSERT_EQ(action.getIncomingEdges().count(&edge), 1) << "TPGEdge pointer contained in the outgoingEdges is incorrect.";

	// Remove edges
	ASSERT_NO_THROW(team.removeOutgoingEdge(&edge)) << "Removing an outgoing edge failed.";
	ASSERT_EQ(team.getOutgoingEdges().size(), 0) << "Size of the outgoing edges of the node is incorrect.";

	ASSERT_NO_THROW(action.removeIncomingEdge(&edge)) << "Removing an outgoing edge to an Action vertex failed.";
	ASSERT_EQ(action.getIncomingEdges().size(), 0) << "Size of the outgoing edges of the node is incorrect.";

	// Remove Null or non existing edge.
	ASSERT_NO_THROW(team.removeOutgoingEdge(NULL)) << "Removing an NULL edge failed, while it should just do nothing.";
	ASSERT_NO_THROW(action.removeIncomingEdge(&edge)) << "Removing an edge no longer in the set should do nothing, but not fail.";
}

TEST_F(TPGTest, TPGEdgeGetSetProgram) {
	TPG::TPGTeam team;
	TPG::TPGAction action;

	const TPG::TPGEdge constEdge(&team, &action, progPointer);
	const Program::Program& constProg = constEdge.getProgram();
	ASSERT_EQ(&constProg, progPointer.get()) << "Program accessor on const TPGEdge returns a Program different from the one given at construction.";

	TPG::TPGEdge edge(&team, &action, progPointer);
	Program::Program& prog = edge.getProgram();
	ASSERT_EQ(&prog, progPointer.get()) << "Program accessor on TPGEdge returns a Program different from the one given at construction.";

	std::shared_ptr<Program::Program> progPointer2(new Program::Program(*e));
	edge.setProgram(progPointer2);
	ASSERT_EQ(&edge.getProgram(), progPointer2.get()) << "Program accessor on TPGEdge returns a Program different from the one set before.";
}

TEST_F(TPGTest, TPGEdgeGetSetSourceAndDestination) {
	TPG::TPGTeam team0, team1;
	TPG::TPGAction action0, action1;

	TPG::TPGEdge edge(&team0, &action0, progPointer);

	ASSERT_EQ(&team0, edge.getSource()) << "Source of the TPGEdge differs from the one given at construction.";
	ASSERT_EQ(&action0, edge.getDestination()) << "Source of the TPGEdge differs from the one given at construction.";

	edge.setSource(&team1);
	ASSERT_EQ(&team1, edge.getSource()) << "Source of the TPGEdge differs from the one set right before.";

	edge.setDestination(&action1);
	ASSERT_EQ(&action1, edge.getDestination()) << "Destination of the TPGEdge differs from the one set right before.";
}

TEST_F(TPGTest, TPGGraphAddTPGVertex) {
	TPG::TPGGraph tpg;
	const TPG::TPGTeam* t;
	const TPG::TPGAction* a;
	ASSERT_NO_THROW(t = &tpg.addNewTeam()) << "Adding a new Team to a TPGGraph failed.";
	ASSERT_NO_THROW(a = &tpg.addNewAction()) << "Adding a new Action to a TPGGraph failed.";

}

TEST_F(TPGTest, TPGGraphGetVertices) {
	TPG::TPGGraph tpg;
	const TPG::TPGVertex& vertex = tpg.addNewTeam();
	const std::vector<const TPG::TPGVertex *> vertices = tpg.getVertices();
	ASSERT_EQ(vertices.size(), 1) << "Size of the retrievd std::vector<TPGVertex> is incorrect.";
	ASSERT_EQ(vertices.front(), &vertex) << "Vertex in the retrieved vertices list does not correspond to the one added to the TPGGrapg (pointer comparison)";
}

TEST_F(TPGTest, TPGGraphRemoveVertex) {
	TPG::TPGGraph tpg;
	const TPG::TPGVertex& vertex0 = tpg.addNewTeam();
	const TPG::TPGAction& vertex1 = tpg.addNewAction();

	ASSERT_NO_THROW(tpg.removeVertex(vertex0)) << "Removing a vertex from the graph failed.";
	ASSERT_EQ(tpg.getVertices().size(), 1) << "Number of vertices of the TPG is incorrect after removing a TPGVertex.";
	ASSERT_EQ(tpg.getVertices().front(), &vertex1) << "Remaining vertex after removal is not correct.";

	// Try to remove a vertex not from the graph
	TPG::TPGAction vertex2;
	ASSERT_NO_THROW(tpg.removeVertex(vertex2)) << "Removing a vertex from the graph (although it is not inside) throwed an exception.";
	ASSERT_EQ(tpg.getVertices().size(), 1) << "Number of vertices of the TPG is incorrect after removing a TPGVertex not from the graph.";
}

TEST_F(TPGTest, TPGGraphAddEdge) {
	TPG::TPGGraph tpg;
	const TPG::TPGVertex& vertex0 = tpg.addNewTeam();
	const TPG::TPGAction& vertex1 = tpg.addNewAction();

	ASSERT_NO_THROW(tpg.addNewEdge(vertex0, vertex1, progPointer)) << "Adding an edge between a team and an action failed.";
	// Add with a vertex not in the graph.
	TPG::TPGAction vertex2;
	ASSERT_THROW(tpg.addNewEdge(vertex0, vertex2, progPointer), std::runtime_error) << "Adding an edge with a vertex not from the graph should have failed.";

	// Add the edge from the action
	ASSERT_THROW(tpg.addNewEdge(vertex1, vertex0, progPointer), std::runtime_error) << "Adding an edge from an Action should have failed.";
}

TEST_F(TPGTest, TPGGraphGetEdges) {
	TPG::TPGGraph tpg;
	const TPG::TPGVertex& vertex0 = tpg.addNewTeam();
	const TPG::TPGAction& vertex1 = tpg.addNewAction();

	TPG::TPGEdge& edge = tpg.addNewEdge(vertex0, vertex1, progPointer);
	ASSERT_EQ(tpg.getEdges().size(), 1) << "Edges of the graph have incorrect size after successful add.";

	// Attempt an impossible add.
	try { tpg.addNewEdge(vertex1, vertex0, progPointer); }
	catch (std::runtime_error e) {
		// do nothing
	}
	ASSERT_EQ(tpg.getEdges().size(), 1) << "Edges of the graph have incorrect size after unsuccessful add.";
}