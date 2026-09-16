
#include <gtest/gtest.h>

#include "selection/truncationSelector.h"
#include "learn/fakeRepresentation.h"
#include "util/counterReset.h"

#include "evaluation/scoreMetric.h"

TEST(TruncationSelectorTest, Constructor)
{
    Selection::TruncationSelector* selection;

    ASSERT_NO_THROW(selection = new Selection::TruncationSelector()) << "Constructor of SurvivingSelection failed.";

    ASSERT_NO_THROW(delete selection) << "Destructor of SurvivingSelection failed.";
}

TEST(SelectorTest, getSelectionMetrics)
{
    Selection::TruncationSelector selection;
    std::vector<std::unique_ptr<Evaluation::EvaluationMetric>> metrics = selection.getSelectionMetrics();

    ASSERT_EQ(metrics.size(), 1) << "Should have only one metric";
    ASSERT_TRUE(dynamic_cast<Evaluation::ScoreMetric*>(metrics.at(0).get()) != nullptr) << "Metric should be a score metric!";
}

TEST(SelectorTest, getRankedScores)
{
    CounterReset::counterReset();
    Selection::TruncationSelector selection;
    Representations::FakeRepresentation rep;


    // Create 200 individuals with scores 0, 1, 2, ..., 199.
    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> population;
    for(size_t idx = 0; idx < 200; idx++) {
        population.insert(std::make_shared<Individual>(rep));
    }

    ASSERT_THROW(selection.assignFitness(population), std::runtime_error) << "Should throw with no metric";
    CounterReset::counterReset();

    population.clear();
    for(size_t idx = 0; idx < 200; idx++) {
        std::shared_ptr<const Individual> indiv = std::make_shared<Individual>(rep);
        
        indiv->addEvaluationRun(std::move(std::make_unique<Evaluation::EvaluationRun>(
                                      std::move(std::make_unique<Evaluation::ScoreMetric>(double(idx))))), 1);
        population.insert(indiv);
    }

    std::vector<std::pair<double, std::shared_ptr<const Individual>>> fitnessAssigned = selection.assignFitness(population);
    for(size_t idx = 0; idx < 200; idx++) {
        ASSERT_EQ(fitnessAssigned.at(idx).second->getIndividualID(), 199 - idx) << "ID should be the same";
    }
}


TEST(SelectorTest, getBest)
{
    CounterReset::counterReset();
    Selection::TruncationSelector selection;
    Representations::FakeRepresentation rep;

    // Create 200 individuals with scores 0, 1, 2, ..., 199.
    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> population;
    for(size_t idx = 0; idx < 200; idx++) {
        std::shared_ptr<const Individual> indiv = std::make_shared<Individual>(rep);
        
        indiv->addEvaluationRun(std::move(std::make_unique<Evaluation::EvaluationRun>(
                                      std::move(std::make_unique<Evaluation::ScoreMetric>(double(idx))))), 1);
        population.insert(indiv);
    }

    const Individual* best;
    ASSERT_NO_THROW(best = &selection.getBest(population)) << "Getting best score failed";
    ASSERT_EQ(best->getIndividualID(), 199) << "Best individual should be last indiv of the vector";

}

TEST(TruncationSelectorTest, select)
{
    CounterReset::counterReset();
    Selection::TruncationSelector selection;
    Representations::FakeRepresentation rep;
    RNG::RNG rng;

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
        ASSERT_EQ(selectionResults.at(idx)->getIndividualID(), 199 - idx) << "ID should be the same";
    }
}