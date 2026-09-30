#include <gtest/gtest.h>

#include "fitnessAssignment/assigner.h"
#include "fitnessAssignment/defaultAssigner.h"

#include "metrics/scalarMetric.h"

#include "learn/fakeRepresentation.h"
#include "learn/stickGameWithOpponent.h"
#include "evaluation/reinforcementProblem.h"
#include "util/counterReset.h"

class FakeMetric: public Metrics::ScalarMetric {
    public:
    FakeMetric(double score) {this->values[0] = score;};
    std::unique_ptr<Metric> cloneEmptyPtr() const override {return std::make_unique<FakeMetric>(this->values[0]);};
    std::string toString(std::string prefix = "") const override {return "";};
};

TEST(FitnessAssignmentTest, Constructor) 
{
    FitnessAssignment::Assigner* assigner;

    ASSERT_NO_THROW(assigner = new FitnessAssignment::DefaultAssigner()) << "Constructor of DefaultAssigner failed.";

    ASSERT_NO_THROW(delete assigner) << "Destructor of DefaultAssigner failed.";
}

TEST(FitnessAssignmentTest, assignFitness)
{
    CounterReset::counterReset();
    FitnessAssignment::DefaultAssigner assigner;
    Representations::FakeRepresentation rep;

    StickGameWithOpponent le;
    Evaluation::ReinforcementProblem problem(le);

    std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>> mapResults;
    
    // Create 200 individuals with scores 0, 1, 2, ..., 199.
    for(size_t idx = 0; idx < 200; idx++) {
        std::shared_ptr<const Individual> indiv = std::make_shared<Individual>(rep);

        auto mapTemplate = std::make_shared<Metrics::MetricMapTemplate>();
        mapTemplate->addRequiredMetric(std::make_unique<FakeMetric>(double(idx)));
        std::unique_ptr<Metrics::MetricMap> map = std::make_unique<Metrics::MetricMap>(mapTemplate);
        map->extractionEnd(0, *indiv, problem);

        mapResults.insert(std::make_pair(indiv, std::move(map)));
    }

    ASSERT_NO_THROW(assigner.assignFitness(mapResults, std::make_unique<FakeMetric>(0.0)->hash())) << "Assigning fitness failed";


    std::map<std::shared_ptr<const Individual>, double, SharedLess<Individual>> fitnessAssigned = assigner.assignFitness(mapResults, std::make_unique<FakeMetric>(0.0)->hash());
    auto it = fitnessAssigned.begin();
    for(size_t idx = 0; idx < 200; idx++) {
        EXPECT_EQ(it->first->getIndividualID(), idx) << "ID should be the same";
        EXPECT_EQ(it->second, double(idx)) << "ID should be the same";

        it++;
    }
}