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

#include "exporter/tpgGraphDotExporter.h"


class ExporterTest : public ::testing::Test {
protected:
	const size_t size1{ 24 };
	std::vector<std::reference_wrapper<DataHandlers::DataHandler>> vect;
	Instructions::Set set;
	Environment* e = NULL;
	std::vector<std::shared_ptr<Program::Program>> progPointers;

	TPG::TPGGraph* tpg;
	std::vector<const TPG::TPGEdge*> edges;


	virtual void SetUp() {
		// Setup environment
		vect.push_back(*(new DataHandlers::PrimitiveTypeArray<double>((unsigned int)size1)));

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

		// Check the characteristics
		ASSERT_EQ(tpg->getNbVertices(), 8);
		ASSERT_EQ(tpg->getEdges().size(), 9);
		ASSERT_EQ(tpg->getRootVertices().size(), 2);
	}

	virtual void TearDown() {
		delete tpg;
		delete e;
		delete (&(vect.at(0).get()));
		delete (&set.getInstruction(0));
		delete (&set.getInstruction(1));
	}
};

TEST_F(ExporterTest, Constructor) {
	Exporter::TPGGraphDotExporter* dotExporter;
	ASSERT_NO_THROW(dotExporter = new Exporter::TPGGraphDotExporter("exported_tpg.dot", *tpg)) << "The TPGGraphDotExporter could not be constructed with a valid file path.";

	ASSERT_NO_THROW(delete dotExporter;) << "TPGGraphDotExporter could not be deleted.";

	ASSERT_THROW(dotExporter = new Exporter::TPGGraphDotExporter("XXX:\\INVALID_PATH", *tpg), std::runtime_error) << "The TPGGraphDotExplorer construction should fail with an invalid path.";
}

TEST_F(ExporterTest, print) {
	Exporter::TPGGraphDotExporter dotExporter("exported_tpg.dot", *tpg);

	ASSERT_NO_THROW(dotExporter.print());
}