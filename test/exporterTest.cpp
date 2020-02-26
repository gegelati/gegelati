#include <gtest/gtest.h>

#include <fstream>

#include "dataHandlers/dataHandler.h"
#include "dataHandlers/primitiveTypeArray.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/multByConstParam.h"
#include "instructions/lambdaInstruction.h"
#include "program/program.h"
#include "tpg/tpgVertex.h"
#include "tpg/tpgTeam.h"
#include "tpg/tpgAction.h"
#include "tpg/tpgEdge.h"
#include "tpg/tpgGraph.h"

#include "file/tpgGraphDotExporter.h"


class ExporterTest : public ::testing::Test {
public:
	ExporterTest() : tpg(NULL) {};

protected:
	const size_t size1{ 24 };
	std::vector<std::reference_wrapper<const DataHandlers::DataHandler>> vect;
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


		auto minus = [](double a, double b)->double {return a - b; };

		set.add(*(new Instructions::AddPrimitiveType<double>()));
		set.add(*(new Instructions::MultByConstParam<double, float>()));
		set.add(*(new Instructions::LambdaInstruction<double>(minus)));

		e = new Environment(set, vect, 8);
		tpg = new TPG::TPGGraph(*e);

		// Create 10 programs
		for (int i = 0; i < 8; i++) {
			progPointers.push_back(std::shared_ptr<Program::Program>(new Program::Program(*e)));
		}

		//add instructions to at least one program.
		for (int i = 0; i < 3; i++)
		{
			Program::Line& l = progPointers.at(0).get()->addNewLine();
			l.setInstructionIndex(0);
			l.setDestinationIndex(1);
			l.setParameter(0, 0.2f);
			l.setOperand(0, 0, 1);
		}

		// Create a TPG 
		// (T= Team, A= Action)
		// 
		//        .------. 
		//        v      |
		// T0---->T1---->T2     T4
		// |     /| \    |      |
		// v    / v  \   v      v
		// A0<-'  A1  `->A2     A3   A4
		// 
		// With four action and four teams
		// All Edges have a unique Program, except T1->A0 and T0->A0 which 
		// share the same program: progPointers.at(0)
		for (int i = 0; i < 4; i++) {
			tpg->addNewTeam();
		}
		for (int i = 0; i < 4; i++) {
			// Each action is linked to a team (and vice-versa)
			tpg->addNewAction(i);
			edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(i), *tpg->getVertices().back(), progPointers.at(i)));
		}

		// Add an additional Root Action
		tpg->addNewAction(4);

		// Add new Edges between teams
		edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(0), *tpg->getVertices().at(1), progPointers.at(4)));
		edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(1), *tpg->getVertices().at(2), progPointers.at(5)));

		// Add a cyclic edge
		edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(2), *tpg->getVertices().at(1), progPointers.at(6)));

		// Add new outgoing edge to one team
		edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(1), *tpg->getVertices().at(4), progPointers.at(0)));
		edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(1), *tpg->getVertices().at(6), progPointers.at(7)));

		// Check the characteristics
		ASSERT_EQ(tpg->getNbVertices(), 9);
		ASSERT_EQ(tpg->getEdges().size(), 9);
		ASSERT_EQ(tpg->getRootVertices().size(), 3);
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
	File::TPGGraphDotExporter* dotExporter;
	ASSERT_NO_THROW(dotExporter = new File::TPGGraphDotExporter("exported_tpg.dot", *tpg)) << "The TPGGraphDotExporter could not be constructed with a valid file path.";

	ASSERT_NO_THROW(delete dotExporter;) << "TPGGraphDotExporter could not be deleted.";

	ASSERT_THROW(dotExporter = new File::TPGGraphDotExporter("XXX://INVALID_PATH", *tpg), std::runtime_error) << "The TPGGraphDotExplorer construction should fail with an invalid path.";
}

TEST_F(ExporterTest, print) {
	File::TPGGraphDotExporter dotExporter("exported_tpg.dot", *tpg);

	ASSERT_NO_THROW(dotExporter.print()) << "File export was executed without error.";
}

TEST_F(ExporterTest, FileContentVerification) {
	// This Test checks the content of the exported file against a golden reference.
	File::TPGGraphDotExporter dotExporter("exported_tpg.dot", *tpg);

	dotExporter.print();

	std::ifstream goldenRef(TESTS_DAT_PATH "exported_tpg_ref.dot");
	ASSERT_TRUE(goldenRef.is_open()) << "Could not open golden reference. Check project configuration.";

	std::ifstream exportedFile("exported_tpg.dot");
	ASSERT_TRUE(exportedFile) << "Could not open exported dot file.";

	// Check the file content line by line
	// print diffs in the console and count number of printed line.
	uint64_t nbDiffs = 0;
	uint64_t lineNumber = 0;
	while (!exportedFile.eof() && !goldenRef.eof()) {
		std::string lineRef;
		std::getline(goldenRef, lineRef);

		std::string lineExport;
		std::getline(exportedFile, lineExport);

		EXPECT_EQ(lineRef, lineExport) << "Diff at Line " << lineNumber;
		nbDiffs += (lineRef != lineExport) ? 1 : 0;

		lineNumber++;
	}

	ASSERT_EQ(exportedFile.eof(), goldenRef.eof()) << "Files have different length.";

	ASSERT_EQ(nbDiffs, 0) << "Differences between reference file and exported file were detected.";
}

