#include <iostream>
#include <unordered_set>
#include <numeric>
#include <string>

#include "instructions/addPrimitiveType.h"

#include "learn/learningEnvironment.h"
#include "learn/learningParameters.h"
#include "learn/learningAgent.h"
#include "dataHandlers/dataHandler.h"
#include "dataHandlers/primitiveTypeArray.h"
#include "instructions/instruction.h"
#include "instructions/addPrimitiveType.h"
#include "exporter/tpgGraphDotExporter.h"
#include "instructions/lambdaInstruction.h"
#include "supportedTypes.h"

#include <random>

#include "dataHandlers/primitiveTypeArray.h"
#include "learn/learningEnvironment.h"

/**
* Play the stick game against a random player
*/
class StickGameWithOpponent : public Learn::LearningEnvironment {
protected:
	/// During a game, number of remaining sticks.
	DataHandlers::PrimitiveTypeArray<int> remainingSticks;

	/// This source of data give useful numbers for helping undertanding the game.
	DataHandlers::PrimitiveTypeArray<int> hints;

	/// Did the player win or lose
	bool win;

	/// Did the player attempt a forbidden move (i.e. removing more sticks than available)
	bool forbiddenMove;

	/// Randomness control
	std::mt19937_64 engine;

public:

	/**
	* Constructor.
	*/
	StickGameWithOpponent() : LearningEnvironment(3), remainingSticks(1), hints(4), win{ false }{
		this->reset(0);
		// Set hints
		this->hints.setDataAt(typeid(PrimitiveType<int>), 0, 1);
		this->hints.setDataAt(typeid(PrimitiveType<int>), 1, 2);
		this->hints.setDataAt(typeid(PrimitiveType<int>), 2, 3);
		this->hints.setDataAt(typeid(PrimitiveType<int>), 3, 4);
	};

	/// Destructor
	~StickGameWithOpponent() {};

	// Inherited via LearningEnvironment
	virtual void doAction(uint64_t actionID) override;

	// Inherited via LearningEnvironment
	virtual void reset(size_t seed = 0) override;

	// Inherited via LearningEnvironment
	virtual std::vector<std::reference_wrapper<DataHandlers::DataHandler>> getDataSources() override;

	/**
	* Returns 1.0 when the player won, 0.0 otherwise.
	*/
	virtual double getScore() const override;

	// Inherited via LearningEnvironment
	virtual bool isTerminal() const override;

};

int main() {
	std::cout << "Hello TPG World" << std::endl;

	Instructions::Set set;
	StickGameWithOpponent le;
	Learn::LearningParameters params;


	auto modulo = [](double a, double b)->double {
		if (b != 0.0) { return fmod(a, b); }
		else { return  DBL_MIN; }	};
	auto minus = [](int a, int b)->double {return (double)a - (double)b; };
	auto cast = [](int a, int b)->double {return (double)a; };
	auto add = [](double a, double b)->double {return a + b; };
	auto max = [](double a, double b)->double {return std::max(a, b); };
	auto nulltest = [](double a, double b)->double {return (a == 0.0)? 10.0: 0.0; };


	// set.add(*(new Instructions::AddPrimitiveType<int>()));
	set.add(*(new Instructions::LambdaInstruction<double>(modulo)));
	set.add(*(new Instructions::LambdaInstruction<int>(minus)));
	set.add(*(new Instructions::LambdaInstruction<double>(add)));
	set.add(*(new Instructions::LambdaInstruction<int>(cast)));
	set.add(*(new Instructions::LambdaInstruction<double>(max)));
	set.add(*(new Instructions::LambdaInstruction<double>(nulltest)));


	// Proba as in Kelly's paper
	params.mutation.tpg.maxInitOutgoingEdges = 3;
	params.mutation.prog.maxProgramSize = 20;
	params.mutation.tpg.nbRoots = 100;
	params.mutation.tpg.pEdgeDeletion = 0.7;
	params.mutation.tpg.pEdgeAddition = 0.7;
	params.mutation.tpg.pProgramMutation = 0.2;
	params.mutation.tpg.pEdgeDestinationChange = 0.1;
	params.mutation.tpg.pEdgeDestinationIsAction = 0.5;
	params.mutation.prog.pAdd = 0.5;
	params.mutation.prog.pDelete = 0.5;
	params.mutation.prog.pMutate = 1.0;
	params.mutation.prog.pSwap = 1.0;
	params.archiveSize = 0;
	params.maxNbActionsPerEval = 11;
	params.nbIterationsPerPolicyEvaluation = 100;
	params.ratioDeletedRoots = 0.5;

	Learn::LearningAgent la(le, set, params);
	la.init();

	Exporter::TPGGraphDotExporter dotExporter("out_00.dot", la.getTPGGraph());

	for (int i = 0; i < 300; i++) {
		char buff[11];
		sprintf(buff, "out_%d.dot", i);
		dotExporter.setNewFilePath(buff);
		dotExporter.print();
		std::multimap<double, const TPG::TPGVertex*> result;
		result = la.evaluateAllRoots(0);
		auto iter = result.begin();
		double min = iter->first;
		std::advance(iter, result.size() - 1);
		double max = iter->first;

		double avg = std::accumulate(result.begin(), result.end(), 0.0,
			[](double acc, std::pair<double, const TPG::TPGVertex*> pair)->double {return acc + pair.first; });
		avg /= result.size();
		printf("%2d\t%lld\t%1.2lf\t%1.2lf\t%1.2lf\n", i, la.getTPGGraph().getVertices().size(), min, avg, max);
		//std::cout << i << "\t" << min << "\t" << avg << "\t" << max << std::endl;
		la.trainOneGeneration(i);

	}

	// Keep best policy
	la.keepBestPolicy();
	dotExporter.setNewFilePath("out_best.dot");
	dotExporter.print();

	// cleanup
	delete (&set.getInstruction(0));
	delete (&set.getInstruction(1));
	delete (&set.getInstruction(2));
	delete (&set.getInstruction(3));




	return 0;
}



