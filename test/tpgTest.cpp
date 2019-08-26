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

