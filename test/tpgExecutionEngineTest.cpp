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

#include "tpg/tpgExecutionEngine.h"

class TPGExecutionEngineTest : public ::testing::Test {
protected:
	const size_t size1{ 24 };
	const size_t size2{ 32 };
	std::vector<std::reference_wrapper<const DataHandlers::DataHandler>> vect;
	Instructions::Set set;
	Environment* e = NULL;
	std::vector<std::shared_ptr<Program::Program>> progPointers;

	TPG::TPGGraph* tpg;
	std::vector<const TPG::TPGEdge*> edges;
	Archive a;

	/**
	* Populate the program instructions so that it returns the given value.
	*
	* \param[in] value a double valut between -1.0 and 1.0.
	*/
	void makeProgramReturn(Program::Program& prog, double value) {
		auto& line = prog.addNewLine();
		// do an multby constant with DHandler 0
		line.setInstructionIndex(1);
		line.setOperand(0, 1, 0); // Dhandler 0 location 0
		line.setDestinationIndex(0); // 0th register des
		line.setParameter(0, (float)value);
	}

	virtual void SetUp() {
		// Setup environment
		vect.push_back(*(new DataHandlers::PrimitiveTypeArray<double>((unsigned int)size1)));
		vect.push_back(*(new DataHandlers::PrimitiveTypeArray<int>((unsigned int)size2)));

		// Put a 1 in the dataHandler to make it easy to have non-zero return in Programs.
		((DataHandlers::PrimitiveTypeArray<double>&)vect.at(0).get()).setDataAt(typeid(PrimitiveType<double>), 0, 1.0);

		set.add(*(new Instructions::AddPrimitiveType<double>()));
		set.add(*(new Instructions::MultByConstParam<double, float>()));
		e = new Environment(set, vect, 8);
		tpg = new TPG::TPGGraph(*e);

		// Create 10 programs
		for (int i = 0; i < 10; i++) {
			progPointers.push_back(std::shared_ptr<Program::Program>(new Program::Program(*e)));
		}

		// Create a TPG 
		// (T= Team, A= Action)
		// 
		//        .------. 
		//        v      |
		// T0---->T1---->T2     T4
		// |     /| \    |      |
		// v    / v  \   v      v
		// A0<-'  A1  `->A2     A3
		// 
		// With four action and four teams
		for (int i = 0; i < 4; i++) {
			tpg->addNewTeam();
		}
		for (int i = 0; i < 4; i++) {
			// Each action is linked to a team (and vice-versa)
			tpg->addNewAction(i);
			edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(i), *tpg->getVertices().back(), progPointers.at(i)));
		}

		// Add new Edges between teams
		edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(0), *tpg->getVertices().at(1), progPointers.at(4)));
		edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(1), *tpg->getVertices().at(2), progPointers.at(5)));

		// Add a cyclic edge
		edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(2), *tpg->getVertices().at(1), progPointers.at(6)));

		// Add new outgoing edge to one team
		edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(1), *tpg->getVertices().at(4), progPointers.at(7)));
		edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(1), *tpg->getVertices().at(6), progPointers.at(8)));

		// Put a weight on edges
		makeProgramReturn(*progPointers.at(0), 0.5); // T0->A0
		makeProgramReturn(*progPointers.at(1), 0.5); // T1->A1
		makeProgramReturn(*progPointers.at(2), 0.3); // T2->A2
		makeProgramReturn(*progPointers.at(3), 0.0); // T3->A3
		makeProgramReturn(*progPointers.at(4), 0.8); // T0->T1
		makeProgramReturn(*progPointers.at(5), 0.9); // T1->T2
		makeProgramReturn(*progPointers.at(6), 0.7); // T2->T1
		makeProgramReturn(*progPointers.at(7), 0.6); // T1->A0
		makeProgramReturn(*progPointers.at(8), 0.3); // T1->A2

		// Check the characteristics
		ASSERT_EQ(tpg->getNbVertices(), 8);
		ASSERT_EQ(tpg->getEdges().size(), 9);
		ASSERT_EQ(tpg->getRootVertices().size(), 2);
	}

	virtual void TearDown() {
		delete tpg;
		delete e;
		delete (&(vect.at(0).get()));
		delete (&(vect.at(1).get()));
		delete (&set.getInstruction(0));
		delete (&set.getInstruction(1));
	}
};

