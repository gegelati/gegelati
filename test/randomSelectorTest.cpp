
#include <gtest/gtest.h>
#include <set>

#include "selection/randomSelector.h"


TEST(RandomSelectorTest, Constructor)
{
    Selection::RandomSelector* selection;

    ASSERT_NO_THROW(selection = new Selection::RandomSelector(true)) << "Constructor of SurvivingSelection failed.";

    ASSERT_NO_THROW(delete selection) << "Destructor of SurvivingSelection failed.";
}

TEST(RandomSelectorTest, selectWithoutReplacement)
{
    Selection::RandomSelector selection(false);
    RNG::RNG rng;
    rng.setSeed(0);

    // 200 individuals with IDs 0, 1, 2, ..., 199 and scores 199, 198, ..., 0.
    std::map<uint64_t, double> populationScores;
    for(size_t idx = 0; idx < 200; idx++) {
        populationScores.insert(std::make_pair(idx, double(199 - idx)));
    }

    std::vector<uint64_t> selectionResults;
    ASSERT_NO_THROW(selectionResults = selection.select(populationScores, 100, rng)) << "Selecting individuals failed";
    ASSERT_EQ(selectionResults.size(), 100) << "There should be 100 selection results";

    for(size_t idx = 0; idx < 100; idx++) {
        ASSERT_LT(selectionResults.at(idx), 200) << "ID should be below 200";
    }

    std::set<uint64_t> setSelection(selectionResults.begin(), selectionResults.end());
    ASSERT_EQ(setSelection.size(), 100) << "There should be 100 selection results";

    ASSERT_NO_THROW(selectionResults = selection.select(populationScores, 1000, rng)) << "Selecting individuals failed";
    ASSERT_EQ(selectionResults.size(), 200) << "There should be 200 selection results";
}

TEST(RandomSelectorTest, selectWithReplacement) 
{
    Selection::RandomSelector selection(true);
    RNG::RNG rng;
    rng.setSeed(0);

    // 200 individuals with IDs 0, 1, 2, ..., 199 and scores 199, 198, ..., 0.
    std::map<uint64_t, double> populationScores;
    for(size_t idx = 0; idx < 200; idx++) {
        populationScores.insert(std::make_pair(idx, double(199 - idx)));
    }

    std::vector<uint64_t> selectionResults;
    ASSERT_NO_THROW(selectionResults = selection.select(populationScores, 100, rng)) << "Selecting individuals failed";
    ASSERT_EQ(selectionResults.size(), 100) << "There should be 100 selection results";

    for(size_t idx = 0; idx < 100; idx++) {
        ASSERT_LT(selectionResults.at(idx), 200) << "ID should be below 200";
    }

    std::set<uint64_t> setSelection(selectionResults.begin(), selectionResults.end());
    ASSERT_LT(setSelection.size(), 100) << "It is very very very unlickly that no individual has been sampled twice!";

    ASSERT_NO_THROW(selectionResults = selection.select(populationScores, 1000, rng)) << "Selecting individuals failed";
    ASSERT_EQ(selectionResults.size(), 1000) << "There should be 1000 selection results";
}

TEST(RandomSelectorTest, selectIsDeterminist) 
{
    Selection::RandomSelector selection(true);
    RNG::RNG rng;
    rng.setSeed(0);

    // Same individuals with scores 199, 198, ..., 0 in the first map, and random scores in the second one.
    std::map<uint64_t, double> populationScores1;
    std::map<uint64_t, double> populationScores2;
    for(size_t idx = 0; idx < 200; idx++) {
        populationScores1.insert(std::make_pair(idx, double(199 - idx)));
        populationScores2.insert(std::make_pair(idx, rng.uniformSample<double>(-1000.0, 1000.0)));
    }
    
    rng.setSeed(0);
    std::vector<uint64_t> selectionResults1;
    ASSERT_NO_THROW(selectionResults1 = selection.select(populationScores1, 100, rng)) << "Selecting individuals failed";

    rng.setSeed(0);
    std::vector<uint64_t> selectionResults2;
    ASSERT_NO_THROW(selectionResults2 = selection.select(populationScores2, 100, rng)) << "Selecting individuals failed";

    ASSERT_EQ(selectionResults1, selectionResults2) << "Order should be exactly the same: fitness is ignored!";
}
