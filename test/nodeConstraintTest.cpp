
#include <fstream>
#include <gtest/gtest.h>

#include "graphBased/nodeConstraint.h"
#include "graphBased/genotypeConstraint.h"

#include "dimensions/numericRange.h"

TEST(NodeConstraintTest, Constructor) 
{
    GraphBased::NodeConstraint* nodeConstraint;

    ASSERT_NO_THROW(nodeConstraint = new GraphBased::NodeConstraint()) << "Creation of constraint failed";
    
    ASSERT_NO_THROW(delete nodeConstraint) << "Destructor of constraint failed.";
}

TEST(NodeConstraintTest, addValue) {
    GraphBased::NodeConstraint nodeConstraint;

    ASSERT_EQ(nodeConstraint.size(), 0) << "Size of the constraint should be 0";
    ASSERT_THROW(nodeConstraint.getConstraintAt(0), std::runtime_error) << "Getting a constraint with empty constraint should fail";

    ASSERT_NO_THROW(nodeConstraint.addConstraint(Dimensions::NumericRange<int>::between(0, 0))) << "Should not have fail to add constraint";
    ASSERT_EQ(nodeConstraint.size(), 1) << "Size of the constraint should be 0";

    Data::DataValue value = Data::DataValue::scalar<int>(0);
    Data::DataValue falseValue = Data::DataValue::scalar<int>(1);
    ASSERT_TRUE(nodeConstraint.getConstraintAt(0).accepts(value)) << "Numeric range should accept the value";
    ASSERT_FALSE(nodeConstraint.getConstraintAt(0).accepts(falseValue)) << "Numeric range should not accept the value";

    std::unique_ptr<GraphBased::NodeConstraint> clone = nodeConstraint.cloneUniquePtr();
    ASSERT_TRUE(clone->getConstraintAt(0).accepts(value)) << "Numeric range should accept the value";
    ASSERT_FALSE(clone->getConstraintAt(0).accepts(falseValue)) << "Numeric range should not accept the value";
}



TEST(GenotypeConstraintTest, Constructor) 
{
    GraphBased::GenotypeConstraint* genotypeConstraint;
    GraphBased::GenotypeConstraint* genotypeConstraint1;

    GraphBased::NodeConstraint nodeConstraint;

    ASSERT_NO_THROW(genotypeConstraint = new GraphBased::GenotypeConstraint()) << "Creation of constraint failed";
    ASSERT_NO_THROW(genotypeConstraint1 = new GraphBased::GenotypeConstraint(nodeConstraint, 1, 2)) << "Creation of constraint failed";
    
    ASSERT_NO_THROW(delete genotypeConstraint) << "Destructor of constraint failed.";
    ASSERT_NO_THROW(delete genotypeConstraint1) << "Destructor of constraint failed.";

}

TEST(GenotypeConstraintTest, addValue) {
    GraphBased::GenotypeConstraint genotypeConstraint;
    ASSERT_EQ(genotypeConstraint.size(), 0) << "Size of genotypeConstraint mismatch.";

    GraphBased::NodeConstraint nodeConstraint0;
    ASSERT_THROW(genotypeConstraint.addNodeConstraint(nodeConstraint0), std::runtime_error) << "Should have failed with empty value.";

    
    nodeConstraint0.addConstraint(Dimensions::NumericRange<int>::between(0, 0));
    nodeConstraint0.addConstraint(Dimensions::NumericRange<int>::between(42, 42)); 
    ASSERT_NO_THROW(genotypeConstraint.addNodeConstraint(nodeConstraint0, 1, 5)) << "Should not have failed.";
    ASSERT_EQ(genotypeConstraint.size(), 1) << "Size of genotypeConstraint mismatch.";

    GraphBased::NodeConstraint nodeConstraint1;
    nodeConstraint1.addConstraint(Dimensions::NumericRange<int>::between(0, 42)); 
    ASSERT_NO_THROW(genotypeConstraint.addNodeConstraint(nodeConstraint1, 5)) << "Should not have failed.";
    

    Data::DataValue value = Data::DataValue::scalar<int>(0);
    const GraphBased::NodeConstraint& nodeConstraintGet0 = genotypeConstraint.getNodeConstraintAt(0);
    const GraphBased::NodeConstraint& nodeConstraintGet1 = genotypeConstraint.getNodeConstraintAt(1);
    ASSERT_EQ(nodeConstraintGet0.size(), 2) << "Size should be equal";
    ASSERT_EQ(nodeConstraintGet1.size(), 1) << "Size should be equal";
    ASSERT_TRUE(nodeConstraintGet0.getConstraintAt(0).accepts(value)) << "Value should be accepted";
    ASSERT_FALSE(nodeConstraintGet0.getConstraintAt(1).accepts(value)) << "Value should be accepted";
    ASSERT_TRUE(nodeConstraintGet1.getConstraintAt(0).accepts(value)) << "Value should be accepted";

    ASSERT_THROW(genotypeConstraint.getNodeConstraintAt(2), std::runtime_error) << "Should have failed with out of range.";
    ASSERT_THROW(genotypeConstraint.getRangeAt(2), std::runtime_error) << "Should have failed with out of range.";


    auto clone = genotypeConstraint.cloneUniquePtr();
    ASSERT_EQ(clone->size(), 2) << "Size of genotypeConstraint mismatch.";
    
    ASSERT_EQ(clone->getNodeConstraintAt(0).size(), 2) << "Size should be equal";
    ASSERT_EQ(clone->getNodeConstraintAt(1).size(), 1) << "Size should be equal";
    ASSERT_TRUE(clone->getNodeConstraintAt(0).getConstraintAt(0).accepts(value)) << "Value should be accepted";
    ASSERT_FALSE(clone->getNodeConstraintAt(0).getConstraintAt(1).accepts(value)) << "Value should be accepted";
    ASSERT_TRUE(clone->getNodeConstraintAt(1).getConstraintAt(0).accepts(value)) << "Value should be accepted";
}