TEST_F(TPGExecutionEngineTest, ConstructorDestructor) {
	TPG::TPGExecutionEngine* tpee;

	ASSERT_NO_THROW(tpee = new TPG::TPGExecutionEngine(*e)) << "Construction of a TPGExecutionEngine failed.";

	ASSERT_NO_THROW(delete tpee) << "Deletion of a TPGExecutionEngine failed.";
}

TEST_F(TPGExecutionEngineTest, EvaluateEdge) {
	TPG::TPGExecutionEngine tpee(*e);

	ASSERT_NEAR(tpee.evaluateEdge(*edges.at(0)), 0.5, PARAM_FLOAT_PRECISION) << "Evaluation of the program of an Edge failed.";
}

TEST_F(TPGExecutionEngineTest, ArchiveUsage) {
	TPG::TPGExecutionEngine tpee(*e, &a);

	ASSERT_NEAR(tpee.evaluateEdge(*edges.at(0)), 0.5, PARAM_FLOAT_PRECISION) << "Evaluation of the program of an Edge failed when result is archived.";
	ASSERT_EQ(a.getNbRecordings(), 1) << "No recording was added to the archive.";
}

TEST_F(TPGExecutionEngineTest, EvaluateTeam) {
	TPG::TPGExecutionEngine tpee(*e);

	const TPG::TPGEdge* result = NULL;
	ASSERT_NO_THROW(result = &tpee.evaluateTeam(*(const TPG::TPGTeam*)(tpg->getVertices().at(1)), {});) << "Evaluation of a valid TPGTeam with no exclusion failed.";
	// Expected result is edge between T1 -> T2 (with 0.9)
	ASSERT_EQ(result, edges.at(5)) << "Edge selected during team evaluation is incorrect.";

	// Exclude an edge
	ASSERT_NO_THROW(result = &tpee.evaluateTeam(*(const TPG::TPGTeam*)(tpg->getVertices().at(1)), { tpg->getVertices().at(2) });) << "Evaluation of a valid TPGTeam with one exclusion failed.";
	// Expected result is edge between T1 -> A0 (with 0.6)
	ASSERT_EQ(result, edges.at(7)) << "Edge selected during team evaluation is incorrect.";

	// Exclude all edges
	ASSERT_THROW(result = &tpee.evaluateTeam(*(const TPG::TPGTeam*)(tpg->getVertices().at(0)), { tpg->getVertices().at(1),  tpg->getVertices().at(4) }), std::runtime_error) << "Evaluation of a TPGTeam with all edges excluded did not fail as expected.";
}

TEST_F(TPGExecutionEngineTest, EvaluateFromRoot) {
	TPG::TPGExecutionEngine tpee(*e);

	std::vector<const TPG::TPGVertex*> result;

	ASSERT_NO_THROW(result = tpee.executeFromRoot(*tpg->getRootVertices().at(0))) << "Execution of a TPGGraph from a valid root failed.";
	// Check the traversed path
	ASSERT_EQ(result.size(), 4) << "Size of the traversed path during the execution of the TPGGraph is not as expected.";
	ASSERT_EQ(result.at(0), tpg->getVertices().at(0)) << "0th element (i.e. the root) of the traversed path during execution is incorrect.";
	ASSERT_EQ(result.at(1), tpg->getVertices().at(1)) << "1st element of the traversed path during execution is incorrect.";
	ASSERT_EQ(result.at(2), tpg->getVertices().at(2)) << "2nd element of the traversed path during execution is incorrect.";
	ASSERT_EQ(result.at(3), tpg->getVertices().at(6)) << "2nd element of the traversed path during execution is incorrect.";
}