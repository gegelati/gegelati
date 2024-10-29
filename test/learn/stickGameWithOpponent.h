/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2024) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2024)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

#ifndef STICK_GAME_WITH_OPPONENT_H
#define STICK_GAME_WITH_OPPONENT_H

#include <random>

#include "data/primitiveTypeArray.h"
#include "learn/learningEnvironment.h"
#include "mutator/rng.h"

/**
 * Play the stick game against a random player
 */
class StickGameWithOpponent : public Learn::LearningEnvironment
{
  protected:
    /// During a game, number of remaining sticks.
    Data::PrimitiveTypeArray<int> remainingSticks;

    /// This source of data give useful numbers for helping undertanding the
    /// game.
    Data::PrimitiveTypeArray<int> hints;

    /// Did the player win or lose
    bool win;

    /// Did the player attempt a forbidden move (i.e. removing more sticks than
    /// available)
    bool forbiddenMove;

    /// Randomness control
    Mutator::RNG rng;

  public:
    /**
     * Constructor.
     */
    StickGameWithOpponent()
        : LearningEnvironment(3), remainingSticks(1), hints(3), win{false}
    {
        this->reset(0);
        // Set hints
        this->hints.setDataAt(typeid(int), 0, 1);
        this->hints.setDataAt(typeid(int), 1, 2);
        this->hints.setDataAt(typeid(int), 2, 3);
    };

    /// Destructor
    ~StickGameWithOpponent(){};

    // Inherited via LearningEnvironment
    virtual bool isCopyable() const override;

    // Inherited via LearningEnvironment
    virtual LearningEnvironment* clone() const override;

    // Inherited via LearningEnvironment
    virtual void doAction(uint64_t actionID) override;

    // Inherited via LearningEnvironment
    virtual void reset(size_t seed = 0,
                       Learn::LearningMode mode = Learn::LearningMode::TRAINING,
                       uint16_t iterationNumber = 0,
                       uint64_t generationNumber = 0) override;

    // Inherited via LearningEnvironment
    virtual std::vector<std::reference_wrapper<const Data::DataHandler>>
    getDataSources() override;

    /**
     * Returns 1.0 when the player won, 0.0 otherwise.
     */
    virtual double getScore() const override;

    // Inherited via LearningEnvironment
    virtual bool isTerminal() const override;
};

#endif
