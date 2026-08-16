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

#include "node/nodeGroup.h"
#include "evolution/individual.h"


TEST(NodeGroupTest, Constructor)
{
    Node::NodeGroup* nodeGroup;

    ASSERT_NO_THROW(nodeGroup = new Node::NodeGroup()) << "Constructor of NodeGroup failed.";

    ASSERT_NO_THROW(delete nodeGroup) << "Destructor of NodeGroup failed.";
}

TEST(NodeGroupTest, addRemoveNodes)
{
    Node::NodeGroup nodeGroup;

    ASSERT_EQ(nodeGroup.getSize(), 0) << "Getting size of the NodeGroup failed.";

    ASSERT_NO_THROW(nodeGroup.addNode(std::make_unique<Node::GPNode>(std::vector<double>{1.0, 2.0, 3.0}))) << "Adding GPNode to the NodeGroup failed.";

    ASSERT_EQ(nodeGroup.getSize(), 1) << "Getting size of the NodeGroup failed.";

    ASSERT_NO_THROW(nodeGroup.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{4, 5, 6}), size_t(0))) << "Adding GPNode to the NodeGroup failed.";

    ASSERT_EQ(nodeGroup.getSize(), 2) << "Getting size of the NodeGroup failed.";
    ASSERT_EQ(nodeGroup.getNode(0).getValues().at(0), Node::NodeValue(size_t{4})) << "Getting values of the GPNode failed.";
    ASSERT_EQ(nodeGroup.getMutableNode(1).getValues().at(2), Node::NodeValue(3.0)) << "Getting values of the GPNode failed.";

    ASSERT_THROW(nodeGroup.addNode(std::make_unique<Node::GPNode>(std::vector<double>{7.0, 8.0, 9.0}), size_t(3)), std::runtime_error) << "Adding GPNode to the NodeGroup should have failed.";
    ASSERT_THROW(nodeGroup.getNode(2), std::runtime_error) << "Getting GPNode of the NodeGroup should have failed.";
    ASSERT_THROW(nodeGroup.getMutableNode(2), std::runtime_error) << "Getting GPNode of the NodeGroup should have failed.";

    ASSERT_NO_THROW(nodeGroup.removeNode(0)) << "Removing GPNode to the NodeGroup failed.";
    ASSERT_EQ(nodeGroup.getSize(), 1) << "Getting size of the NodeGroup failed.";
    ASSERT_EQ(nodeGroup.getMutableNode(0).getValues().at(2), Node::NodeValue(3.0)) << "Getting values of the GPNode failed.";
    
    ASSERT_THROW(nodeGroup.removeNode(1), std::runtime_error) << "Removing GPNode of the NodeGroup should have failed.";
}

TEST(NodeGroupTest, GetNodeGroup)
{
    Node::NodeGroup nodeGroup;

    ASSERT_NO_THROW(nodeGroup.addNode(std::make_unique<Node::GPNode>(std::vector<double>{1.0, 2.0, 3.0}))) << "Adding GPNode to the NodeGroup failed.";
    ASSERT_NO_THROW(nodeGroup.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{4, 5, 6}))) << "Adding GPNode to the NodeGroup failed.";

    std::vector<std::reference_wrapper<const Node::GPNode>> nodes = nodeGroup.getNodes();
    ASSERT_EQ(nodes.size(), 2) << "Getting nodeGroup of the NodeGroup failed.";
    ASSERT_EQ(nodes[0].get().getValues().at(0), Node::NodeValue(1.0)) << "Getting values of the GPNode failed.";
    ASSERT_EQ(nodes[1].get().getValues().at(0), Node::NodeValue(size_t{4})) << "Getting values of the GPNode failed.";
}

TEST(NodeGroupTest, equality){

    Node::NodeGroup group1;
    Node::NodeGroup group2;

    ASSERT_TRUE (group1 == group2) << "Empty groups should be equal!";

    group1.addNode(std::make_unique<Node::GPNode>(std::vector<double>{1.0, 2.0, 3.0}));

    ASSERT_TRUE (group1 != group2) << "Should not be equal with different number of nodes";

    group2.addNode(std::make_unique<Node::GPNode>(std::vector<double>{1.0, 2.0}));
    ASSERT_TRUE(group1 != group2) << "Should not be equal with different sizes of nodes";

    group2.removeNode(0);
    group2.addNode(std::make_unique<Node::GPNode>(std::vector<double>{1.0, 2.0, 4.0}));
    ASSERT_TRUE(group1 != group2) << "Should not be equal with different values of nodes";
    
    group2.removeNode(0);
    group2.addNode(std::make_unique<Node::GPNode>(std::vector<double>{1.0, 2.0, 3.0}));
    ASSERT_TRUE (group1 == group2) << "groups should be equal";
    
    group1.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{4, 5, 6}));
    group2.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{4, 5, 6}));
    
    ASSERT_TRUE (group1 == group2) << "groups should be equal";
    ASSERT_TRUE (group1 == group2) << "groups should be equal both directions";
}