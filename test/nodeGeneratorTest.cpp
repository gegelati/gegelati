
#include <fstream>
#include <gtest/gtest.h>

#include "node/nodeGenerator.h"
#include "node/genotypeGenerator.h"

#include "dimensions/dataValueGenerator.h"

TEST(NodeGeneratorTest, Constructor) 
{
    Node::NodeGenerator* nodeGenerator;

    ASSERT_NO_THROW(nodeGenerator = new Node::NodeGenerator()) << "Creation of generator failed";
    
    ASSERT_NO_THROW(delete nodeGenerator) << "Destructor of generator failed.";
}

TEST(NodeGeneratorTest, addValue) {
    Node::NodeGenerator nodeGenerator;

    ASSERT_EQ(nodeGenerator.size(), 0) << "Size of the generator should be 0";
    ASSERT_THROW(nodeGenerator.getGeneratorAt(0), std::runtime_error) << "Getting a generator with empty generator should fail";

    ASSERT_NO_THROW(nodeGenerator.addGenerator(Dimensions::NumericUniformGenerator<int>(1, 1))) << "Should not have fail to add generator";
    ASSERT_EQ(nodeGenerator.size(), 1) << "Size of the generator should be 0";

    RNG::RNG rng;
    Data::DataValue value = Data::DataValue::scalar<int>(1);
    ASSERT_EQ(nodeGenerator.getGeneratorAt(0).sample(rng), value) << "Value should be 0";

    std::unique_ptr<Node::NodeGenerator> clone = nodeGenerator.cloneUniquePtr();
    ASSERT_EQ(clone->getGeneratorAt(0).sample(rng), value) << "Value should be 0";
}



TEST(GenotypeGeneratorTest, Constructor) 
{
    Node::GenotypeGenerator* genotypeGenerator;
    Node::GenotypeGenerator* genotypeGenerator1;

    Node::NodeGenerator nodeGenerator;

    ASSERT_NO_THROW(genotypeGenerator = new Node::GenotypeGenerator()) << "Creation of generator failed";
    ASSERT_NO_THROW(genotypeGenerator1 = new Node::GenotypeGenerator(nodeGenerator, 1, 2)) << "Creation of generator failed";
    
    ASSERT_NO_THROW(delete genotypeGenerator) << "Destructor of generator failed.";
    ASSERT_NO_THROW(delete genotypeGenerator1) << "Destructor of generator failed.";

}

TEST(GenotypeGeneratorTest, addValue) {
    Node::GenotypeGenerator genotypeGenerator;
    ASSERT_EQ(genotypeGenerator.size(), 0) << "Size of genotypeGenerator mismatch.";

    Node::NodeGenerator nodeGenerator0;
    ASSERT_THROW(genotypeGenerator.addNodeGenerator(nodeGenerator0), std::runtime_error) << "Should have failed with empty value.";

    
    nodeGenerator0.addGenerator(Dimensions::NumericUniformGenerator<int>(1, 1));
    nodeGenerator0.addGenerator(Dimensions::NumericUniformGenerator<int>(42, 42)); 
    ASSERT_NO_THROW(genotypeGenerator.addNodeGenerator(nodeGenerator0, 1, 5)) << "Should not have failed.";
    ASSERT_EQ(genotypeGenerator.size(), 1) << "Size of genotypeGenerator mismatch.";

    Node::NodeGenerator nodeGenerator1;
    nodeGenerator1.addGenerator(Dimensions::NumericUniformGenerator<int>(1, 42)); 
    ASSERT_NO_THROW(genotypeGenerator.addNodeGenerator(nodeGenerator1, 5)) << "Should not have failed.";
    

    Data::DataValue valueMin = Data::DataValue::scalar<int>(1);
    Data::DataValue valueMax = Data::DataValue::scalar<int>(42);
    Node::NodeGenerator& nodeGeneratorGet0 = genotypeGenerator.getNodeGeneratorAt(0);
    Node::NodeGenerator& nodeGeneratorGet1 = genotypeGenerator.getNodeGeneratorAt(1);
    ASSERT_EQ(nodeGeneratorGet0.size(), 2) << "Size should be equal";
    ASSERT_EQ(nodeGeneratorGet1.size(), 1) << "Size should be equal";

    RNG::RNG rng;
    ASSERT_TRUE(nodeGeneratorGet0.getGeneratorAt(0).sample(rng) == valueMin) << "Value should be the same";
    ASSERT_TRUE(nodeGeneratorGet0.getGeneratorAt(1).sample(rng) == valueMax) << "Value should be the same";
    Data::DataValue sampled = nodeGeneratorGet1.getGeneratorAt(0).sample(rng);
    ASSERT_TRUE(sampled.getScalar<int>() >= 1 || sampled.getScalar<int>() <= 42) << "Value should be accepted";

    ASSERT_THROW(genotypeGenerator.getNodeGeneratorAt(2), std::runtime_error) << "Should have failed with out of range.";
    ASSERT_THROW(genotypeGenerator.getRangeAt(2), std::runtime_error) << "Should have failed with out of range.";
}