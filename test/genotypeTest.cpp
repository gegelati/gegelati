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

#include "evolution/genotype.h"
#include "evolution/individual.h"


TEST(GenotypeTest, Constructor)
{
    Evolution::Genotype* genotype;

    ASSERT_NO_THROW(genotype = new Evolution::Genotype()) << "Constructor of Genotype failed.";

    ASSERT_NO_THROW(delete genotype) << "Destructor of Genotype failed.";
}

TEST(GenotypeTest, addRemoveNodeGroup)
{
    Evolution::Genotype genotype;

    ASSERT_EQ(genotype.getSize(), 0) << "Getting size of the Genotype failed.";

    ASSERT_NO_THROW(genotype.addNodeGroup()) << "Adding NodeGroup to the Genotype failed.";
    ASSERT_EQ(genotype.getSize(), 1) << "Getting size of the Genotype failed.";

    Node::NodeGroup* group;
    ASSERT_NO_THROW(group = &genotype.getMutableNodeGroup(0)) << "Getting group failed";
    group->addNode(std::make_unique<Node::GPNode>(std::vector<double>{1.0, 2.0, 3.0}));
    ASSERT_EQ(group->getSize(), 1) << "Group should have size 1";


    ASSERT_NO_THROW(genotype.addNodeGroup(0)) << "Adding NodeGroup to the Genotype failed.";
    ASSERT_EQ(genotype.getSize(), 2) << "Getting size of the Genotype failed.";

    const Node::NodeGroup* group2;
    ASSERT_NO_THROW(group2 = &genotype.getNodeGroup(0)) << "Getting group failed";
    ASSERT_EQ(group2->getSize(), 0) << "Group should have size 0";


    ASSERT_NO_THROW(genotype.removeNodeGroup(0)) << "Removing NodeGroup to the Genotype failed.";
    ASSERT_EQ(genotype.getSize(), 1) << "Getting size of the Genotype failed.";
    
    ASSERT_THROW(genotype.removeNodeGroup(1), std::runtime_error) << "Removing NodeGroup of the Genotype should have failed.";
    ASSERT_THROW(genotype.addNodeGroup(2), std::runtime_error) << "Adding NodeGroup of the Genotype should have failed.";
}

TEST(GenotypeTest, getSizes)
{
    Evolution::Genotype genotype;

    genotype.addNodeGroup();
    Node::NodeGroup& group1 = genotype.getMutableNodeGroup(0);
    group1.addNode(std::make_unique<Node::GPNode>(std::vector<double>{1.0, 2.0, 3.0}));
    group1.addNode(std::make_unique<Node::GPNode>(std::vector<double>{2.0, 3.0, 4.0}));
    group1.addNode(std::make_unique<Node::GPNode>(std::vector<double>{3.0, 4.0, 5.0}));

    genotype.addNodeGroup();
    Node::NodeGroup& group2 = genotype.getMutableNodeGroup(1);
    group2.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{4, 5, 6}));
    group2.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{6, 5, 4}));

    ASSERT_EQ(genotype.getFullSize(), 5) << "Full genotype has 5 nodes";
    ASSERT_EQ(genotype.getSize(), 2) << "Genotype has 2 nodeGroup";
    ASSERT_EQ(genotype.getNodeGroups().size(), 2) << "Genotype has 2 nodeGroup";
    ASSERT_EQ(genotype.getSizes(), std::vector<size_t>({size_t(3), size_t(2)})) << "Genotype has two node groups of size 3 and 2";
}

