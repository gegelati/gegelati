
#include <gtest/gtest.h>

#include "selection/truncationSelector.h"


TEST(TruncationSelectorTest, Constructor)
{
    Selection::TruncationSelector* selection;

    ASSERT_NO_THROW(selection = new Selection::TruncationSelector()) << "Constructor of SurvivingSelection failed.";

    ASSERT_NO_THROW(delete selection) << "Destructor of SurvivingSelection failed.";
}

TEST(SelectorTest, getOrderedFitness)
{
    // 200 individuals with IDs 0, 1, 2, ..., 199 and scores 0, 1, 2, ..., 199.
    std::map<uint64_t, double> populationScores;
    for(size_t idx = 0; idx < 200; idx++) {
        populationScores.insert(std::make_pair(idx, double(idx)));
    }

    ASSERT_NO_THROW(Selection::Selector::getOrderedFitness(populationScores, true)) << "Ordering fitness failed";
    std::vector<std::pair<uint64_t, double>> descendingOrder = Selection::Selector::getOrderedFitness(populationScores, false);

    for(size_t idx = 0; idx < 200; idx++) {
        EXPECT_EQ(descendingOrder.at(idx).first, 199 - idx) << "ID should be the same";
        EXPECT_EQ(descendingOrder.at(idx).second, double(199 - idx)) << "Score should be the same";
    }

    std::vector<std::pair<uint64_t, double>> ascendingOrder = Selection::Selector::getOrderedFitness(populationScores, true);

    for(size_t idx = 0; idx < 200; idx++) {
        ASSERT_EQ(ascendingOrder.at(idx).first, idx) << "ID should be the same";
        ASSERT_EQ(ascendingOrder.at(idx).second, double(idx)) << "Score should be the same";
    }
}

TEST(SelectorTest, getOrderedFitnessEqualScores)
{
    // With the same score, the highest ID is considered higher.
    std::map<uint64_t, double> populationScores{{3, 1.0}, {5, 1.0}, {9, 1.0}};

    std::vector<std::pair<uint64_t, double>> descendingOrder = Selection::Selector::getOrderedFitness(populationScores, false);
    EXPECT_EQ(descendingOrder.at(0).first, 3);
    EXPECT_EQ(descendingOrder.at(2).first, 9);

    std::vector<std::pair<uint64_t, double>> ascendingOrder = Selection::Selector::getOrderedFitness(populationScores, true);
    EXPECT_EQ(ascendingOrder.at(0).first, 9);
    EXPECT_EQ(ascendingOrder.at(2).first, 3);
}

TEST(TruncationSelectorTest, select)
{
    Selection::TruncationSelector selection;
    RNG::RNG rng;

    // 200 individuals with IDs 0, 1, 2, ..., 199 and scores 0, 1, 2, ..., 199.
    std::map<uint64_t, double> populationScores;
    for(size_t idx = 0; idx < 200; idx++) {
        populationScores.insert(std::make_pair(idx, double(idx)));
    }

    std::vector<uint64_t> selectionResults;
    ASSERT_NO_THROW(selectionResults = selection.select(populationScores, 100, rng)) << "Selecting individuals failed";
    ASSERT_EQ(selectionResults.size(), 100) << "There should be 100 selection results";

    for(size_t idx = 0; idx < 100; idx++) {
        ASSERT_EQ(selectionResults.at(idx), 199 - idx) << "ID should be the same";
    }

    ASSERT_NO_THROW(selectionResults = selection.select(populationScores, 1000, rng)) << "Selecting individuals failed";
    ASSERT_EQ(selectionResults.size(), 200) << "All the individuals should be selected";
}
