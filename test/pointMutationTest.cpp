

#include <algorithm>
#include <fstream>
#include <gtest/gtest.h>
#include <numeric>

#include "mutation/mutator.h"
#include "individual.h"


#include "mutation/mutator.h"
#include "mutation/pointMutator.h"

#include "graphBased/nodeGenerator.h"
#include "graphBased/genotypeGenerator.h"

#include "dimensions/dataValueGenerator.h"
#include "learn/fakeRepresentation.h"


/*
 * ============================================================================
 * Mutator tests
 * ============================================================================
 */


TEST(MutatorTest, createRandomNode)
{
    Mutation::PointMutator mutator(0.0);
    GraphBased::NodeGenerator nodeGenerator;
    RNG::RNG rng;

    ASSERT_THROW(
        mutator.createRandomNode(nodeGenerator, rng),
        std::runtime_error
    ) << "Creating a random node with an empty NodeGenerator should fail";


    nodeGenerator.addGenerator(
        Dimensions::NumericUniformGenerator<int>(1, 1)
    );

    nodeGenerator.addGenerator(
        Dimensions::NumericUniformGenerator<int>(42, 42)
    );

    std::unique_ptr<GraphBased::GPNode> node;

    ASSERT_NO_THROW(
        node = mutator.createRandomNode(nodeGenerator, rng)
    ) << "Creating a random node should not fail";

    ASSERT_EQ(node->getSize(), 2)
        << "Generated node should contain one value per generator";

    ASSERT_EQ(
        node->getValue(0),
        Data::DataValue::scalar<int>(1)
    ) << "First generated value mismatch";

    ASSERT_EQ(
        node->getValue(1),
        Data::DataValue::scalar<int>(42)
    ) << "Second generated value mismatch";
}


TEST(MutatorTest, initRandomGenotype)
{
    Mutation::PointMutator mutator(0.0);
    RNG::RNG rng;

    GraphBased::NodeGenerator nodeGenerator0;
    nodeGenerator0.addGenerator(Dimensions::NumericUniformGenerator<int>(1, 1));
    nodeGenerator0.addGenerator(Dimensions::NumericUniformGenerator<int>(42, 42));
    GraphBased::NodeGenerator nodeGenerator1;
    nodeGenerator1.addGenerator(Dimensions::NumericUniformGenerator<double>(0.0, 1.0));
    nodeGenerator1.addGenerator(Dimensions::NumericUniformGenerator<int>(-10, 0));
    nodeGenerator1.addGenerator(Dimensions::NumericUniformGenerator<size_t>(2, 2));
    std::unique_ptr<GraphBased::GenotypeGenerator> genotypeGenerator =
        std::make_unique<GraphBased::GenotypeGenerator>();
    genotypeGenerator->addNodeGenerator(nodeGenerator0, 3);
    genotypeGenerator->addNodeGenerator(nodeGenerator1, 5, 10);

    std::unique_ptr<GraphBased::Genotype> sampled;
    ASSERT_NO_THROW(sampled = std::move(mutator.initRandomGenotype(std::move(genotypeGenerator), rng))) << "Creating genotype failed";

    ASSERT_EQ(sampled->getSize(), 2);
    GraphBased::NodeGroup& group0 = sampled->getNodeGroup(0);
    GraphBased::NodeGroup& group1 = sampled->getNodeGroup(1);

    ASSERT_EQ(group0.getSize(), 3);
    ASSERT_TRUE((group1.getSize() >= 5) && (group1.getSize() <= 10)) << "Size should be in the range";

    for (size_t idx = 0; idx < group0.getSize(); idx++) {
        ASSERT_EQ(group0.getNode(idx).getSize(), 2) << "Should be size 2";
        ASSERT_EQ(group0.getNode(idx).getValue(0).getScalar<int>() , 1) << "Value should be 1";
        ASSERT_EQ(group0.getNode(idx).getValue(1).getScalar<int>() , 42) << "Value should be 1";
    }

    
    for (size_t idx = 0; idx < group1.getSize(); idx++) {
        ASSERT_EQ(group1.getNode(idx).getSize(), 3) << "Should be size 3";
        double v0 = group1.getNode(idx).getValue(0).getScalar<double>();
        ASSERT_TRUE(v0 >= 0.0 && v0 <= 1.0) << "Value should be in range [0,1]";
        int v1 = group1.getNode(idx).getValue(1).getScalar<int>();
        ASSERT_TRUE(v1 >= -10 && v1 <= 0.0) << "Value should be in range [-10,0]";
        ASSERT_EQ(group1.getNode(idx).getValue(2).getScalar<size_t>() , 2) << "Value should be 2";
    }
    

    std::unique_ptr<GraphBased::GenotypeGenerator> emptyGenerator =
        std::make_unique<GraphBased::GenotypeGenerator>();

    ASSERT_THROW(
        mutator.initRandomGenotype(std::move(emptyGenerator), rng),
        std::runtime_error
    ) << "Creating a genotype with an empty GenotypeGenerator should fail";
}


