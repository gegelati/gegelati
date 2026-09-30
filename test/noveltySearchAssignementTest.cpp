
#include <gtest/gtest.h>

#include "fitnessAssignment/noveltySearchAssigner.h"
#include "metrics/vectorMetric.h"

#include "learn/fakeRepresentation.h"
#include "learn/stickGameWithOpponent.h"
#include "evaluation/reinforcementProblem.h"
#include "util/counterReset.h"

#include <cstdlib>

class FakeVectorMetric: public Metrics::VectorMetric {
    public:
    FakeVectorMetric(std::vector<double> values = {0.0}): Metrics::VectorMetric(2) {this->values = values;};
    std::unique_ptr<Metric> cloneEmptyPtr() const override {return std::make_unique<FakeVectorMetric>(this->values);};
    std::string toString(std::string prefix = "") const override {return "";};
};


TEST(NoveltySearchTest, Constructor) 
{
    FitnessAssignment::Assigner* assigner;

    ASSERT_NO_THROW(assigner = new FitnessAssignment::NoveltySearchAssigner(2)) << "Constructor of NoveltySearchAssigner failed.";

    ASSERT_NO_THROW(delete assigner) << "Destructor of NoveltySearchAssigner failed.";
}


TEST(NoveltySearchTest, computeDescriptors)
{
    CounterReset::counterReset();
    FitnessAssignment::NoveltySearchAssigner noveltySearch(2);

    Representations::FakeRepresentation rep;

    StickGameWithOpponent le;
    Evaluation::ReinforcementProblem problem(le);

    std::vector<std::vector<double>> descriptorValues{{0.0}, {0.1}, {0.3}, {1.0}};
    std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>> mapResults;

    for(const std::vector<double>& value: descriptorValues) {
        std::shared_ptr<const Individual> indiv = std::make_shared<Individual>(rep);

        auto mapTemplate = std::make_shared<Metrics::MetricMapTemplate>();
        mapTemplate->addRequiredMetric(std::make_unique<FakeVectorMetric>(value));
        std::unique_ptr<Metrics::MetricMap> map = std::make_unique<Metrics::MetricMap>(mapTemplate);
        map->extractionEnd(0, *indiv, problem);

        mapResults.insert(std::make_pair(indiv, std::move(map)));
    }

    ASSERT_NO_THROW(noveltySearch.computeDescriptors(mapResults, std::make_unique<FakeVectorMetric>()->hash())) << "Assigning fitness failed";
    std::map<uint64_t, std::vector<double>> descriptor = noveltySearch.computeDescriptors(mapResults, std::make_unique<FakeVectorMetric>()->hash());

    ASSERT_EQ(descriptor.size(), 4);
    
    ASSERT_NE(descriptor.find(0), descriptor.end());
    ASSERT_NE(descriptor.find(1), descriptor.end());
    ASSERT_NE(descriptor.find(2), descriptor.end());
    ASSERT_NE(descriptor.find(3), descriptor.end());

    ASSERT_EQ(descriptor.at(0), std::vector<double>({0.0}));
    ASSERT_EQ(descriptor.at(1), std::vector<double>({0.1}));
    ASSERT_EQ(descriptor.at(2), std::vector<double>({0.3}));
    ASSERT_EQ(descriptor.at(3), std::vector<double>({1.0}));
}

TEST(NoveltySearchTest, computeDescriptorsInvalidMetrics1)
{
    CounterReset::counterReset();
    FitnessAssignment::NoveltySearchAssigner noveltySearch(2);

    Representations::FakeRepresentation rep;

    StickGameWithOpponent le;
    Evaluation::ReinforcementProblem problem(le);

    std::vector<std::vector<double>> descriptorValues{{}, {1.0}};
    std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>> mapResults;

    for(const std::vector<double>& value: descriptorValues) {
        std::shared_ptr<const Individual> indiv = std::make_shared<Individual>(rep);

        auto mapTemplate = std::make_shared<Metrics::MetricMapTemplate>();
        mapTemplate->addRequiredMetric(std::make_unique<FakeVectorMetric>(value));
        std::unique_ptr<Metrics::MetricMap> map = std::make_unique<Metrics::MetricMap>(mapTemplate);
        map->extractionEnd(0, *indiv, problem);

        mapResults.insert(std::make_pair(indiv, std::move(map)));
    }

    ASSERT_THROW(noveltySearch.computeDescriptors(mapResults, std::make_unique<FakeVectorMetric>()->hash()), std::runtime_error) << "Assigning fitness should fail with empty descriptors";
}


