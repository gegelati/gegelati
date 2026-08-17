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

#include "evolution/individual.h"
#include "node/gpNode.h"
#include "util/counterReset.h"


// Set all file in comment

class GPNodeTest : public ::testing::Test
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


TEST_F(GPNodeTest, Constructor)
{
    Node::GPNode* intNode;
    Node::GPNode* doubleNode;
    Node::GPNode* indivNode;
    Node::GPNode* variantNode;

    std::vector<size_t> intValues = {1,2,3};
    ASSERT_NO_THROW(intNode = new Node::GPNode(intValues))
        << "Construction of the int GPNode failed.";

    std::vector<double> doubleValues = {1.0, 2.0, 3.0};
    ASSERT_NO_THROW(doubleNode = new Node::GPNode(doubleValues))
        << "Construction of the double GPNode failed.";

    std::shared_ptr<const Evolution::Individual> indiv1 = std::make_shared<Evolution::Individual>();
    std::shared_ptr<const Evolution::Individual> indiv2 = std::make_shared<Evolution::Individual>();
    std::vector<std::shared_ptr<const Evolution::Individual>> indivValues = {indiv1, indiv2};
    ASSERT_NO_THROW(indivNode = new Node::GPNode(indivValues))
        << "Construction of the individual GPNode failed.";

    std::vector<Node::NodeValue> variantValues = {size_t{1}, 2.5, indiv1};
    ASSERT_NO_THROW(variantNode = new Node::GPNode(variantValues))
        << "Construction of the variant GPNode failed.";

    ASSERT_NO_THROW(delete intNode) << "Destruction of the int GPNode failed.";
    ASSERT_NO_THROW(delete doubleNode) << "Destruction of the double GPNode failed.";
    ASSERT_NO_THROW(delete indivNode) << "Destruction of the individual GPNode failed.";
    ASSERT_NO_THROW(delete variantNode) << "Destruction of the variant GPNode failed.";
}

TEST_F(GPNodeTest, SetGetValue)
{
    std::shared_ptr<const Evolution::Individual> indiv1 = std::make_shared<Evolution::Individual>();
    std::vector<Node::NodeValue> variantValues = {size_t{1}, 2.5, indiv1};
    Node::GPNode node(variantValues);

    
    ASSERT_EQ(node.getValue(0), Node::NodeValue(size_t{1})) << "Getting value of the GPNode failed.";
    ASSERT_EQ(node.getValue(1), Node::NodeValue(2.5)) << "Getting value of the GPNode failed.";
    ASSERT_EQ(node.getValue(2), Node::NodeValue(indiv1)) << "Getting value of the GPNode failed.";


    ASSERT_NO_THROW(node.setValue(0, size_t{5})) << "Setting value of the GPNode failed.";
    ASSERT_NO_THROW(node.setValue(1, indiv1)) << "Setting value of the GPNode failed.";
    ASSERT_NO_THROW(node.setValue(2, 2.9)) << "Setting value of the GPNode failed.";

    ASSERT_EQ(node.getValue(0), Node::NodeValue(size_t{5})) << "Getting value of the GPNode failed.";
    ASSERT_EQ(node.getValue(1), Node::NodeValue(indiv1)) << "Getting value of the GPNode failed.";
    ASSERT_EQ(node.getValue(2), Node::NodeValue(2.9)) << "Getting value of the GPNode failed.";

    ASSERT_THROW(node.setValue(3, size_t(25)), std::runtime_error) << "Setting value of the GPNode should have failed.";
    ASSERT_THROW(node.getValue(3), std::runtime_error) << "Getting value of the GPNode should have failed.";

    ASSERT_EQ(node.getSize(), 3) << "Getting size of the GPNode failed.";
    ASSERT_EQ(node.getValues(), std::vector<Node::NodeValue>({size_t{5}, indiv1, 2.9})) << "Getting values of the GPNode failed.";

    ASSERT_FALSE(node.getIsIntron()) << "Node should not be an intron by default.";
    ASSERT_NO_THROW(node.setIsIntron(true)) << "Setting node to intron state failed";
    ASSERT_TRUE(node.getIsIntron()) << "Node should now be an intron";
}

TEST_F(GPNodeTest, IDCounter)
{
    ASSERT_EQ(Node::GPNode::getGPNodeIDCounter(), 0) << "GPNode ID counter should be 0 at the beginning.";

    std::vector<size_t> values = {1, 2, 3};
    std::vector<double> doubleValues = {1.1, 2.1, 3.1};
    Node::GPNode node1(values);
    Node::GPNode node2(doubleValues);

    ASSERT_EQ(node1.getGPNodeID(), 0) << "GPNode ID should be 0.";
    ASSERT_EQ(node2.getGPNodeID(), 1) << "GPNode ID should be 1.";

    ASSERT_EQ(Node::GPNode::getGPNodeIDCounter(), 2) << "GPNode ID counter should be 2 after creating two nodes.";

    node1.setGPNodeID(100);
    ASSERT_EQ(node1.getGPNodeID(), 100) << "Setting GPNode ID failed.";

    ASSERT_EQ(Node::GPNode::getGPNodeIDCounter(), 101) << "GPNode ID counter should be 101 after setting the first node's ID.";
    
    // Check <, = and != operators
    ASSERT_TRUE(node1 != node2) << "operator != failed.";
    ASSERT_TRUE(node2 < node1) << "operator < failed.";
    ASSERT_FALSE(node1 == node2) << "operator == failed.";
}