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
    Node::GPNode* node1;
    Node::GPNode* node2;

    ASSERT_NO_THROW(node1 = new Node::GPNode({1, 2, 3}, {10, 20, 30}))
        << "Construction of the GPNode failed.";

        
    ASSERT_NO_THROW(node2 = new Node::GPNode( {10, 20, 30}))
        << "Construction of the GPNode failed.";

    ASSERT_NO_THROW(delete node1) << "Destruction of the GPNode failed.";
    ASSERT_NO_THROW(delete node2) << "Destruction of the GPNode failed.";
}

TEST_F(GPNodeTest, SetGetValue)
{
    Node::GPNode node({1, 2, 3}, {10, 20, 30});

    ASSERT_NO_THROW(node.setValue(0, 5)) << "Setting value of the GPNode failed.";
    ASSERT_NO_THROW(node.setValue(1, 15)) << "Setting value of the GPNode failed.";
    ASSERT_NO_THROW(node.setValue(2, 25)) << "Setting value of the GPNode failed.";

    ASSERT_THROW(node.setValue(0, 15), std::runtime_error) << "Setting value of the GPNode should have failed.";
    ASSERT_THROW(node.setValue(3, 25), std::runtime_error) << "Setting value of the GPNode should have failed.";


    ASSERT_EQ(node.getValue(0), 5) << "Getting value of the GPNode failed.";
    ASSERT_EQ(node.getValue(1), 15) << "Getting value of the GPNode failed.";
    ASSERT_EQ(node.getValue(2), 25) << "Getting value of the GPNode failed.";

    ASSERT_THROW(node.getValue(3), std::runtime_error) << "Getting value of the GPNode should have failed.";


    ASSERT_EQ(node.getMaxRange(0), 10) << "Getting range of the GPNode failed.";
    ASSERT_THROW(node.getMaxRange(3), std::runtime_error) << "Getting range of the GPNode should have failed.";


    ASSERT_EQ(node.getSize(), 3) << "Getting size of the GPNode failed.";
    ASSERT_EQ(node.getValues(), std::vector<size_t>({5, 15, 25})) << "Getting values of the GPNode failed.";
    ASSERT_EQ(node.getMaxRanges(), std::vector<size_t>({10, 20, 30})) << "Getting values of the GPNode failed.";
}

TEST_F(GPNodeTest, IDCounter)
{
    ASSERT_EQ(Node::GPNode::getGPNodeIDCounter(), 0) << "GPNode ID counter should be 0 at the beginning.";

    Node::GPNode node1({1, 2, 3}, {10, 20, 30});
    Node::GPNode node2({1, 2, 3}, {10, 20, 30});

    size_t id1 = node1.getGPNodeID();
    size_t id2 = node2.getGPNodeID();

    ASSERT_EQ(Node::GPNode::getGPNodeIDCounter(), 2) << "GPNode ID counter should be 2 after creating two nodes.";

    ASSERT_NE(id1, id2) << "GPNode IDs should be different.";

    node1.setGPNodeID(100);
    ASSERT_EQ(node1.getGPNodeID(), 100) << "Setting GPNode ID failed.";

    ASSERT_EQ(Node::GPNode::getGPNodeIDCounter(), 101) << "GPNode ID counter should be 101 after setting the first node's ID.";

}