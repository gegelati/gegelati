
#include <gtest/gtest.h>

#include "selection/truncationSelector.h"
#include "learn/fakeRepresentation.h"
#include "util/counterReset.h"


TEST(TruncationSelectorTest, Constructor)
{
    Selection::TruncationSelector* selection;

    ASSERT_NO_THROW(selection = new Selection::TruncationSelector()) << "Constructor of SurvivingSelection failed.";

    ASSERT_NO_THROW(delete selection) << "Destructor of SurvivingSelection failed.";
}

TEST(SelectorTest, getOrderedFitness)
{
    CounterReset::counterReset();
    Representations::FakeRepresentation rep;
    RNG::RNG rng;

    // Create 200 individuals with scores 0, 1, 2, ..., 199.
    std::map<std::shared_ptr<const Individual>, double, SharedLess<Individual>> populationScores;
    for(size_t idx = 0; idx < 200; idx++) {
        std::shared_ptr<const Individual> indiv = std::make_shared<Individual>(rep);
        populationScores.insert(std::make_pair(indiv, double(idx)));
    }

    ASSERT_NO_THROW(Selection::Selector::getOrderedFitness(populationScores, true)) << "Ordering fitness failed";
    std::vector<std::pair<std::shared_ptr<const Individual>, double>> ascendingOrder = Selection::Selector::getOrderedFitness(populationScores, false);

   
    for(size_t idx = 0; idx < 200; idx++) {
        EXPECT_EQ(ascendingOrder.at(idx).first->getIndividualID(), 199 - idx) << "ID should be the same";
        EXPECT_EQ(ascendingOrder.at(idx).second, double(199 - idx)) << "ID should be the same";
    }

    ASSERT_NO_THROW(Selection::Selector::getOrderedFitness(populationScores, false)) << "Ordering fitness failed";
    std::vector<std::pair<std::shared_ptr<const Individual>, double>> descendingOrder = Selection::Selector::getOrderedFitness(populationScores, true);

    for(size_t idx = 0; idx < 200; idx++) {
        ASSERT_EQ(descendingOrder.at(idx).first->getIndividualID(), idx) << "ID should be the same";
        ASSERT_EQ(descendingOrder.at(idx).second, double(idx)) << "ID should be the same";
    }

}

TEST(TruncationSelectorTest, select)
{
    CounterReset::counterReset();
    Selection::TruncationSelector selection;
    Representations::FakeRepresentation rep;
    RNG::RNG rng;

    // Create 200 individuals with scores 0, 1, 2, ..., 199.
    std::map<std::shared_ptr<const Individual>, double, SharedLess<Individual>> populationScores;
    for(size_t idx = 0; idx < 200; idx++) {
        std::shared_ptr<const Individual> indiv = std::make_shared<Individual>(rep);
        populationScores.insert(std::make_pair(indiv, double(idx)));
    }


    std::vector<std::shared_ptr<const Individual>> selectionResults;
    ASSERT_NO_THROW(selectionResults = selection.select(populationScores, 100, rng)) << "Selecting individuals failed";
    ASSERT_EQ(selectionResults.size(), 100) << "There should be 200 selection results";

    for(size_t idx = 0; idx < 100; idx++) {
        ASSERT_EQ(selectionResults.at(idx)->getIndividualID(), 199 - idx) << "ID should be the same";
    }
}