TEST(NoveltySearchTest, computeDescriptorsInvalidMetrics2)
{
    CounterReset::counterReset();
    FitnessAssignment::NoveltySearchAssigner noveltySearch(2);

    Representations::FakeRepresentation rep;

    StickGameWithOpponent le;
    Evaluation::ReinforcementProblem problem(le);

    std::vector<std::vector<double>> descriptorValues{{1.0}, {2.0, 3.0}};
    std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>> mapResults;

    for(const std::vector<double>& value: descriptorValues) {
        std::shared_ptr<const Individual> indiv = std::make_shared<Individual>(rep);

        auto mapTemplate = std::make_shared<Metrics::MetricMapTemplate>();
        mapTemplate->addRequiredMetric(std::make_unique<FakeVectorMetric>(value));
        std::unique_ptr<Metrics::MetricMap> map = std::make_unique<Metrics::MetricMap>(mapTemplate);
        map->extractionEnd(0, *indiv, problem);

        mapResults.insert(std::make_pair(indiv, std::move(map)));
    }

    ASSERT_THROW(noveltySearch.computeDescriptors(mapResults, std::make_unique<FakeVectorMetric>()->hash()), std::runtime_error) << "Assigning fitness should fail with different size of descriptors";
}


TEST(NoveltySearchTest, computeDistances) 
{
    FitnessAssignment::NoveltySearchAssigner noveltySearch(2);
    std::map<uint64_t, std::vector<double>> descriptorValues;
    descriptorValues.insert(std::make_pair(0, std::vector<double>({0.0})));
    descriptorValues.insert(std::make_pair(1, std::vector<double>({0.1})));
    descriptorValues.insert(std::make_pair(2, std::vector<double>({0.3})));
    descriptorValues.insert(std::make_pair(3, std::vector<double>({1.0})));

    ASSERT_NO_THROW(noveltySearch.computeDistances(descriptorValues)) << "computing distances failed";
    std::map<uint64_t, std::map<uint64_t, double>> distances = noveltySearch.computeDistances(descriptorValues);
    ASSERT_EQ(distances.size(), 4) << "Should be size 3";

    const std::array<std::array<size_t, 3>, 4> expectedIDs = {{
        {1, 2, 3},
        {0, 2, 3},
        {0, 1, 3},
        {0, 1, 2},
    }};
    const std::array<std::array<double, 3>, 4> expectedDistances = {{
        {0.1, 0.3, 1.0},
        {0.1, 0.2, 0.9},
        {0.3, 0.2, 0.7},
        {1.0, 0.9, 0.7},
    }};

    size_t idx = 0;
    for(auto it = distances.begin(); it != distances.end(); it++) {
        ASSERT_EQ(it->first, idx) << "Should be good index";
        ASSERT_EQ(it->second.size(), 3) << "Should be size 2";
        
        size_t idy = 0;
        for(auto itY = it->second.begin(); itY != it->second.end(); itY++) {
            EXPECT_EQ(itY->first, expectedIDs.at(idx).at(idy)) << "Should be good index";
            
            ASSERT_NEAR(itY->second, expectedDistances.at(idx).at(idy), 0.00001) << "Should be good Distance";
         
            idy++;
        }

        idx++;
    }
}

TEST(NoveltySearchTest, computeNeighborsTest)
{
    FitnessAssignment::NoveltySearchAssigner noveltySearch(2);

    uint64_t id0 = 0; uint64_t id1 = 1;
    uint64_t id2 = 2; uint64_t id3 = 3;

    std::map<uint64_t, std::map<uint64_t, double>> distances;
    distances[id0] = {{id1, 0.1}, {id2, 0.3}, {id3, 1.0}};
    distances[id1] = {{id0, 0.1}, {id2, 0.2}, {id3, 0.9}};
    distances[id2] = {{id0, 0.3}, {id1, 0.2}, {id3, 0.7}};
    distances[id3] = {{id0, 1.0}, {id1, 0.9}, {id2, 0.7}};


    ASSERT_NO_THROW(noveltySearch.computeNeighbors(distances)) << "computing neighbors failed";
    std::map<uint64_t, std::vector<uint64_t>> neighbors = noveltySearch.computeNeighbors(distances);

    ASSERT_EQ(neighbors.size(), 4);

    ASSERT_EQ(neighbors.at(id0).size(), 2);
    ASSERT_EQ(neighbors.at(id0).at(0), id1);
    ASSERT_EQ(neighbors.at(id0).at(1), id2);

    ASSERT_EQ(neighbors.at(id1).size(), 2);
    ASSERT_EQ(neighbors.at(id1).at(0), id0);
    ASSERT_EQ(neighbors.at(id1).at(1), id2);

    ASSERT_EQ(neighbors.at(id2).size(), 2);
    ASSERT_EQ(neighbors.at(id2).at(0), id1);
    ASSERT_EQ(neighbors.at(id2).at(1), id0);

    ASSERT_EQ(neighbors.at(id3).size(), 2);
    ASSERT_EQ(neighbors.at(id3).at(0), id2);
    ASSERT_EQ(neighbors.at(id3).at(1), id1);
}

