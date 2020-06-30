/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
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

#include <gtest/gtest.h>

#include "learn/learningEnvironment.h"
#include "learn/stickGameWithOpponent.h"

TEST(LearningEnvironmentTest, Constructor)
{
    Learn::LearningEnvironment* le = NULL;

    ASSERT_NO_THROW(le = new StickGameWithOpponent())
        << "Construction of the Learning Environment failed";

    ASSERT_NO_THROW(delete le)
        << "Destruction of the Learning Environment failed";
}

// Create a fake LearningEnvironment for testing purpose.
class FakeLearningEnvironment : public Learn::LearningEnvironment
{
    Data::PrimitiveTypeArray<int> data;

  public:
    FakeLearningEnvironment() : LearningEnvironment(2), data(3){};
    void reset(size_t seed, Learn::LearningMode mode){};
    std::vector<std::reference_wrapper<const Data::DataHandler>>
    getDataSources()
    {
        std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
        vect.push_back(data);
        return vect;
    }
    double getScore() const
    {
        return 0.0;
    }
    bool isTerminal() const
    {
        return false;
    }
};

TEST(LearningEnvironmentTest, Clonable)
{
    Learn::LearningEnvironment* le = new FakeLearningEnvironment();

    ASSERT_FALSE(le->isCopyable())
        << "Default behavior of isCopyable is false.";
    ASSERT_EQ(le->clone(), (Learn::LearningEnvironment*)NULL)
        << "Default behavior of clone is NULL.";

    // for code coverage
    le->reset();
    le->getDataSources();
    le->getScore();
    le->isTerminal();

    delete le;
}

TEST(LearningEnvironmentTest, getNbAction)
{
    StickGameWithOpponent le;

    ASSERT_EQ(le.getNbActions(), 3) << "Number of action is incorrect";
}

TEST(LearningEnvironmentTest, getDataSource)
{
    StickGameWithOpponent le;

    std::vector<std::reference_wrapper<const Data::DataHandler>> dataSrc;
    ASSERT_NO_THROW(dataSrc = le.getDataSources())
        << "Getting data sources should not fail";
    ASSERT_EQ(dataSrc.size(), 2) << "Number of dataSource is incorrect";

    // Check initial number of sticks
    int initNr = (const int)*(dataSrc.at(1)
                                  .get()
                                  .getDataAt(typeid(int), 0)
                                  .getSharedPointer<const int>());
    ASSERT_EQ(initNr, 21) << "Initial number of stick is incorrect";
}

TEST(LearningEnvironmentTest, doAction)
{
    StickGameWithOpponent le;

    ASSERT_NO_THROW(le.doAction(1))
        << "Remove 2 stick after game init should not fail.";
    std::shared_ptr<const int> nbSticks =
        (le.getDataSources().at(1).get().getDataAt(typeid(int), 0))
            .getSharedPointer<const int>();
    // Remove 2 sticks brings us to 19 sticks
    // Other player removes between 1 and 3 sticks
    // thus, number of remaining sticks is within 18 and 16
    ASSERT_TRUE(*nbSticks <= 18 && *nbSticks >= 16)
        << "Number of stick remaining after one action is not within expected "
           "range.";

    // Check the illegal action
    ASSERT_THROW(le.doAction(3), std::runtime_error)
        << "Illegal action not detected as such.";
}

TEST(LearningEnvironmentTest, getScoreAndIsTerminal)
{
    StickGameWithOpponent le;

    ASSERT_EQ(le.getScore(), 0.0)
        << "Score should be zero until the game is over";

    // Play the full game and lose with known seed (0)
    std::vector<int> actions = {0, 1, 2, 1, 2, 0};
    for (auto& action : actions) {
        ASSERT_FALSE(le.isTerminal())
            << "With a known seed and action sequence, the game should not be "
               "over.";
        le.doAction(action);
    }

    ASSERT_TRUE(le.isTerminal())
        << "With a known seed and action sequence, the game should be over.";
    ASSERT_EQ(le.getScore(), 0.0) << "Score when losing the game should be 0.";

    le.reset(0);
    actions = {0, 1, 2, 2, 1, 2};
    for (auto& action : actions) {
        ASSERT_FALSE(le.isTerminal())
            << "With a known seed and action sequence, the game should not be "
               "over.";
        le.doAction(action);
    }
    ASSERT_TRUE(le.isTerminal())
        << "With a known seed and action sequence, the game should be over.";
    ASSERT_EQ(le.getScore(), -1.0)
        << "Score when losing the game with an illegal action should be -1.0.";

    le.reset(0);
    actions = {0, 1, 2, 2, 0, 0};
    for (auto action : actions) {
        ASSERT_FALSE(le.isTerminal())
            << "With a known seed and action sequence, the game should not be "
               "over.";
        le.doAction(action);
    }
    ASSERT_TRUE(le.isTerminal())
        << "With a known seed and action sequence, the game should be over.";
    ASSERT_EQ(le.getScore(), 1.0)
        << "Score when winning the game should be 1.0.";
}