/*
 * ============================================================================
 * PointMutator tests
 * ============================================================================
 */

TEST(PointMutatorTest, Constructor)
{
    Mutation::PointMutator* pointMutator;

    ASSERT_NO_THROW(
        pointMutator = new Mutation::PointMutator(0.5)
    ) << "Creation of PointMutator failed";

    ASSERT_NO_THROW(
        delete pointMutator
    ) << "Destructor of PointMutator failed.";


    ASSERT_NO_THROW(
        pointMutator = new Mutation::PointMutator(0.5, 0.2, 0.1)
    ) << "Creation of PointMutator with all probabilities failed";

    ASSERT_NO_THROW(
        delete pointMutator
    ) << "Destructor of PointMutator failed.";
}

TEST(PointMutatorTest, mutateNode)
{
    Mutation::PointMutator pointMutator(1.0);
    GraphBased::NodeGenerator nodeGenerator;
    RNG::RNG rng;

    nodeGenerator.addGenerator(
        Dimensions::NumericUniformGenerator<int>(1, 1)
    );

    nodeGenerator.addGenerator(
        Dimensions::NumericUniformGenerator<int>(42, 42)
    );

    // Create a GPNode containing two values.
    GraphBased::GPNode node;
    node.addValue(Data::DataValue::scalar<int>(0));
    node.addValue(Data::DataValue::scalar<int>(0));

    ASSERT_NO_THROW(
        pointMutator.mutateNode(node, nodeGenerator, rng)
    );

    // Exactly one value should have been replaced.
    ASSERT_TRUE(
        node.getValue(0) == Data::DataValue::scalar<int>(1) ||
        node.getValue(0) == Data::DataValue::scalar<int>(0)
    );

    ASSERT_TRUE(
        node.getValue(1) == Data::DataValue::scalar<int>(42) ||
        node.getValue(1) == Data::DataValue::scalar<int>(0)
    );

    // At least one value must have changed.
    ASSERT_TRUE(
        node.getValue(0) == Data::DataValue::scalar<int>(1) ||
        node.getValue(1) == Data::DataValue::scalar<int>(42)
    );

    GraphBased::NodeGenerator wrongNodeGenerator;

    wrongNodeGenerator.addGenerator(
        Dimensions::NumericUniformGenerator<int>(1, 1)
    );

    ASSERT_THROW(
        pointMutator.mutateNode(node, wrongNodeGenerator, rng),
        std::runtime_error
    ) << "Mutation should fail when node and NodeGenerator sizes differ";
}


