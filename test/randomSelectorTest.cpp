
#include <gtest/gtest.h>

#include "selection/randomSelector.h"
#include "learn/fakeRepresentation.h"
#include "util/counterReset.h"

#include "evaluation/scoreMetric.h"

TEST(RandomSelectorTest, Constructor)
{
    Selection::RandomSelector* selection;

    ASSERT_NO_THROW(selection = new Selection::RandomSelector(true)) << "Constructor of SurvivingSelection failed.";

    ASSERT_NO_THROW(delete selection) << "Destructor of SurvivingSelection failed.";
}


TEST(RandomSelectorTest, selectWithoutReplacement)
{
    CounterReset::counterReset();
    Selection::RandomSelector selection(false);
    Representations::FakeRepresentation rep;
    RNG::RNG rng;
    rng.setSeed(0);

    // Create 200 individuals with scores 0, 1, 2, ..., 199.
    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> population;
    for(size_t idx = 0; idx < 200; idx++) {
        std::shared_ptr<const Individual> indiv = std::make_shared<Individual>(rep);
        
        indiv->addEvaluationRun(std::move(std::make_unique<Evaluation::EvaluationRun>(
                                      std::move(std::make_unique<Evaluation::ScoreMetric>(double(idx))))), 1);
        population.insert(indiv);
    }


    std::vector<std::shared_ptr<const Individual>> selectionResults;
    ASSERT_NO_THROW(selectionResults = selection.select(population, 100, rng)) << "Selecting individuals failed";
    ASSERT_EQ(selectionResults.size(), 100) << "There should be 100 selection results";

    for(size_t idx = 0; idx < 100; idx++) {
        ASSERT_LT(selectionResults.at(idx)->getIndividualID(), 200) << "ID should be below 200";
    }

    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> setSelection;
    setSelection.insert(selectionResults.begin(), selectionResults.end());
    ASSERT_EQ(setSelection.size(), 100) << "There should be 100 selection results";

    ASSERT_NO_THROW(selectionResults = selection.select(population, 1000, rng)) << "Selecting individuals failed";
    ASSERT_EQ(selectionResults.size(), 200) << "There should be 200 selection results";
}

TEST(RandomSelectorTest, selectWithReplacement) 
{
    CounterReset::counterReset();
    Selection::RandomSelector selection(true);
    Representations::FakeRepresentation rep;
    RNG::RNG rng;
    rng.setSeed(0);


    // Create 200 individuals with scores 0, 1, 2, ..., 199.
    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> population;
    for(size_t idx = 0; idx < 200; idx++) {
        std::shared_ptr<const Individual> indiv = std::make_shared<Individual>(rep);
        
        indiv->addEvaluationRun(std::move(std::make_unique<Evaluation::EvaluationRun>(
                                      std::move(std::make_unique<Evaluation::ScoreMetric>(double(idx))))), 1);
        population.insert(indiv);
    }


    std::vector<std::shared_ptr<const Individual>> selectionResults;
    ASSERT_NO_THROW(selectionResults = selection.select(population, 100, rng)) << "Selecting individuals failed";
    ASSERT_EQ(selectionResults.size(), 100) << "There should be 200 selection results";

    for(size_t idx = 0; idx < 100; idx++) {
        ASSERT_LT(selectionResults.at(idx)->getIndividualID(), 200) << "ID should be below 200";
    }

    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> setSelection;
    setSelection.insert(selectionResults.begin(), selectionResults.end());
    ASSERT_LT(setSelection.size(), 100) << "It is very very very unlickly that no individual has been sampled twice!";

    ASSERT_NO_THROW(selectionResults = selection.select(population, 1000, rng)) << "Selecting individuals failed";
    ASSERT_EQ(selectionResults.size(), 1000) << "There should be 1000 selection results";
}

TEST(RandomSelectorTest, selectIsDeterminist) 
{
    CounterReset::counterReset();
    Selection::RandomSelector selection(true);
    Representations::FakeRepresentation rep;
    RNG::RNG rng;
    rng.setSeed(0);


    // Create 200 individuals with scores 0, 1, 2, ..., 199 and IDS 0, 2, 4, ..., 398.
    // Create 200 individuals with random scores in ranges  and IDS 1, 3, 5, ..., 399.
    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> population1;
    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> population2;
    for(size_t idx = 0; idx < 200; idx++) {
        std::shared_ptr<const Individual> indiv1 = std::make_shared<Individual>(rep);
        
        indiv1->addEvaluationRun(std::move(std::make_unique<Evaluation::EvaluationRun>(
                                      std::move(std::make_unique<Evaluation::ScoreMetric>(double(idx))))), 1);
        population1.insert(indiv1);

        
        std::shared_ptr<const Individual> indiv2 = std::make_shared<Individual>(rep);
        
        indiv2->addEvaluationRun(std::move(std::make_unique<Evaluation::EvaluationRun>(
                                      std::move(std::make_unique<Evaluation::ScoreMetric>(rng.uniformSample<double>(-1000.0, 1000.0))))), 1);
        population2.insert(indiv2);
    }

    rng.setSeed(0);
    std::vector<std::shared_ptr<const Individual>> selectionResults1;
    ASSERT_NO_THROW(selectionResults1 = selection.select(population1, 100, rng)) << "Selecting individuals failed";

    rng.setSeed(0);
    std::vector<std::shared_ptr<const Individual>> selectionResults2;
    ASSERT_NO_THROW(selectionResults2 = selection.select(population2, 100, rng)) << "Selecting individuals failed";

    for(size_t idx = 0; idx < 100; idx++) {
        ASSERT_EQ(selectionResults1.at(idx)->getIndividualID(), selectionResults2.at(idx)->getIndividualID() - 1) << "Order should be exactly the same!";
    }
} 