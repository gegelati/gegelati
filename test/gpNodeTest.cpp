/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2023 - 2025)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

#include <algorithm>
#include <fstream>
#include <gtest/gtest.h>
#include <numeric>

#include "individual.h"
#include "graphBased/gpNode.h"
#include "util/counterReset.h"
#include "representations/LGP.h"
#include "learn/fakeRepresentation.h"


// Set all file in comment

class GPNodeTest : public ::testing::Test
{
  protected:

    Representations::FakeRepresentation rep;

    virtual void SetUp()
    {
        CounterReset::counterReset();
    }

    virtual void TearDown()
    {
    }
};


TEST_F(GPNodeTest, Constructor)
{
    GraphBased::GPNode* emptyNode;
    GraphBased::GPNode* intNode;
    GraphBased::GPNode* doubleNode;
    GraphBased::GPNode* variantNode;

    ASSERT_NO_THROW(emptyNode = new GraphBased::GPNode(true))
        << "Construction of the empty GPNode failed.";

    std::vector<size_t> intValues = {1,2,3};
    ASSERT_NO_THROW(intNode = new GraphBased::GPNode(intValues))
        << "Construction of the int GPNode failed.";

    std::vector<double> doubleValues = {1.0, 2.0, 3.0};
    ASSERT_NO_THROW(doubleNode = new GraphBased::GPNode(doubleValues))
        << "Construction of the double GPNode failed.";

    std::vector<Data::DataValue> variantValues;
    variantValues.push_back(Data::DataValue::scalar<size_t>(1));
    variantValues.push_back(Data::DataValue::zeros<double>(10, 2));
    ASSERT_NO_THROW(variantNode = new GraphBased::GPNode(variantValues))
        << "Construction of the variant GPNode failed.";

    ASSERT_NO_THROW(delete emptyNode) << "Destruction of the int GPNode failed.";
    ASSERT_NO_THROW(delete intNode) << "Destruction of the int GPNode failed.";
    ASSERT_NO_THROW(delete doubleNode) << "Destruction of the double GPNode failed.";
    ASSERT_NO_THROW(delete variantNode) << "Destruction of the variant GPNode failed.";
}

TEST_F(GPNodeTest, clone) 
{
    std::shared_ptr<const Individual> indiv1 = std::make_shared<Individual>(rep);
    std::vector<Data::DataValue> variantValues;
    variantValues.push_back(Data::DataValue::scalar<size_t>(1));
    variantValues.push_back(Data::DataValue::zeros<double>(10, 2));
    variantValues.push_back(Data::DataValue::scalar<std::shared_ptr<const Individual>>(indiv1));
    GraphBased::GPNode node(variantValues);

    std::unique_ptr<GraphBased::GPNode> clone = node.cloneUniquePtr();
    ASSERT_TRUE(node.getValue(0) == clone->getValue(0)) << "Getting value of the GPNode failed.";
    ASSERT_TRUE(node.getValue(1) == clone->getValue(1)) << "Getting value of the GPNode failed.";
    ASSERT_TRUE(node.getValue(2) == clone->getValue(2)) << "Getting value of the GPNode failed.";

    ASSERT_TRUE(node.hasSameValues(*clone)) << "Nodes should have exactly the same values";
    node.addValue<int>(2);
    ASSERT_FALSE(node.hasSameValues(*clone)) << "Value should not be passed to the clone";
    clone->addValue<double>(3.1);
    ASSERT_FALSE(node.hasSameValues(*clone)) << "For coverage";
}

TEST_F(GPNodeTest, SetGetValue)
{
    std::shared_ptr<const Individual> indiv1 = std::make_shared<Individual>(rep);
    std::vector<Data::DataValue> variantValues;
    variantValues.push_back(Data::DataValue::scalar<size_t>(1));
    variantValues.push_back(Data::DataValue::zeros<double>(10, 2));
    variantValues.push_back(Data::DataValue::scalar<std::shared_ptr<const Individual>>(indiv1));
    GraphBased::GPNode node(variantValues);

    
    ASSERT_TRUE(node.getValue(0) == Data::DataValue::scalar<size_t>(1)) << "Getting value of the GPNode failed.";
    ASSERT_TRUE(node.getValue(1) == Data::DataValue::zeros<double>(10, 2)) << "Getting value of the GPNode failed.";
    ASSERT_TRUE(node.getValue(2) == Data::DataValue::scalar<std::shared_ptr<const Individual>>(indiv1)) << "Getting value of the GPNode failed.";


    ASSERT_NO_THROW(node.setValue<double>(0, 10.5)) << "Setting value of the GPNode failed.";

    ASSERT_TRUE(node.getValue(0) == Data::DataValue::scalar<double>(10.5)) << "Getting value of the GPNode failed.";

    ASSERT_THROW(node.setValue(3, Data::DataValue::scalar<double>(10.5)), std::runtime_error) << "Setting value of the GPNode should have failed.";
    ASSERT_THROW(node.getValue(3), std::runtime_error) << "Getting value of the GPNode should have failed.";

    ASSERT_TRUE(node.getSize() == 3) << "Getting size of the GPNode failed.";
    ASSERT_TRUE(node.getValues().size() == 3) << "Getting values of the GPNode failed.";

    ASSERT_FALSE(node.getIsIntron()) << "Node should not be an intron by default.";
    ASSERT_NO_THROW(node.setIsIntron(true)) << "Setting node to intron state failed";
    ASSERT_TRUE(node.getIsIntron()) << "Node should now be an intron";

    ASSERT_NO_THROW(node.addValue(Data::DataValue::scalar<std::shared_ptr<const Individual>>(indiv1))) << "Adding a value should not fail";
    ASSERT_TRUE(node.getValue(3).getScalar<std::shared_ptr<const Individual>>() == indiv1) << "Value should be indiv1.";
    
    ASSERT_NO_THROW(node.addValue<float>(10)) << "Adding a value should not fail";
    ASSERT_TRUE(node.getValue(4) == Data::DataValue::scalar<float>(10)) << "Value should be a float of 10.";
}

TEST_F(GPNodeTest, IDCounter)
{
    ASSERT_EQ(GraphBased::GPNode::getGPNodeIDCounter(), 0) << "GPNode ID counter should be 0 at the beginning.";

    std::vector<size_t> values = {1, 2, 3};
    std::vector<double> doubleValues = {1.1, 2.1, 3.1};
    GraphBased::GPNode node1(values);
    GraphBased::GPNode node2(doubleValues);

    ASSERT_EQ(node1.getGPNodeID(), 0) << "GPNode ID should be 0.";
    ASSERT_EQ(node2.getGPNodeID(), 1) << "GPNode ID should be 1.";

    ASSERT_EQ(GraphBased::GPNode::getGPNodeIDCounter(), 2) << "GPNode ID counter should be 2 after creating two nodes.";

    node1.setGPNodeID(100);
    ASSERT_EQ(node1.getGPNodeID(), 100) << "Setting GPNode ID failed.";

    ASSERT_EQ(GraphBased::GPNode::getGPNodeIDCounter(), 101) << "GPNode ID counter should be 101 after setting the first node's ID.";
    
    // Check <, = and != operators
    ASSERT_TRUE(node1 != node2) << "operator != failed.";
    ASSERT_TRUE(node2 < node1) << "operator < failed.";
    ASSERT_FALSE(node1 == node2) << "operator == failed.";
}