TEST(PointMutatorTest, mutateGenotype)
{
    Mutation::PointMutator pointMutator(
        1.0,  // pMutateNode
        0.0,  // pAddNode
        0.0   // pRemoveNode
    );

    RNG::RNG rng;


    /*
     * Empty GenotypeGenerator should fail.
     */
    std::unique_ptr<GraphBased::GenotypeGenerator> emptyGenerator =
        std::make_unique<GraphBased::GenotypeGenerator>();

    GraphBased::Genotype emptyGenotype;

    ASSERT_THROW(
        pointMutator.mutateGenotype(
            emptyGenotype,
            std::move(emptyGenerator),
            rng
        ),
        std::runtime_error
    ) << "Mutation with an empty GenotypeGenerator should fail";

    /*
     * Create a deterministic NodeGenerator.
     */
    GraphBased::NodeGenerator nodeGenerator;
    nodeGenerator.addGenerator(
        Dimensions::NumericUniformGenerator<int>(1, 1)
    );
    nodeGenerator.addGenerator(
        Dimensions::NumericUniformGenerator<int>(42, 42)
    );

    /*
     * Create a GenotypeGenerator with one node group.
     *
     * The group may contain between 1 and 2 nodes.
     */
    GraphBased::GenotypeGenerator genotypeGenerator;
    genotypeGenerator.addNodeGenerator(nodeGenerator, 1, 2);

    /*
     * Create a matching genotype containing one deterministic node.
     */
    GraphBased::Genotype genotype;

    std::unique_ptr<GraphBased::NodeGroup> group =
        std::make_unique<GraphBased::NodeGroup>();

    auto node = std::make_unique<GraphBased::GPNode>();
    node->addValue(Data::DataValue::scalar<int>(1));
    node->addValue(Data::DataValue::scalar<int>(42));

    group->addNode(std::move(node));
    genotype.addNodeGroup(std::move(group));

    /*
     * Mutate the genotype.
     *
     * Mutation is enabled, while addition/removal are disabled.
     */
    std::unique_ptr<GraphBased::Genotype> mutatedGenotype;

    ASSERT_NO_THROW(
        mutatedGenotype = pointMutator.mutateGenotype(
            genotype,
            genotypeGenerator.cloneUniquePtr(),
            rng
        )
    );

    ASSERT_NE(mutatedGenotype, nullptr);

    /*
     * Verify that the original genotype has not been modified.
     */
    ASSERT_EQ(genotype.getSize(), 1);
    ASSERT_EQ(genotype.getNodeGroup(0).getSize(), 1);

    ASSERT_EQ(
        genotype.getNodeGroup(0).getNode(0).getValue(0),
        Data::DataValue::scalar<int>(1)
    );

    ASSERT_EQ(
        genotype.getNodeGroup(0).getNode(0).getValue(1),
        Data::DataValue::scalar<int>(42)
    );

    /*
     * Since pAddNode = 0 and pRemoveNode = 0, the structure must
     * remain unchanged.
     */
    ASSERT_EQ(mutatedGenotype->getSize(), genotype.getSize());
    ASSERT_EQ(
        mutatedGenotype->getNodeGroup(0).getSize(),
        genotype.getNodeGroup(0).getSize()
    );

    ASSERT_EQ(
        mutatedGenotype->getNodeGroup(0).getNode(0).getValue(0),
        Data::DataValue::scalar<int>(1)
    );

    ASSERT_EQ(
        mutatedGenotype->getNodeGroup(0).getNode(0).getValue(1),
        Data::DataValue::scalar<int>(42)
    );


    /*
     * ------------------------------------------------------------------------
     * Test node addition.
     * ------------------------------------------------------------------------
     */
    Mutation::PointMutator addMutator(
        0.0,  // pMutateNode
        1.0,  // pAddNode
        0.0   // pRemoveNode
    );

    ASSERT_EQ(genotype.getNodeGroup(0).getSize(), 1);

    std::unique_ptr<GraphBased::Genotype> addedGenotype;

    ASSERT_NO_THROW(
        addedGenotype = addMutator.mutateGenotype(
            genotype,
            genotypeGenerator.cloneUniquePtr(),
            rng
        )
    );

    ASSERT_NE(addedGenotype, nullptr);
    ASSERT_EQ(addedGenotype->getSize(), genotype.getSize());

    /*
     * The group starts at 1 and has a maximum of 2, so one node
     * must have been added.
     */
    ASSERT_EQ(addedGenotype->getNodeGroup(0).getSize(), 2);

    /*
     * The added node is deterministic.
     */
    ASSERT_EQ(addedGenotype->getNodeGroup(0).getNode(1).getValue(0),Data::DataValue::scalar<int>(1));

    ASSERT_EQ(addedGenotype->getNodeGroup(0).getNode(1).getValue(1), Data::DataValue::scalar<int>(42) );


    /*
     * ------------------------------------------------------------------------
     * Test node removal.
     * ------------------------------------------------------------------------
     */
    Mutation::PointMutator removeMutator(
        0.0,  // pMutateNode
        0.0,  // pAddNode
        1.0   // pRemoveNode
    );

    /*
     * Create a genotype containing two nodes.
     */
    GraphBased::Genotype twoNodeGenotype;

    std::unique_ptr<GraphBased::NodeGroup> twoNodeGroup =
        std::make_unique<GraphBased::NodeGroup>();

    auto node0 = std::make_unique<GraphBased::GPNode>();
    node0->addValue(Data::DataValue::scalar<int>(1));
    node0->addValue(Data::DataValue::scalar<int>(42));

    auto node1 = std::make_unique<GraphBased::GPNode>();
    node1->addValue(Data::DataValue::scalar<int>(1));
    node1->addValue(Data::DataValue::scalar<int>(42));

    twoNodeGroup->addNode(std::move(node0));
    twoNodeGroup->addNode(std::move(node1));

    twoNodeGenotype.addNodeGroup(std::move(twoNodeGroup));

    ASSERT_EQ(twoNodeGenotype.getNodeGroup(0).getSize(), 2);

    std::unique_ptr<GraphBased::Genotype> removedGenotype;

    ASSERT_NO_THROW(
        removedGenotype = removeMutator.mutateGenotype(
            twoNodeGenotype,
            genotypeGenerator.cloneUniquePtr(),
            rng
        )
    );

    ASSERT_NE(removedGenotype, nullptr);

    /*
     * The minimum is 1, so exactly one of the two nodes must remain.
     */
    ASSERT_EQ(removedGenotype->getNodeGroup(0).getSize(), 1);

    /*
     * The original genotype must remain unchanged.
     */
    ASSERT_EQ(twoNodeGenotype.getNodeGroup(0).getSize(), 2);


    /*
     * ------------------------------------------------------------------------
     * Test minimum-node boundary.
     * ------------------------------------------------------------------------
     *
     * The group is already at range.first == 1, so removal must not happen.
     */
    ASSERT_EQ(genotype.getNodeGroup(0).getSize(), 1);

    std::unique_ptr<GraphBased::Genotype> minimumGenotype;

    ASSERT_NO_THROW(
        minimumGenotype = removeMutator.mutateGenotype(
            genotype,
            genotypeGenerator.cloneUniquePtr(),
            rng
        )
    );

    ASSERT_NE(minimumGenotype, nullptr);
    ASSERT_EQ(minimumGenotype->getNodeGroup(0).getSize(), 1);


    /*
     * ------------------------------------------------------------------------
     * Test maximum-node boundary.
     * ------------------------------------------------------------------------
     *
     * The group is already at range.second == 2, so addition must not happen.
     */
    std::unique_ptr<GraphBased::Genotype> maximumGenotype;

    ASSERT_NO_THROW(
        maximumGenotype = addMutator.mutateGenotype(
            twoNodeGenotype,
            genotypeGenerator.cloneUniquePtr(),
            rng
        )
    );

    ASSERT_NE(maximumGenotype, nullptr);
    ASSERT_EQ(maximumGenotype->getNodeGroup(0).getSize(), 2);
}


