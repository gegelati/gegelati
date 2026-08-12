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

#include "evolution/mutation.h"


// Set all file in comment

class MutationTest : public ::testing::Test
{
  protected:
    RNG::RNG rng = RNG::RNG(0);

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};


TEST_F(MutationTest, Constructor)
{
    Evolution::Mutation* mutation;

    ASSERT_NO_THROW(mutation = new Evolution::Mutation()) << "Constructor of mutation failed.";

    ASSERT_NO_THROW(delete mutation) << "Destructor of mutation failed.";
}

TEST_F(MutationTest, createRandomNode)
{
    Evolution::Mutation mutation;
    std::unique_ptr<Node::GPNode> node;
    
    std::vector<size_t> maxRanges = {8, 4, 2, 8, 2, 8};
    Node::NodeTemplate nodeTemplate;
    for(size_t idx = 0; idx < maxRanges.size(); idx++) {
        auto config(std::make_shared<Node::NodeValueConfiguration>(std::make_pair(size_t(0), maxRanges[idx])));
        nodeTemplate.addValueTemplate(std::make_shared<Node::NodeValueTemplate>(config));
    }
    
    ASSERT_NO_THROW(node = std::move(mutation.createRandomNode(nodeTemplate, rng))) << "Creation of random node failed.";
    
    ASSERT_EQ(node->getSize(), 6) << "Node does not have the right size";
    
    for(size_t idx = 0; idx < node->getSize(); idx++) {
        ASSERT_TRUE(std::holds_alternative<size_t>(node->getValue(idx))) << "Node value is not size_t";
        ASSERT_LT(std::get<size_t>(node->getValue(idx)), maxRanges[idx]) << "Node value is out of range";
    }
}

TEST_F(MutationTest, initRandomIndividual)
{
    Evolution::Mutation mutation;
    Evolution::Individual indiv;


    std::vector<size_t> maxRanges = {8, 4, 2, 8, 2, 8};
    auto nodeTemplate(std::make_shared<Node::NodeTemplate>());
    for(size_t idx = 0; idx < maxRanges.size(); idx++) {
        auto config(std::make_shared<Node::NodeValueConfiguration>(std::make_pair(size_t(0), maxRanges[idx])));
        nodeTemplate->addValueTemplate(std::make_shared<Node::NodeValueTemplate>(config));
    }

    Node::GenotypeTemplate genotypeTemplate;
    genotypeTemplate.addNodeTemplate(nodeTemplate, 5, 10);

    ASSERT_NO_THROW(mutation.initRandomIndividual(indiv, genotypeTemplate, rng)) << "Initialization of individual failed";

    ASSERT_GE(indiv.getSize(), 5) << "Individual does not have enough nodes";
    ASSERT_LE(indiv.getSize(), 10) << "Individual has too much nodes";

    for(size_t idx = 0; idx < indiv.getSize(); idx++) {
        ASSERT_EQ(indiv.getGPNode(idx).getSize(), 6) << "Nodes have different sizes";
    }
}