TEST(GenotypeTest, getEffectiveNodes)
{
    Evolution::Genotype genotype;

    genotype.addNodeGroup();
    Node::NodeGroup& group1 = genotype.getMutableNodeGroup(0);
    group1.addNode(std::make_unique<Node::GPNode>(std::vector<double>{1.0, 2.0, 3.0}));
    group1.addNode(std::make_unique<Node::GPNode>(std::vector<double>{2.0, 3.0, 4.0}));
    group1.addNode(std::make_unique<Node::GPNode>(std::vector<double>{3.0, 4.0, 5.0}));

    genotype.addNodeGroup();
    Node::NodeGroup& group2 = genotype.getMutableNodeGroup(1);
    group2.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{4, 5, 6}));
    group2.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{6, 5, 4}));


    std::vector<std::vector<std::reference_wrapper<const Node::GPNode>>> effectiveNodes = genotype.getEffectiveNodes();
    ASSERT_EQ(effectiveNodes.size(), 2) << "Should be same shape as genotype with no introns";
    ASSERT_EQ(effectiveNodes.at(0).size(), 3) << "Should be same shape as genotype with no introns";
    ASSERT_EQ(effectiveNodes.at(1).size(), 2) << "Should be same shape as genotype with no introns";
    ASSERT_EQ(effectiveNodes.at(0).at(1).get().getValue(1), Node::NodeValue(double(3.0))) << "Should be same value as genotype with no introns";
    ASSERT_EQ(effectiveNodes.at(1).at(1).get().getValue(2), Node::NodeValue(size_t(4))) << "Should be same value as genotype with no introns";

    genotype.getMutableNodeGroup(0).getMutableNode(1).setIsIntron(true);
    genotype.getMutableNodeGroup(1).getMutableNode(0).setIsIntron(true);

    effectiveNodes = genotype.getEffectiveNodes();
    ASSERT_EQ(effectiveNodes.size(), 2) << "Should be same shape";
    ASSERT_EQ(effectiveNodes.at(0).size(), 2) << "Should be different size as genotype with introns";
    ASSERT_EQ(effectiveNodes.at(1).size(), 1) << "Should be different size as genotype with introns";
    ASSERT_EQ(effectiveNodes.at(0).at(1).get().getValue(1), Node::NodeValue(double(4.0))) << "Should be same value as genotype with no introns";
    ASSERT_EQ(effectiveNodes.at(1).at(0).get().getValue(2), Node::NodeValue(size_t(4))) << "Should be same value as genotype with no introns";
}


TEST(GenotypeTest, equality){

    Evolution::Genotype genotype1;
    genotype1.addNodeGroup();
    genotype1.addNodeGroup();
    Node::NodeGroup& group1_1 = genotype1.getMutableNodeGroup(0);
    Node::NodeGroup& group1_2 = genotype1.getMutableNodeGroup(1);

    Evolution::Genotype genotype2;
    genotype2.addNodeGroup();
    genotype2.addNodeGroup();
    Node::NodeGroup& group2_1 = genotype2.getMutableNodeGroup(0);
    Node::NodeGroup& group2_2 = genotype2.getMutableNodeGroup(1);


    ASSERT_TRUE(genotype1 == genotype2) << "Empty genotypes should be equal!";

    group1_1.addNode(std::make_unique<Node::GPNode>(std::vector<double>{1.0, 2.0, 3.0}));

    ASSERT_TRUE(genotype1 != genotype2) << "Should not be equal with different number of nodes";

    group2_1.addNode(std::make_unique<Node::GPNode>(std::vector<double>{1.0, 2.0}));
    ASSERT_TRUE(genotype1 != genotype2) << "Should not be equal with different sizes of nodes";

    group2_1.removeNode(0);
    group2_1.addNode(std::make_unique<Node::GPNode>(std::vector<double>{1.0, 2.0, 4.0}));
    ASSERT_TRUE(genotype1 != genotype2) << "Should not be equal with different values of nodes";
    
    group2_1.removeNode(0);
    group2_1.addNode(std::make_unique<Node::GPNode>(std::vector<double>{1.0, 2.0, 3.0}));
    ASSERT_TRUE(genotype1 == genotype2) << "genotypes should be equal";
    
    group1_2.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{4, 5, 6}));
    group2_1.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{4, 5, 6}));
    ASSERT_TRUE(genotype1 != genotype2) << "Should not be equal with nodes on different groups";

    group2_1.removeNode(1);
    group2_2.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{4, 5, 6}));
    ASSERT_TRUE(genotype1 == genotype2) << "genotypes should be equal";
    ASSERT_TRUE(genotype2 == genotype1) << "genotypes should be equal both directions";
}