/*
 * ============================================================================
 * Additional boundary tests for PointMutator
 * ============================================================================
 */

TEST(PointMutatorTest, mutateNodeWithEmptyNode)
{
    Mutation::PointMutator pointMutator(1.0);
    GraphBased::NodeGenerator nodeGenerator;
    RNG::RNG rng;

    GraphBased::GPNode emptyNode;

    ASSERT_THROW(
        pointMutator.mutateNode(emptyNode, nodeGenerator, rng),
        std::runtime_error
    );

    nodeGenerator.addGenerator(
        Dimensions::NumericUniformGenerator<int>(1, 1)
    );

    ASSERT_THROW(
        pointMutator.mutateNode(emptyNode, nodeGenerator, rng),
        std::runtime_error
    );
}

TEST(PointMutatorTest, mutateGenotypeDoesNotMutateWhenProbabilityIsZero)
{
    Mutation::PointMutator pointMutator(
        0.0,  // pMutateNode
        0.0,  // pAddNode
        0.0   // pRemoveNode
    );

    RNG::RNG rng;

    GraphBased::NodeGenerator nodeGenerator;
    nodeGenerator.addGenerator(
        Dimensions::NumericUniformGenerator<int>(1, 1)
    );

    nodeGenerator.addGenerator(
        Dimensions::NumericUniformGenerator<int>(42, 42)
    );

    GraphBased::GenotypeGenerator genotypeGenerator;
    genotypeGenerator.addNodeGenerator(nodeGenerator, 2, 2);

    GraphBased::Genotype genotype;

    // Populate genotype with two deterministic nodes.
    std::unique_ptr<GraphBased::NodeGroup> group = std::make_unique<GraphBased::NodeGroup>();

    auto node0 = std::make_unique<GraphBased::GPNode>();
    node0->addValue(Data::DataValue::scalar<int>(1));
    node0->addValue(Data::DataValue::scalar<int>(42));

    auto node1 = std::make_unique<GraphBased::GPNode>();
    node1->addValue(Data::DataValue::scalar<int>(1));
    node1->addValue(Data::DataValue::scalar<int>(42));

    group->addNode(std::move(node0));
    group->addNode(std::move(node1));

    genotype.addNodeGroup(std::move(group));

    std::unique_ptr<GraphBased::Genotype> mutatedGenotype;

    ASSERT_NO_THROW(mutatedGenotype = pointMutator.mutateGenotype(genotype, genotypeGenerator.cloneUniquePtr(), rng));

    ASSERT_NE(mutatedGenotype, nullptr);

    ASSERT_EQ(mutatedGenotype->getSize(), genotype.getSize());
    ASSERT_EQ(mutatedGenotype->getNodeGroup(0).getSize(),
              genotype.getNodeGroup(0).getSize());

    for (size_t i = 0; i < genotype.getNodeGroup(0).getSize(); ++i) {
        ASSERT_EQ(
            mutatedGenotype->getNodeGroup(0).getNode(i).getValue(0),
            genotype.getNodeGroup(0).getNode(i).getValue(0)
        );

        ASSERT_EQ(
            mutatedGenotype->getNodeGroup(0).getNode(i).getValue(1),
            genotype.getNodeGroup(0).getNode(i).getValue(1)
        );
    }
}