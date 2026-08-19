
#include <gtest/gtest.h>

#include "evolution/survivingSelection.h"
#include "util/counterReset.h"

class SurvivingSelectionTest : public ::testing::Test
{
  protected:

    virtual void SetUp()
    {
        CounterReset::counterReset();
    }

    virtual void TearDown()
    {
    }
};

TEST_F(SurvivingSelectionTest, Constructor)
{
    Evolution::SurvivingSelection* selection;

    ASSERT_NO_THROW(selection = new Evolution::SurvivingSelection()) << "Constructor of SurvivingSelection failed.";

    ASSERT_NO_THROW(delete selection) << "Destructor of SurvivingSelection failed.";
}

TEST_F(SurvivingSelectionTest, select)
{
    Evolution::SurvivingSelection selection;

    // Create 200 individuals with scores 0, 1, 2, ..., 199.
    std::vector<Evolution::Individual*> indivs;
    std::map<std::reference_wrapper<const Evolution::Individual>, std::shared_ptr<Learn::EvaluationResult>> scores;
    for(size_t idx = 0; idx < 200; idx++) {
        indivs.push_back(new Evolution::Individual());
        
        std::shared_ptr<Learn::EvaluationResult> result = std::make_shared<Learn::EvaluationResult>(
            std::make_shared<Selector::SelectionMetrics>(double(idx)), 1
        );
        scores.insert({*indivs.back(), result});
    }

    std::map<std::reference_wrapper<const Evolution::Individual>, bool> selectionResults;
    ASSERT_NO_THROW(selectionResults = selection.select(scores)) << "Selecting individuals failed";
    ASSERT_EQ(selectionResults.size(), 200) << "There should be 200 selection results";

    for(auto it = selectionResults.begin(); it != selectionResults.end(); it++) {
        if(it->second) {
            ASSERT_GE(it->first.get().getIndividualID(), 100) << "Individual ID shoudl be in [100, 199] with set score";
        } else {
            ASSERT_LT(it->first.get().getIndividualID(), 100) << "Individual ID should be in [0, 99] with set score";
        }
    }

    // Delete ptr
    for(size_t idx = 0; idx < 200; idx++) {
        delete indivs.at(idx);
    }
}

TEST_F(SurvivingSelectionTest, getBest)
{
    Evolution::SurvivingSelection selection;

    // Create 200 individuals with scores 0, 1, 2, ..., 199.
    std::vector<Evolution::Individual*> indivs;
    std::map<std::reference_wrapper<const Evolution::Individual>, std::shared_ptr<Learn::EvaluationResult>> scores;
    for(size_t idx = 0; idx < 200; idx++) {
        indivs.push_back(new Evolution::Individual());
        
        std::shared_ptr<Learn::EvaluationResult> result = std::make_shared<Learn::EvaluationResult>(
            std::make_shared<Selector::SelectionMetrics>(double(idx)), 1
        );
        scores.insert({*indivs.back(), result});
    }

    std::pair<std::reference_wrapper<const Evolution::Individual>, std::shared_ptr<Learn::EvaluationResult>> bestPair = *scores.begin();
    ASSERT_NO_THROW(bestPair = selection.getBest(scores)) << "Getting best score failed";
    ASSERT_TRUE(bestPair.first == *indivs.back()) << "Best individual should be last indiv of the vector";
    ASSERT_TRUE(bestPair.second->getSelectionMetrics()->getScore() == 199) << "Best score should be 199";


    // Delete ptr
    for(size_t idx = 0; idx < 200; idx++) {
        delete indivs.at(idx);
    }
}