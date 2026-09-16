
#include <gtest/gtest.h>

#include "learn/fakeRepresentation.h"
#include "reproduction/replicator.h"
#include "util/counterReset.h"

TEST(ReplicationTest, reproduce) 
{
    CounterReset::counterReset();
    GraphBased::Genotype genotype1;
    GraphBased::Genotype genotype2;
    
    GraphBased::NodeGroup group1;
    group1.addNode(std::make_unique<GraphBased::GPNode>(std::vector<double>{1.0, 2.0, 3.0}));
    group1.addNode(std::make_unique<GraphBased::GPNode>(std::vector<double>{2.0, 3.0, 4.0}));
    group1.addNode(std::make_unique<GraphBased::GPNode>(std::vector<double>{3.0, 4.0, 5.0}));
    genotype1.addNodeGroup(group1.cloneUniquePtr());

    GraphBased::NodeGroup group2;
    group2.addNode(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{4, 5, 6}));
    group2.addNode(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{6, 5, 4}));
    genotype2.addNodeGroup(group2.cloneUniquePtr());

    Representations::FakeRepresentation rep;
    RNG::RNG rng;

    Reproduction::Replicator reproducer;

    std::vector<std::shared_ptr<const Individual>> parents;
    parents.push_back(std::make_shared<Individual>(rep, genotype1.cloneUniquePtr()));
    parents.push_back(std::make_shared<Individual>(rep, genotype2.cloneUniquePtr()));

    std::set<std::shared_ptr<Individual>, SharedLess<Individual>> offspring = reproducer.reproduce(parents, 10, rng);
    int nbIndiv1 = 0;
    int nbIndiv2 = 0;
    ASSERT_EQ(offspring.size(), 10) << "Should have 10 offspring";
    for(auto it = offspring.begin(); it != offspring.end(); it++) {
        const GraphBased::Genotype& genotypeCreated = (*it)->getGenotype();
        bool isIndiv1 = (genotypeCreated == genotype1);
        bool isIndiv2 = (genotypeCreated == genotype2);
        ASSERT_TRUE(isIndiv1 || isIndiv2) << "Should be one of the two";
        nbIndiv1 += isIndiv1;
        nbIndiv2 += isIndiv2;
    }
    ASSERT_GT(nbIndiv1, 0) << "Very unlikely that individual 1 has not been sampled once!";
    ASSERT_GT(nbIndiv2, 0) << "Very unlikely that individual 2 has not been sampled once!";
}