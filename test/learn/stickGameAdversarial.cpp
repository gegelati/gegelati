/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
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

#include "stickGameAdversarial.h"

bool StickGameAdversarial::isCopyable() const
{
    return true;
}

Learn::LearningEnvironment* StickGameAdversarial::clone() const
{
    // Default copy constructor does the trick.
    return (Learn::LearningEnvironment*)new StickGameAdversarial(*this);
}

void StickGameAdversarial::doAction(uint64_t actionID)
{
    LearningEnvironment::doAction(actionID);

    // gets the information of which player currently plays
    bool isFirstPlayer = turn % 2 == 0;
    // set a reference to update the correct forbiddenMove if there is one
    bool& forbiddenMove =
        isFirstPlayer ? forbiddenMoveFirstPlayer : forbiddenMoveSecondPlayer;

    // if the game is not over
    if (!this->isTerminal()) {
        // Execute the action
        // Get current state
        int currentState =
            (int)*(((this->remainingSticks.getDataAt(typeid(int), 0))
                        .getSharedPointer<const int>()));
        if ((actionID + 1) > currentState) {
            // Illegal move
            forbiddenMove = true;
            // and game over
            this->remainingSticks.setDataAt(typeid(int), 0, 0);
            firstPlayerWon = !isFirstPlayer;

            // stop there
            return;
        }
        else {
            // update state
            currentState -= ((int)actionID + 1);
            this->remainingSticks.setDataAt(typeid(int), 0, currentState);
            // if current state is now zero, the player lost
            if (currentState == 0) {
                firstPlayerWon = !isFirstPlayer;
            }
            turn++;
        }
    }
}

void StickGameAdversarial::reset(size_t seed, Learn::LearningMode mode,
                                 uint16_t iterationNumber,
                                 uint64_t generationNumber)
{
    // Create seed from seed and mode
    size_t hash_seed =
        Data::Hash<size_t>()(seed) ^ Data::Hash<Learn::LearningMode>()(mode);
    this->rng.setSeed(hash_seed);
    this->remainingSticks.setDataAt(typeid(int), 0, 21);
    this->firstPlayerWon = false;
    this->forbiddenMoveFirstPlayer = false;
    this->forbiddenMoveSecondPlayer = false;
    this->turn = 0;
}

std::vector<std::reference_wrapper<const Data::DataHandler>>
StickGameAdversarial::getDataSources()
{
    std::vector<std::reference_wrapper<const Data::DataHandler>> res = {
        this->hints, this->remainingSticks};

    return res;
}

std::shared_ptr<Learn::AdversarialEvaluationResult> StickGameAdversarial::
    getScores() const
{
    double scoreFirst;
    double scoreSecond;
    if (this->firstPlayerWon) {
        scoreFirst = 1.0;
        if (!this->forbiddenMoveSecondPlayer) {
            scoreSecond = 0.0;
        }
        else {
            scoreSecond = -1.0;
        }
    }
    else {
        scoreSecond = 1.0;
        if (!this->forbiddenMoveFirstPlayer) {
            scoreFirst = 0.0;
        }
        else {
            scoreFirst = -1.0;
        }
    }

    return std::make_shared<Learn::AdversarialEvaluationResult>(
        Learn::AdversarialEvaluationResult({scoreFirst, scoreSecond}));
}

bool StickGameAdversarial::isTerminal() const
{
    return (int)*((this->remainingSticks.getDataAt(typeid(int), 0))
                      .getSharedPointer<const int>()) == 0;
}
