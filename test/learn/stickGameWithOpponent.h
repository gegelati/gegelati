#ifndef STICK_GAME_WITH_OPPONENT_H
#define STICK_GAME_WITH_OPPONENT_H

#include <random>

#include "mutator/rng.h"
#include "data/primitiveTypeArray.h"
#include "learn/learningEnvironment.h"

/**
* Play the stick game against a random player
*/
class StickGameWithOpponent : public Learn::LearningEnvironment {
protected:
	/// During a game, number of remaining sticks.
	Data::PrimitiveTypeArray<int> remainingSticks;

	/// This source of data give useful numbers for helping undertanding the game.
	Data::PrimitiveTypeArray<int> hints;

	/// Did the player win or lose
	bool win;

	/// Did the player attempt a forbidden move (i.e. removing more sticks than available)
	bool forbiddenMove;

	/// Randomness control
	Mutator::RNG rng;

public:

	/**
	* Constructor.
	*/
	StickGameWithOpponent() : LearningEnvironment(3), remainingSticks(1), hints(3), win{ false }{
		this->reset(0);
		// Set hints
		this->hints.setDataAt(typeid(int), 0, 1);
		this->hints.setDataAt(typeid(int), 1, 2);
		this->hints.setDataAt(typeid(int), 2, 3);
	};

	/// Destructor
	~StickGameWithOpponent() {};

	// Inherited via LearningEnvironment
	virtual bool isCopyable() const override;

	// Inherited via LearningEnvironment
	virtual LearningEnvironment* clone() const override;

	// Inherited via LearningEnvironment
	virtual void doAction(uint64_t actionID) override;

	// Inherited via LearningEnvironment
	virtual void reset(size_t seed = 0, Learn::LearningMode mode = Learn::LearningMode::TRAINING) override;

	// Inherited via LearningEnvironment
	virtual std::vector<std::reference_wrapper<const Data::DataHandler>> getDataSources() override;

	/**
	* Returns 1.0 when the player won, 0.0 otherwise.
	*/
	virtual double getScore() const override;

	// Inherited via LearningEnvironment
	virtual bool isTerminal() const override;

};

#endif
