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
		l->setOperand(1, 0, 13); // Register[5]

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

TEST_F(PolicyStatsTest, AnalyzeProgram) {
	TPG::PolicyStats ps;
	ps.setEnvironment(*e);

	// Do the analysis twice to check that analyzing the same program 
	// a second twice does not change most attributes except nbUsePerProgram.
	for (auto i = 0; i < 2; i++) {
		ASSERT_NO_THROW(ps.analyzeProgram(progPointers.at(0).get())) << "Analysis of a valid Program failed unexpectedly.";

		// Check analysis results
		ASSERT_EQ(ps.nbLinesPerProgram.size(), 1) << "Incorrect attribute value after analyzing a Program.";
		ASSERT_EQ(ps.nbLinesPerProgram.at(0), 3) << "Incorrect attribute value after analyzing a Program.";
		ASSERT_EQ(ps.nbIntronPerProgram.size(), 1) << "Incorrect attribute value after analyzing a Program.";
		ASSERT_EQ(ps.nbIntronPerProgram.at(0), 1) << "Incorrect attribute value after analyzing a Program.";
		ASSERT_EQ(ps.nbUsePerProgram.size(), 1) << "Incorrect attribute value after analyzing a Program.";
		ASSERT_EQ(ps.nbUsePerProgram.begin()->first, progPointers.at(0).get()) << "Incorrect attribute value after analyzing a Program.";
		ASSERT_EQ(ps.nbUsePerProgram.begin()->second, i + 1) << "Incorrect attribute value after analyzing a Program.";
		// Only non intron are counted
		ASSERT_EQ(ps.nbUsagePerInstruction.size(), 2) << "Incorrect attribute value after analyzing a Program.";
		auto iter1 = ps.nbUsagePerInstruction.begin();
		ASSERT_EQ(iter1->first, 1) << "Incorrect attribute value after analyzing a Program.";
		ASSERT_EQ(iter1->second, 1) << "Incorrect attribute value after analyzing a Program.";
		iter1++;
		ASSERT_EQ(iter1->first, 2) << "Incorrect attribute value after analyzing a Program.";
		ASSERT_EQ(iter1->second, 1) << "Incorrect attribute value after analyzing a Program.";
		ASSERT_EQ(ps.nbUsagePerDataLocation.size(), 5) << "Incorrect attribute value after analyzing a Program.";
		auto iter2 = ps.nbUsagePerDataLocation.begin();
		std::map<std::pair<size_t, size_t>, size_t> content = {
			{ {0,1}, 1 },
			{ {0,2}, 1 },
			{ {0,3}, 1 },
			{ {0,5}, 1 },
			{ {1,2}, 2 }
		};
		ASSERT_EQ(ps.nbUsagePerDataLocation, content) << "Incorrect attribute value after analyzing a Program.";
	}
}

TEST_F(PolicyStatsTest, AnalyzeTPGTeam) {
	TPG::PolicyStats ps;
	ps.setEnvironment(*e);

	for (auto i = 0; i < 2; i++) {
		ASSERT_NO_THROW(ps.analyzeTPGTeam((const TPG::TPGTeam*)tpg->getVertices().at(0))) << "Analysis of a valid TPGTeam failed unexpectedly.";

		// Check attributes
		ASSERT_EQ(ps.nbUsePerTPGTeam.size(), 1);
		ASSERT_EQ(ps.nbUsePerTPGTeam.begin()->first, tpg->getVertices().at(0));
		ASSERT_EQ(ps.nbUsePerTPGTeam.begin()->second, i + 1);
		ASSERT_EQ(ps.nbOutgoingEdgesPerTeam.size(), 1);
		ASSERT_EQ(*ps.nbOutgoingEdgesPerTeam.begin(), 2);
		ASSERT_EQ(ps.nbDistinctTeams, 1);
	}
}

TEST_F(PolicyStatsTest, AnalyzeTPGAction) {
	TPG::PolicyStats ps;
	ps.setEnvironment(*e);

	for (auto i = 0; i < 2; i++) {
		ASSERT_NO_THROW(ps.analyzeTPGAction((const TPG::TPGAction*)tpg->getVertices().at(4))) << "Analysis of a valid TPGAction failed unexpectedly.";

		// Check attributes
		ASSERT_EQ(ps.nbUsePerTPGAction.size(), 1);
		ASSERT_EQ(ps.nbUsePerTPGAction.begin()->first, tpg->getVertices().at(4));
		ASSERT_EQ(ps.nbUsePerTPGAction.begin()->second, i + 1);
		ASSERT_EQ(ps.nbUsagePerActionID.size(), 1);
		ASSERT_EQ(ps.nbUsagePerActionID.begin()->first, 0);
		ASSERT_EQ(ps.nbUsagePerActionID.begin()->second, i + 1);
	}
}

