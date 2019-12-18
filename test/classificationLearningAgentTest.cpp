#include <gtest/gtest.h>

#include "dataHandlers/primitiveTypeArray.h"

#include "instructions/set.h"
#include "instructions/addPrimitiveType.h"

#include "learn/learningEnvironment.h"
#include "learn/learningParameters.h"

#include "learn/classificationLearningAgent.h"

class ClassificationLearningAgentTest : public ::testing::Test {
protected:
	Instructions::Set set;
	Learn::LearningParameters params;

	virtual void SetUp() {
		set.add(*(new Instructions::AddPrimitiveType<int>()));
		set.add(*(new Instructions::AddPrimitiveType<double>()));

		// Proba as in Kelly's paper
		params.mutation.tpg.maxInitOutgoingEdges = 3;
		params.mutation.prog.maxProgramSize = 96;
		params.mutation.tpg.nbRoots = 15;
		params.mutation.tpg.pEdgeDeletion = 0.7;
		params.mutation.tpg.pEdgeAddition = 0.7;
		params.mutation.tpg.pProgramMutation = 0.2;
		params.mutation.tpg.pEdgeDestinationChange = 0.1;
		params.mutation.tpg.pEdgeDestinationIsAction = 0.5;
		params.mutation.prog.pAdd = 0.5;
		params.mutation.prog.pDelete = 0.5;
		params.mutation.prog.pMutate = 1.0;
		params.mutation.prog.pSwap = 1.0;
	}

	virtual void TearDown() {
		delete (&set.getInstruction(0));
		delete (&set.getInstruction(1));
	}
};

// Create a fake LearningEnvironment for testing purpose.
class FakeLearningEnvironment : public Learn::LearningEnvironment {
protected:
	DataHandlers::PrimitiveTypeArray<int> data;

public:
	FakeLearningEnvironment() : LearningEnvironment(2), data(1) {};
	void reset(size_t seed, Learn::LearningMode mode) {};
	std::vector<std::reference_wrapper<const DataHandlers::DataHandler>> getDataSources() {
		std::vector<std::reference_wrapper<const DataHandlers::DataHandler>> vect;
		vect.push_back(data);
		return vect;
	}
	double getScore() const { return 0.0; }
	bool isTerminal() const { return false; }
};

TEST_F(ClassificationLearningAgentTest, Constructor) {
	Learn::ClassificationLearningAgent<Learn::LearningAgent> * cla;

	FakeLearningEnvironment fle;
	cla = new Learn::ClassificationLearningAgent<Learn::LearningAgent>(fle,set,params);
}