#include <gtest/gtest.h>

#include "instructions/instruction.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/multByConstParam.h"
#include "instructions/lambdaInstruction.h"

#include "tpg/policyStats.h"

class PolicyStatsTest : public ::testing::Test {
protected:

	std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
	Instructions::Set set;
	Environment* e = NULL;
	std::vector<std::shared_ptr<Program::Program>> progPointers;

	TPG::TPGGraph* tpg;
	std::vector<const TPG::TPGEdge*> edges;

	virtual void SetUp() {
		// Instructions
		std::function<double(double, const double[3])> mac = [](double a, const double b[3]) {
			return a * (b[0] + b[1] + b[2]);
		};
		set.add(*(new Instructions::MultByConstParam<double, float>()));
		set.add(*(new Instructions::AddPrimitiveType<double>()));
		set.add(*(new Instructions::LambdaInstruction<double, const double[3]>(mac)));

		// Data handler
		// Setup environment
		vect.push_back(*(new Data::PrimitiveTypeArray<double>((unsigned int)25)));

		// Environment
		e = new Environment(set, vect, 8);

		// Create 9 programs
		for (int i = 0; i < 9; i++) {
			progPointers.push_back(std::shared_ptr<Program::Program>(new Program::Program(*e)));
		}

		// Create a TPG 
		// (T= Team, A= Action)
		// 
		//        .------. 
		//        v      |
		// T0---->T1---->T2<----T3
		// |     /| \    |       |
		// v    / v  \   v       v
		// A0<-'  A1  `->A2     A3
		// 
		// With four action and four teams
		// All Edges have a unique Program, except T1->A0 and T0->A0 which 
		// share the same program: progPointers.at(0)
		tpg = new TPG::TPGGraph(*e);
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
		edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(3), *tpg->getVertices().at(2), progPointers.at(6)));

		// Add a cyclic edge
		edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(2), *tpg->getVertices().at(1), progPointers.at(7)));

		// Add new outgoing edge to one team
		edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(1), *tpg->getVertices().at(4), progPointers.at(0)));
		edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(1), *tpg->getVertices().at(6), progPointers.at(8)));

		// Check the characteristics
		ASSERT_EQ(tpg->getNbVertices(), 8);
		ASSERT_EQ(tpg->getEdges().size(), 10);
		ASSERT_EQ(tpg->getRootVertices().size(), 2);

		// Add instructions to 2 programs

		// Program 0 (referenced by two edges)
		Program::Line* l = &progPointers.at(0).get()->addNewLine();
		// Intron
		l->setInstructionIndex(0); // MultByConst
		l->setDestinationIndex(4); // Register[4]
		l->setParameter(0, 0.2f); // Param
		l->setOperand(0, 1, 0); //Array[0]

		l = &progPointers.at(0).get()->addNewLine();
		l->setInstructionIndex(1); // Add
		l->setDestinationIndex(1); // Register[1]
		l->setOperand(0, 1, 2); // Array[2]
		l->setOperand(1, 1, 30); // Array[5]

		l = &progPointers.at(0).get()->addNewLine();
		l->setInstructionIndex(2); // Lambda
		l->setDestinationIndex(0); // Register[0]
		l->setOperand(0, 1, 2); // Array[2]
		l->setOperand(1, 0, 1); // Register[1 .. 3]

		progPointers.at(0).get()->identifyIntrons();

		// Program 1 (referenced by one edge)
		l = &progPointers.at(1).get()->addNewLine();
		l->setInstructionIndex(2); // Lambda
		l->setDestinationIndex(0); // Register[0]
		l->setOperand(0, 1, 10); //Array[10]
		l->setOperand(1, 1, 12); //Array[12 .. 15]

		progPointers.at(1).get()->identifyIntrons();
	}

	virtual void TearDown() {
		delete tpg;
		delete e;
		delete (&(vect.at(0).get()));
		delete (&set.getInstruction(0));
		delete (&set.getInstruction(1));
		delete (&set.getInstruction(2));
	}
};


TEST_F(PolicyStatsTest, SetEnvironment) {
	TPG::PolicyStats ps;
	ASSERT_NO_THROW(ps.setEnvironment(*e)) << "Setting an Environment for the PolicyStats failed.";
}

TEST_F(PolicyStatsTest, AnalyzeLine) {
	TPG::PolicyStats ps;
	ps.setEnvironment(*e);

	ASSERT_NO_THROW(ps.analyzeLine(&progPointers.at(0)->getLine(0))) << "Analysis of a valid line failed unexpectedly.";

	// Check analysis results
	ASSERT_EQ(ps.nbUsagePerDataLocation.size(), 1) << "Incorrect attribute value after analyzing one line.";
	ASSERT_EQ(ps.nbUsagePerDataLocation.begin()->first, (std::make_pair<size_t, size_t>(1, 0))) << "Incorrect attribute value after analyzing one line.";
	ASSERT_EQ(ps.nbUsagePerDataLocation.begin()->second, 1) << "Incorrect attribute value after analyzing one line.";
	ASSERT_EQ(ps.nbUsagePerInstruction.size(), 1) << "Incorrect attribute value after analyzing one line.";
	ASSERT_EQ(ps.nbUsagePerInstruction.begin()->first, 0) << "Incorrect attribute value after analyzing one line.";
	ASSERT_EQ(ps.nbUsagePerInstruction.begin()->second, 1) << "Incorrect attribute value after analyzing one line.";
}