TEST_F(PolicyStatsTest, AnalyzePolicy) {
	TPG::PolicyStats ps;
	ps.setEnvironment(*e);

	ASSERT_NO_THROW(ps.analyzePolicy(tpg->getVertices().at(0))) << "Analysis of a valid Policy failed";

	// Check analysis results
	ASSERT_EQ(ps.maxPolicyDepth, 3);
	ASSERT_EQ(ps.nbDistinctTeams, 3);

	std::map<size_t, size_t> nbTPGVertexPerLevel{
		 {0, 1}, {1, 2}, {2, 4}, {3, 2} };
	ASSERT_EQ(ps.nbTPGVertexPerDepthLevel, nbTPGVertexPerLevel);

	std::vector<size_t> nbLinesPerProgram{ 3, 0, 1, 0, 0, 0, 0 };
	ASSERT_EQ(ps.nbLinesPerProgram, nbLinesPerProgram);

	std::vector<size_t> nbIntronPerProgram{ 1, 0, 0, 0, 0, 0, 0 };
	ASSERT_EQ(ps.nbIntronPerProgram, nbIntronPerProgram);

	std::vector<size_t> nbOutgoingEdgesPerTeam{ 2, 4, 2 };
	ASSERT_EQ(ps.nbOutgoingEdgesPerTeam, nbOutgoingEdgesPerTeam);

	std::map<size_t, size_t> nbUsagePerActionID{ {0, 2}, {1, 1}, {2, 2} };
	ASSERT_EQ(ps.nbUsagePerActionID, nbUsagePerActionID);

	std::map<size_t, size_t>nbUsagePerInstruction{ {1,1}, {2,2} };
	ASSERT_EQ(ps.nbUsagePerInstruction, nbUsagePerInstruction);

	std::map<std::pair<size_t, size_t>, size_t>nbUsagePerDataLocation{
			{ {0,1}, 1 },
			{ {0,2}, 1 },
			{ {0,3}, 1 },
			{ {0,5}, 1 },
			{ {1,2}, 2 },
			{ {1,10}, 1 },
			{ {1,12}, 1 },
			{ {1,13}, 1 },
			{ {1,14}, 1 }
	};
	ASSERT_EQ(ps.nbUsagePerDataLocation, nbUsagePerDataLocation);

	std::vector<size_t> nbUsePerProgram{ 2, 1, 1, 0, 1, 1, 0, 1 };
	for (auto i = 0; i < nbUsePerProgram.size(); i++) {
		if (nbUsePerProgram[i] > 0) {
			ASSERT_EQ(ps.nbUsePerProgram.at(progPointers.at(i).get()), nbUsePerProgram[i]);
		}
	}

	std::vector<size_t> nbUsePerTPGTeam{ 1, 2, 1 };
	for (auto i = 0; i < nbUsePerTPGTeam.size(); i++) {
		ASSERT_EQ(ps.nbUsePerTPGTeam.at((const TPG::TPGTeam*)tpg->getVertices().at(i)), nbUsePerTPGTeam[i]);
	}

	std::vector<size_t> nbUsePerTPGAction{ 2, 1, 2 };
	for (auto i = 0; i < nbUsePerTPGAction.size(); i++) {
		ASSERT_EQ(ps.nbUsePerTPGAction.at((const TPG::TPGAction*)tpg->getVertices().at(i + 4)), nbUsePerTPGAction[i]);
	}
}

TEST_F(PolicyStatsTest, Clear) {
	TPG::PolicyStats ps;
	ps.setEnvironment(*e);

	ps.analyzePolicy(tpg->getVertices().at(0));

	ASSERT_NO_THROW(ps.clear());

	// Check analysis results
	ASSERT_EQ(ps.maxPolicyDepth, 0);
	ASSERT_EQ(ps.nbDistinctTeams, 0);

	ASSERT_TRUE(ps.nbTPGVertexPerDepthLevel.empty());
	ASSERT_TRUE(ps.nbLinesPerProgram.empty());
	ASSERT_TRUE(ps.nbIntronPerProgram.empty());
	ASSERT_TRUE(ps.nbOutgoingEdgesPerTeam.empty());
	ASSERT_TRUE(ps.nbUsagePerActionID.empty());
	ASSERT_TRUE(ps.nbUsagePerInstruction.empty());
	ASSERT_TRUE(ps.nbUsagePerDataLocation.empty());
	ASSERT_TRUE(ps.nbUsePerProgram.empty());
	ASSERT_TRUE(ps.nbUsePerTPGTeam.empty());
	ASSERT_TRUE(ps.nbUsePerTPGAction.empty());
}

TEST_F(PolicyStatsTest, InsertOperator) {
	TPG::PolicyStats ps;
	ps.setEnvironment(*e);

	ps.analyzePolicy(tpg->getVertices().at(0));

	std::stringstream ss;
	ASSERT_NO_THROW(ss << ps);
	// Do not check the result of the insert operation since it is subject to change.
}