TEST(NoveltySearchTest, computeNoveltyScore)
{
    FitnessAssignment::NoveltySearchAssigner noveltySearch(2);

    uint64_t id0 = 0; uint64_t id1 = 1;
    uint64_t id2 = 2; uint64_t id3 = 3;

    std::map<uint64_t, std::map<uint64_t, double>> distances;
    distances[id0] = {{id1, 0.1}, {id2, 0.3}, {id3, 1.0}};
    distances[id1] = {{id0, 0.1}, {id2, 0.2}, {id3, 0.9}};
    distances[id2] = {{id0, 0.3}, {id1, 0.2}, {id3, 0.7}};
    distances[id3] = {{id0, 1.0}, {id1, 0.9}, {id2, 0.7}};

    std::map<uint64_t, std::vector<uint64_t>> neighbors;
    neighbors[id0] = {id1, id2};
    neighbors[id1] = {id0, id2};
    neighbors[id2] = {id1, id0};
    neighbors[id3] = {id2, id1};
    
    ASSERT_NO_THROW(noveltySearch.computeNoveltyScores(distances, neighbors)) << "computing noveltyScores failed";
    std::map<uint64_t, double> noveltyScore = noveltySearch.computeNoveltyScores(distances, neighbors);

    ASSERT_EQ(noveltyScore.size(), 4);
    
    ASSERT_NE(noveltyScore.find(id0), noveltyScore.end());
    ASSERT_NE(noveltyScore.find(id1), noveltyScore.end());
    ASSERT_NE(noveltyScore.find(id2), noveltyScore.end());
    ASSERT_NE(noveltyScore.find(id3), noveltyScore.end());

    ASSERT_NEAR(noveltyScore.at(id0), (0.1 + 0.3) / 2, 0.00001);
    ASSERT_NEAR(noveltyScore.at(id1), (0.1 + 0.2) / 2, 0.00001);
    ASSERT_NEAR(noveltyScore.at(id2), (0.2 + 0.3) / 2, 0.00001);
    ASSERT_NEAR(noveltyScore.at(id3), (0.9 + 0.7) / 2, 0.00001);

    // Checking errors
    neighbors[id0].clear();
    ASSERT_THROW(noveltySearch.computeNoveltyScores(distances, neighbors), std::runtime_error) << "Should fail with an empty list of neighbors";
}



TEST(NoveltySearchTest, doSelection)
{
    CounterReset::counterReset();
    FitnessAssignment::NoveltySearchAssigner noveltySearch(2);

    Representations::FakeRepresentation rep;

    StickGameWithOpponent le;
    Evaluation::ReinforcementProblem problem(le);

    std::vector<std::vector<double>> descriptorValues{{0.0}, {0.1}, {0.3}, {1.0}};
    std::vector<std::shared_ptr<const Individual>> indivs;
    std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>> mapResults;

    for(const std::vector<double>& value: descriptorValues) {
        std::shared_ptr<const Individual> indiv = std::make_shared<Individual>(rep);
        indivs.push_back(indiv);

        auto mapTemplate = std::make_shared<Metrics::MetricMapTemplate>();
        mapTemplate->addRequiredMetric(std::make_unique<FakeVectorMetric>(value));
        std::unique_ptr<Metrics::MetricMap> map = std::make_unique<Metrics::MetricMap>(mapTemplate);
        map->extractionEnd(0, *indiv, problem);

        mapResults.insert(std::make_pair(indiv, std::move(map)));
    }

    ASSERT_NO_THROW(noveltySearch.assignFitness(mapResults, std::make_unique<FakeVectorMetric>()->hash())) << "Assigning fitness failed";
    std::map<std::shared_ptr<const Individual>, double, SharedLess<Individual>> noveltyFitness = noveltySearch.assignFitness(mapResults, std::make_unique<FakeVectorMetric>()->hash());

    ASSERT_EQ(noveltyFitness.size(), 4);
    
    ASSERT_NE(noveltyFitness.find(indivs.at(0)), noveltyFitness.end());
    ASSERT_NE(noveltyFitness.find(indivs.at(1)), noveltyFitness.end());
    ASSERT_NE(noveltyFitness.find(indivs.at(2)), noveltyFitness.end());
    ASSERT_NE(noveltyFitness.find(indivs.at(3)), noveltyFitness.end());

    ASSERT_NEAR(noveltyFitness.at(indivs.at(0)), (0.1 + 0.3) / 2, 0.00001);
    ASSERT_NEAR(noveltyFitness.at(indivs.at(1)), (0.1 + 0.2) / 2, 0.00001);
    ASSERT_NEAR(noveltyFitness.at(indivs.at(2)), (0.2 + 0.3) / 2, 0.00001);
    ASSERT_NEAR(noveltyFitness.at(indivs.at(3)), (0.9 + 0.7) / 2, 0.00001);
} 


