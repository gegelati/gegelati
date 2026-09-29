/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2024 - 2025)
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

#include "data/hash.h"
#include "evaluation/reinforcementEnvironment.h"
#include "dimensions/numericRange.h"
#include "rng/rng.h"

/**
 * Play the stick game against a random player
 */
class StickGameWithOpponent : public Evaluation::ReinforcementEnvironment
{
  protected:
    /// During a game, number of remaining sticks.
    double remainingSticks;

    /// Did the player win or lose
    bool win;

    /// Did the player attempt a forbidden move (i.e. removing more sticks than
    /// available)
    bool forbiddenMove;

    /// Randomness control
    RNG::RNG rng;

  public:
    /**
     * Constructor.
     */
    StickGameWithOpponent()
        : Evaluation::ReinforcementEnvironment(
            {Dimensions::Requirement::array1d<int>(3), Dimensions::Requirement::scalar<double>()}, 
             Dimensions::Requirement::scalar<size_t>(Dimensions::NumericRange<size_t>::atMost(2))), win{false}
    {
        this->dataSources.push_back(Data::DataValue::array1d<int[3]>({1, 2, 3}));
        this->dataSources.push_back(Data::DataValue::scalar(remainingSticks));
        this->reset(0);
    };

    /// Destructor
    ~StickGameWithOpponent(){};

    // Inherited via Problem
    virtual bool isCopyable() const override;

    // Inherited via Problem
    virtual std::unique_ptr<ReinforcementEnvironment> cloneUniquePtr() const override;

    // Inherited via Problem
    virtual void doAction(const Data::DataValue& action) override;

    // Inherited via Problem
    virtual void reset(size_t seed = 0) override;

    /**
     * Returns 1.0 when the player won, 0.0 otherwise.
     */
    virtual double getLastReward() const override;

    // Inherited via Problem
    virtual bool isTerminal() const override;
};

#endif
