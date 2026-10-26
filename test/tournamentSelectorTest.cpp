#include <gtest/gtest.h>

#include "selection/tournamentSelector.h"
#include "learn/fakeRepresentation.h"
#include "util/counterReset.h"


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
    CounterReset::counterReset();
    Selection::TournamentSelector selection(2, true);
    Representations::FakeRepresentation rep;
    RNG::RNG rng;
    rng.setSeed(0);

    // Create 200 individuals with scores 0, 1, 2, ..., 199.
    std::map<std::shared_ptr<const Individual>, double, SharedLess<Individual>> populationScores;
    for(size_t idx = 0; idx < 200; idx++) {
        std::shared_ptr<const Individual> indiv = std::make_shared<Individual>(rep);
        populationScores.insert(std::make_pair(indiv, double(199 - idx)));
    }

    std::vector<std::shared_ptr<const Individual>> selectionResults;
    ASSERT_NO_THROW(selectionResults = selection.select(populationScores, 100, rng))
        << "Selecting individuals failed";

    ASSERT_EQ(selectionResults.size(), 100)
        << "There should be 100 selection results";

    for(size_t idx = 0; idx < selectionResults.size(); idx++) {
        ASSERT_LT(selectionResults.at(idx)->getIndividualID(), 200)
            << "ID should be below 200";
    }

    // With tournament selection, individuals with higher fitness should be
    // favored, but the same individual can be selected more than once.
    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> setSelection;
    setSelection.insert(selectionResults.begin(), selectionResults.end());

    ASSERT_LE(setSelection.size(), selectionResults.size())
        << "There cannot be more unique individuals than selection results";

    ASSERT_NO_THROW(selectionResults = selection.select(populationScores, 1000, rng))
        << "Selecting individuals failed";

    ASSERT_EQ(selectionResults.size(), 1000)
        << "There should be 1000 selection results";
}


TEST(TournamentSelectorTest, selectWithoutReplacement)
{
    CounterReset::counterReset();
    Selection::TournamentSelector selection(2, false);
    Representations::FakeRepresentation rep;
    RNG::RNG rng;
    rng.setSeed(0);

    // Create 200 individuals with scores 0, 1, 2, ..., 199.
    std::map<std::shared_ptr<const Individual>, double, SharedLess<Individual>> populationScores;
    for(size_t idx = 0; idx < 200; idx++) {
        std::shared_ptr<const Individual> indiv = std::make_shared<Individual>(rep);
        populationScores.insert(std::make_pair(indiv, double(199 - idx)));
    }

    std::vector<std::shared_ptr<const Individual>> selectionResults;
    ASSERT_NO_THROW(selectionResults = selection.select(populationScores, 100, rng))
        << "Selecting individuals failed";

    ASSERT_EQ(selectionResults.size(), 100)
        << "There should be 100 selection results";

    for(size_t idx = 0; idx < selectionResults.size(); idx++) {
        ASSERT_LT(selectionResults.at(idx)->getIndividualID(), 200)
            << "ID should be below 200";
    }

    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> setSelection;
    setSelection.insert(selectionResults.begin(), selectionResults.end());

    ASSERT_EQ(setSelection.size(), 100)
        << "There should be no duplicate selected individuals";

    // Selecting the whole population without replacement should succeed.
    ASSERT_THROW(selectionResults = selection.select(populationScores, 200, rng), std::runtime_error)
        << "Selecting individuals should failed";
}


TEST(TournamentSelectorTest, selectBestIndividual)
{
    CounterReset::counterReset();
    Representations::FakeRepresentation rep;
    RNG::RNG rng;
    rng.setSeed(0);

    // Create 200 individuals with scores 0, 1, 2, ..., 199.
    std::map<std::shared_ptr<const Individual>, double, SharedLess<Individual>> populationScores;
    for(size_t idx = 0; idx < 200; idx++) {
        std::shared_ptr<const Individual> indiv = std::make_shared<Individual>(rep);
        populationScores.insert(std::make_pair(indiv, double(idx)));
    }

    // A tournament containing the entire population must select the
    // individual with the highest fitness.
    Selection::TournamentSelector selection(200, true);

    std::vector<std::shared_ptr<const Individual>> selectionResults;
    ASSERT_NO_THROW(selectionResults = selection.select(populationScores, 10, rng))
        << "Selecting individuals failed";

    ASSERT_EQ(selectionResults.size(), 10)
        << "There should be 10 selection results";

    for(size_t idx = 0; idx < selectionResults.size(); idx++) {
        ASSERT_EQ(selectionResults.at(idx)->getIndividualID(), 199)
            << "The individual with the highest fitness should always be selected";
    }
}


TEST(TournamentSelectorTest, selectIsDeterminist)
{
    CounterReset::counterReset();
    Selection::TournamentSelector selection(2, true);
    Representations::FakeRepresentation rep;
    RNG::RNG rng;
    rng.setSeed(0);

    // Create 200 individuals with identical identities in both maps,
    // but different fitness values.
    std::map<std::shared_ptr<const Individual>, double, SharedLess<Individual>> populationScores1;
    std::map<std::shared_ptr<const Individual>, double, SharedLess<Individual>> populationScores2;

    for(size_t idx = 0; idx < 200; idx++) {
        std::shared_ptr<const Individual> indiv = std::make_shared<Individual>(rep);
        populationScores1.insert(std::make_pair(indiv, double(199 - idx)));
        populationScores2.insert(std::make_pair(indiv, rng.uniformSample<double>(-1000.0, 1000.0)));
    }

    rng.setSeed(0);
    std::vector<std::shared_ptr<const Individual>> selectionResults1;
    ASSERT_NO_THROW(selectionResults1 = selection.select(populationScores1, 100, rng))
        << "Selecting individuals failed";

    rng.setSeed(0);
    std::vector<std::shared_ptr<const Individual>> selectionResults2;
    ASSERT_NO_THROW(selectionResults2 = selection.select(populationScores1, 100, rng))
        << "Selecting individuals failed";

    for(size_t idx = 0; idx < selectionResults1.size(); idx++) {
        ASSERT_EQ(selectionResults1.at(idx), selectionResults2.at(idx))
            << "Selection should be deterministic for the same seed and fitness map";
    }
}


TEST(TournamentSelectorTest, InvalidTournamentSize)
{
    CounterReset::counterReset();
    Representations::FakeRepresentation rep;
    RNG::RNG rng;

    std::map<std::shared_ptr<const Individual>, double, SharedLess<Individual>> populationScores;
    for(size_t idx = 0; idx < 10; idx++) {
        std::shared_ptr<const Individual> indiv = std::make_shared<Individual>(rep);
        populationScores.insert(std::make_pair(indiv, double(idx)));
    }

    Selection::TournamentSelector selectionTooLarge(11, true);

    ASSERT_THROW(selectionTooLarge.select(populationScores, 1, rng), std::runtime_error)
        << "Tournament size larger than the population should throw an exception";

    Selection::TournamentSelector selectionTooSmall(1, true);

    ASSERT_THROW(selectionTooSmall.select(populationScores, 1, rng), std::runtime_error)
        << "Tournament size smaller than 2 should throw an exception";
}