void StickGameWithOpponent::doAction(uint64_t actionID)
{
	LearningEnvironment::doAction(actionID);

	// if the game is not over
	if (!this->isTerminal()) {
		// Execute the action
		// Get current state
		int currentState = (int)((const PrimitiveType<int>&)(this->remainingSticks.getDataAt(typeid(PrimitiveType<int>), 0)));
		if ((actionID + 1) > currentState) {
			// Illegal move
			this->forbiddenMove = true;
			// and game over
			this->remainingSticks.setDataAt(typeid(PrimitiveType<int>), 0, 0);
			// stop there
			return;
		}
		else {
			// update state
			currentState -= ((int)actionID + 1);
			this->remainingSticks.setDataAt(typeid(PrimitiveType<int>), 0, currentState);
			// if current state is now zero, the player lost
		}

		// Random player's turn
		if (currentState > 0) {
			int errorRate = 1; // error wil be made 1/errorRate of the time
			std::uniform_int_distribution<int> distribution(0, errorRate - 1);
			if (distribution(engine) != 0 && (currentState - 1) % 4 != 0) {
				currentState -= (currentState - 1) % 4;
			}
			else {
				std::uniform_int_distribution<int> distribution(1, std::min(currentState, 3));
				currentState -= distribution(engine);
			}

			this->remainingSticks.setDataAt(typeid(PrimitiveType<int>), 0, currentState);
			if (currentState == 0) {
				this->win = true;
			}
		}
	}
}

void StickGameWithOpponent::reset(size_t seed)
{
	this->engine.seed(seed);
	this->remainingSticks.setDataAt(typeid(PrimitiveType<int>), 0, 20);
	this->win = false;
	this->forbiddenMove = false;
}

std::vector<std::reference_wrapper<DataHandlers::DataHandler>> StickGameWithOpponent::getDataSources()
{
	std::vector<std::reference_wrapper<DataHandlers::DataHandler>> res = { this->hints, this->remainingSticks };

	return res;
}

double StickGameWithOpponent::getScore() const
{
	if (this->win)
	{
		return 1.0;
	}
	else {
		if (!this->forbiddenMove) {
			return 0.0;
		}
		else {
			return -10.0;
		}
	}
}

bool StickGameWithOpponent::isTerminal() const
{
	return (int)((const PrimitiveType<int>&)(this->remainingSticks.getDataAt(typeid(PrimitiveType<int>), 0))) == 0;
}
