#include <gtest/gtest.h>
#include <set>

#include "selection/tournamentSelector.h"


TEST(TournamentSelectorTest, Constructor)
{
    Selection::TournamentSelector* selection;

    ASSERT_NO_THROW(selection = new Selection::TournamentSelector(2, true))
        << "Constructor of TournamentSelector failed.";

    ASSERT_NO_THROW(delete selection)
        << "Destructor of TournamentSelector failed.";
}


TEST(TournamentSelectorTest, selectWithReplacement)
{
    Selection::TournamentSelector selection(2, true);
    RNG::RNG rng;
    rng.setSeed(0);

    // 200 individuals with IDs 0, 1, 2, ..., 199 and scores 199, 198, ..., 0.
    std::map<uint64_t, double> populationScores;
    for(size_t idx = 0; idx < 200; idx++) {
        populationScores.insert(std::make_pair(idx, double(199 - idx)));
    }

    std::vector<uint64_t> selectionResults;
    ASSERT_NO_THROW(selectionResults = selection.select(populationScores, 100, rng))
        << "Selecting individuals failed";

    ASSERT_EQ(selectionResults.size(), 100)
        << "There should be 100 selection results";

    for(size_t idx = 0; idx < selectionResults.size(); idx++) {
        ASSERT_LT(selectionResults.at(idx), 200)
            << "ID should be below 200";
    }

    // With tournament selection, individuals with higher fitness should be
    // favored, but the same individual can be selected more than once.
    std::set<uint64_t> setSelection(selectionResults.begin(), selectionResults.end());

    ASSERT_LE(setSelection.size(), selectionResults.size())
        << "There cannot be more unique individuals than selection results";

    ASSERT_NO_THROW(selectionResults = selection.select(populationScores, 1000, rng))
        << "Selecting individuals failed";

    ASSERT_EQ(selectionResults.size(), 1000)
        << "There should be 1000 selection results";
}


TEST(TournamentSelectorTest, selectWithoutReplacement)
{
    Selection::TournamentSelector selection(2, false);
    RNG::RNG rng;
    rng.setSeed(0);

    // 200 individuals with IDs 0, 1, 2, ..., 199 and scores 199, 198, ..., 0.
    std::map<uint64_t, double> populationScores;
    for(size_t idx = 0; idx < 200; idx++) {
        populationScores.insert(std::make_pair(idx, double(199 - idx)));
    }

    std::vector<uint64_t> selectionResults;
    ASSERT_NO_THROW(selectionResults = selection.select(populationScores, 100, rng))
        << "Selecting individuals failed";

    ASSERT_EQ(selectionResults.size(), 100)
        << "There should be 100 selection results";

    for(size_t idx = 0; idx < selectionResults.size(); idx++) {
        ASSERT_LT(selectionResults.at(idx), 200)
            << "ID should be below 200";
    }

    std::set<uint64_t> setSelection(selectionResults.begin(), selectionResults.end());

    ASSERT_EQ(setSelection.size(), 100)
        << "There should be no duplicate selected individuals";

    // Selecting the whole population without replacement should fail: tournaments get too large.
    ASSERT_THROW(selectionResults = selection.select(populationScores, 200, rng), std::runtime_error)
        << "Selecting individuals should failed";
}


TEST(TournamentSelectorTest, selectBestIndividual)
{
    RNG::RNG rng;
    rng.setSeed(0);

    // 200 individuals with IDs 0, 1, 2, ..., 199 and scores 0, 1, 2, ..., 199.
    std::map<uint64_t, double> populationScores;
    for(size_t idx = 0; idx < 200; idx++) {
        populationScores.insert(std::make_pair(idx, double(idx)));
    }

    // A tournament containing the entire population must select the
    // individual with the highest fitness.
    Selection::TournamentSelector selection(200, true);

    std::vector<uint64_t> selectionResults;
    ASSERT_NO_THROW(selectionResults = selection.select(populationScores, 10, rng))
        << "Selecting individuals failed";

    ASSERT_EQ(selectionResults.size(), 10)
        << "There should be 10 selection results";

    for(size_t idx = 0; idx < selectionResults.size(); idx++) {
        ASSERT_EQ(selectionResults.at(idx), 199)
            << "The individual with the highest fitness should always be selected";
    }
}


TEST(TournamentSelectorTest, selectIsDeterminist)
{
    Selection::TournamentSelector selection(2, true);
    RNG::RNG rng;

    // 200 individuals with scores 199, 198, ..., 0.
    std::map<uint64_t, double> populationScores;
    for(size_t idx = 0; idx < 200; idx++) {
        populationScores.insert(std::make_pair(idx, double(199 - idx)));
    }

    rng.setSeed(0);
    std::vector<uint64_t> selectionResults1;
    ASSERT_NO_THROW(selectionResults1 = selection.select(populationScores, 100, rng))
        << "Selecting individuals failed";

    rng.setSeed(0);
    std::vector<uint64_t> selectionResults2;
    ASSERT_NO_THROW(selectionResults2 = selection.select(populationScores, 100, rng))
        << "Selecting individuals failed";

    ASSERT_EQ(selectionResults1, selectionResults2)
        << "Selection should be deterministic for the same seed and fitness map";
}


TEST(TournamentSelectorTest, InvalidTournamentSize)
{
    RNG::RNG rng;

    std::map<uint64_t, double> populationScores;
    for(size_t idx = 0; idx < 10; idx++) {
        populationScores.insert(std::make_pair(idx, double(idx)));
    }

    Selection::TournamentSelector selectionTooLarge(11, true);

    ASSERT_THROW(selectionTooLarge.select(populationScores, 1, rng), std::runtime_error)
        << "Tournament size larger than the population should throw an exception";

    Selection::TournamentSelector selectionTooSmall(1, true);

    ASSERT_THROW(selectionTooSmall.select(populationScores, 1, rng), std::runtime_error)
        << "Tournament size smaller than 2 should throw an exception";
}
