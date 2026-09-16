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

#include "graphBased/nodeGroup.h"
#include "individual.h"


TEST(NodeGroupTest, Constructor)
{
    GraphBased::NodeGroup* nodeGroup;

    ASSERT_NO_THROW(nodeGroup = new GraphBased::NodeGroup()) << "Constructor of NodeGroup failed.";

    ASSERT_NO_THROW(delete nodeGroup) << "Destructor of NodeGroup failed.";
}

TEST(NodeGroupTest, changeNodes)
{
    GraphBased::NodeGroup nodeGroup;

    ASSERT_EQ(nodeGroup.getSize(), 0) << "Getting size of the NodeGroup failed.";

    ASSERT_NO_THROW(nodeGroup.addNode(std::make_unique<GraphBased::GPNode>(std::vector<double>{1.0, 2.0, 3.0}))) << "Adding GPNode to the NodeGroup failed.";

    ASSERT_EQ(nodeGroup.getSize(), 1) << "Getting size of the NodeGroup failed.";

    ASSERT_NO_THROW(nodeGroup.addNode(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{4, 5, 6}), 0)) << "Adding GPNode to the NodeGroup failed.";

    ASSERT_EQ(nodeGroup.getSize(), 2) << "Getting size of the NodeGroup failed.";
    ASSERT_EQ(nodeGroup.getNodes().size(), 2) << "Size not equal";
    ASSERT_EQ(nodeGroup.getNode(0).getValues().at(0).getScalar<size_t>(), size_t{4}) << "Getting values of the GPNode failed.";
    // for coverage of const method
    const GraphBased::NodeGroup& cNodeGroup = nodeGroup ;
    ASSERT_EQ(cNodeGroup.getNode(1).getValues().at(2).getScalar<double>(), 3.0) << "Getting values of the GPNode failed.";

    ASSERT_THROW(nodeGroup.addNode(std::make_unique<GraphBased::GPNode>(std::vector<double>{7.0, 8.0, 9.0}), size_t(3)), std::runtime_error) << "Adding GPNode to the NodeGroup should have failed.";
    ASSERT_THROW(nodeGroup.getNode(2), std::runtime_error) << "Getting GPNode of the NodeGroup should have failed.";
    ASSERT_THROW(cNodeGroup.getNode(2), std::runtime_error) << "Getting GPNode of the NodeGroup should have failed.";

    ASSERT_NO_THROW(nodeGroup.removeNode(0)) << "Removing GPNode to the NodeGroup failed.";
    ASSERT_EQ(nodeGroup.getSize(), 1) << "Getting size of the NodeGroup failed.";
    ASSERT_NO_THROW(nodeGroup.setNode(std::make_unique<GraphBased::GPNode>(std::vector<float>{1.6, 2.8, 3.9, 5.2}), 0)) << "Setting a node failed";
    ASSERT_EQ(cNodeGroup.getNode(0).getValues().at(3).getScalar<float>(), 5.2f) << "Getting values of the GPNode failed.";
    
    ASSERT_THROW(nodeGroup.removeNode(1), std::runtime_error) << "Removing GPNode of the NodeGroup should have failed.";
    ASSERT_THROW(nodeGroup.setNode(std::make_unique<GraphBased::GPNode>(std::vector<float>{1.6, 2.8, 3.9, 5.2}), 1), std::runtime_error) << "Set of GPNode of the NodeGroup should have failed.";

}


TEST(NodeGroupTest, equalityAndClone){

    GraphBased::NodeGroup group1;
    GraphBased::NodeGroup group2;

    ASSERT_TRUE (group1 == group2) << "Empty groups should be equal!";

    group1.addNode(std::make_unique<GraphBased::GPNode>(std::vector<double>{1.0, 2.0, 3.0}));

    ASSERT_TRUE (group1 != group2) << "Should not be equal with different number of nodes";

    group2.addNode(std::make_unique<GraphBased::GPNode>(std::vector<double>{1.0, 2.0}));
    ASSERT_TRUE(group1 != group2) << "Should not be equal with different sizes of nodes";

    group2.removeNode(0);
    group2.addNode(std::make_unique<GraphBased::GPNode>(std::vector<double>{1.0, 2.0, 4.0}));
    ASSERT_TRUE(group1 != group2) << "Should not be equal with different values of nodes";
    
    group2.removeNode(0);
    group2.addNode(std::make_unique<GraphBased::GPNode>(std::vector<double>{1.0, 2.0, 3.0}));
    ASSERT_TRUE (group1 == group2) << "groups should be equal";
    
    group1.addNode(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{4, 5, 6}));
    group2.addNode(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{4, 5, 6}));
    
    ASSERT_TRUE (group1 == group2) << "groups should be equal";
    ASSERT_TRUE (group2 == group1) << "groups should be equal both directions";

    std::unique_ptr<GraphBased::NodeGroup> clone = group1.cloneUniquePtr();
    ASSERT_TRUE (*clone == group1) << "Clone should be equal to its origin";
    ASSERT_TRUE (*clone == group2) << "Clone should be equal to something equal to its origin";

}