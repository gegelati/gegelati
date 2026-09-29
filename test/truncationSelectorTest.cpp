
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

TEST(TruncationSelectorTest, select)
{
    CounterReset::counterReset();
    Selection::TruncationSelector selection;
    Representations::FakeRepresentation rep;
    RNG::RNG rng;

    // Create 200 individuals with scores 0, 1, 2, ..., 199.
    std::vector<std::pair<double, std::shared_ptr<const Individual>>> populationScores;
    for(size_t idx = 0; idx < 200; idx++) {
        std::shared_ptr<const Individual> indiv = std::make_shared<Individual>(rep);
        populationScores.push_back(std::make_pair(double(199 - idx), indiv));
    }


    std::vector<std::shared_ptr<const Individual>> selectionResults;
    ASSERT_NO_THROW(selectionResults = selection.select(populationScores, 100, rng)) << "Selecting individuals failed";
    ASSERT_EQ(selectionResults.size(), 100) << "There should be 200 selection results";

    for(size_t idx = 0; idx < 100; idx++) {
        ASSERT_EQ(selectionResults.at(idx)->getIndividualID(), idx) << "ID should be the same";
    }
}