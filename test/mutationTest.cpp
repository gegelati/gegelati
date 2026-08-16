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
    size_t nbRepeats = 100;

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

TEST_F(MutationTest, sampleNodeValue)
{
    Evolution::Mutation mutation;
    Node::NodeValue value;

    Node::NodeValueTemplate emptyValueTemplate;
    ASSERT_THROW(value = mutation.sampleNodeValue(emptyValueTemplate, rng), std::runtime_error) << "Should fail with empty value";

    // Range of size_t
    auto config0(std::make_shared<Node::NodeValueConfiguration>(std::make_pair(size_t(5), size_t(10))));
    Node::NodeValueTemplate valueTemplate0(config0);
    for(size_t idxRepeat = 0; idxRepeat < nbRepeats; idxRepeat++) {
        ASSERT_NO_THROW(value = mutation.sampleNodeValue(valueTemplate0, rng)) << "Should not have failed";
        ASSERT_TRUE(std::holds_alternative<size_t>(value)) << "Value should be a size_t";
        ASSERT_GE(std::get<size_t>(value), 5) << "Value should be above or equal to 5";
        ASSERT_LT(std::get<size_t>(value), 10) << "Value should be below to 10";
    }
    
    // Range of double
    auto config1(std::make_shared<Node::NodeValueConfiguration>(std::make_pair(-2.0, 3.0)));
    Node::NodeValueTemplate valueTemplate1(config1);
    for(size_t idxRepeat = 0; idxRepeat < nbRepeats; idxRepeat++) {
        ASSERT_NO_THROW(value = mutation.sampleNodeValue(valueTemplate1, rng)) << "Should not have failed";
        ASSERT_TRUE(std::holds_alternative<double>(value)) << "Value should be a double";
        ASSERT_GE(std::get<double>(value), -2.0) << "Value should be above or equal to -2.0";
        ASSERT_LE(std::get<double>(value), 3.0) << "Value should be below or equal to 3.0";
    }

    // Vector of diverse accepted values.
    Evolution::Individual indiv;
    std::vector<Node::NodeValue> vectValues = {size_t(0), size_t(1), 5.0, 5.5, indiv};
    auto config2(std::make_shared<Node::NodeValueConfiguration>(vectValues));
    Node::NodeValueTemplate valueTemplate2(config2);
    for(size_t idxRepeat = 0; idxRepeat < nbRepeats; idxRepeat++) {
        ASSERT_NO_THROW(value = mutation.sampleNodeValue(valueTemplate2, rng)) << "Should not have failed";
        bool isValueContained = false;
        for(size_t idx = 0; idx < vectValues.size() && !isValueContained; idx++) {
            isValueContained = (value == vectValues.at(idx));
        }
        ASSERT_TRUE(isValueContained) << "Value should have been found in the vector";
    }

    // Vector of diverse accepted values AND range of doubles
    Node::NodeValueTemplate valueTemplate3({config1, config2});
    for(size_t idxRepeat = 0; idxRepeat < nbRepeats; idxRepeat++) {
        ASSERT_NO_THROW(value = mutation.sampleNodeValue(valueTemplate3, rng)) << "Should not have failed";
        bool isRange = false;
        if(std::holds_alternative<double>(value)) {
            isRange = std::get<double>(value) >= -2.0 && std::get<double>(value) <= 3.0;
        }
        bool isValueContained = false;
        for(size_t idx = 0; idx < vectValues.size() && !isValueContained; idx++) {
            isValueContained = (value == vectValues.at(idx));
        }
        ASSERT_TRUE(isRange || isValueContained) << "Value Should be sampled in range or in vector";
        ASSERT_FALSE(isRange && isValueContained) << "Value should node be both in range and vector";
    }
}

TEST_F(MutationTest, createRandomNode)
{
    Evolution::Mutation mutation;
    std::unique_ptr<Node::GPNode> node;
    
    Node::NodeTemplate nodeEmptyTemplate;
    ASSERT_THROW(mutation.createRandomNode(nodeEmptyTemplate, rng), std::runtime_error) << "Should failed with empty template"; 

    auto config0(std::make_shared<Node::NodeValueConfiguration>(std::make_pair(size_t(5), size_t(10))));
    auto valueTemplate0(std::make_shared<Node::NodeValueTemplate>(config0));

    auto config1(std::make_shared<Node::NodeValueConfiguration>(std::make_pair(-2.0, 3.0)));
    auto valueTemplate1(std::make_shared<Node::NodeValueTemplate>(config1));

    Evolution::Individual indiv0; 
    Evolution::Individual indiv1;
    std::vector<Node::NodeValue> vectValues = {indiv0, indiv1};
    auto config2(std::make_shared<Node::NodeValueConfiguration>(vectValues));
    auto valueTemplate2(std::make_shared<Node::NodeValueTemplate>(config2));

    Node::NodeTemplate nodeTemplate({valueTemplate0, valueTemplate1, valueTemplate2, valueTemplate0});
    for(size_t idxRepeat = 0; idxRepeat < nbRepeats; idxRepeat++) {
        ASSERT_NO_THROW(node = std::move(mutation.createRandomNode(nodeTemplate, rng))) << "Creation of random node failed.";
        
        ASSERT_EQ(node->getSize(), 4) << "Node does not have the right size";
        
        // Node value 0
        ASSERT_TRUE(std::holds_alternative<size_t>(node->getValue(0))) << "Node value is not size_t";
        ASSERT_GE(std::get<size_t>(node->getValue(0)), 5) << "Value should be above or equal to 5";
        ASSERT_LT(std::get<size_t>(node->getValue(0)), 10) << "Value should be below to 10";
    
        // Node value 1
        ASSERT_TRUE(std::holds_alternative<double>(node->getValue(1))) << "Node value is not double";
        ASSERT_GE(std::get<double>(node->getValue(1)), -2.0) << "Value should be above or equal to -2.0";
        ASSERT_LT(std::get<double>(node->getValue(1)), 3.0) << "Value should be below or equal to 3.0";
    
        // Node value 2
        ASSERT_TRUE(std::holds_alternative<std::reference_wrapper<const Evolution::Individual>>(node->getValue(2))) << "Node value is not indiv";
        const Evolution::Individual& indivSampled = std::get<std::reference_wrapper<const Evolution::Individual>>(node->getValue(2));
        ASSERT_TRUE(indivSampled == indiv0 || indivSampled == indiv1) << "Value should be either indiv0 or indiv1";
    
        // Node value 3
        ASSERT_TRUE(std::holds_alternative<size_t>(node->getValue(3))) << "Node value is not size_t";
        ASSERT_GE(std::get<size_t>(node->getValue(3)), 5) << "Value should be above or equal to 5";
        ASSERT_LT(std::get<size_t>(node->getValue(3)), 10) << "Value should be below to 10";
    }
}

TEST_F(MutationTest, initRandomIndividual)
{
    Evolution::Mutation mutation;

    Evolution::Individual emptyIndiv;
    Node::GenotypeTemplate genotypeEmptyTemplate;
    ASSERT_THROW(mutation.initRandomIndividual(emptyIndiv, genotypeEmptyTemplate, rng), std::runtime_error) << "Should failed with empty template"; 

    auto config0(std::make_shared<Node::NodeValueConfiguration>(std::make_pair(size_t(5), size_t(10))));
    auto valueTemplate0(std::make_shared<Node::NodeValueTemplate>(config0));

    auto config1(std::make_shared<Node::NodeValueConfiguration>(std::make_pair(-2.0, 3.0)));
    auto valueTemplate1(std::make_shared<Node::NodeValueTemplate>(config1));

    Evolution::Individual indiv0; 
    Evolution::Individual indiv1;
    std::vector<Node::NodeValue> vectValues = {indiv0, indiv1};
    auto config2(std::make_shared<Node::NodeValueConfiguration>(vectValues));
    auto valueTemplate2(std::make_shared<Node::NodeValueTemplate>(config2));

    std::vector<std::shared_ptr<const Node::NodeValueTemplate>> vect{valueTemplate0, valueTemplate1, valueTemplate2};
    auto nodeTemplate0(std::make_shared<Node::NodeTemplate>(vect));
    auto nodeTemplate1(std::make_shared<Node::NodeTemplate>(valueTemplate1));

    Node::GenotypeTemplate genotypeTemplate;
    genotypeTemplate.addNodeTemplate(nodeTemplate0, 5, 10);
    genotypeTemplate.addNodeTemplate(nodeTemplate1);

    
    for(size_t idxRepeat = 0; idxRepeat < nbRepeats; idxRepeat++) {
        Evolution::Individual initIndiv;
        ASSERT_NO_THROW(mutation.initRandomIndividual(initIndiv, genotypeTemplate, rng)) << "Initialization of individual failed";

        ASSERT_GE(initIndiv.getSize(), 5 + 1) << "Individual does not have enough nodes";
        ASSERT_LE(initIndiv.getSize(), 10 + 1) << "Individual has too much nodes";

        for(size_t idxNode = 0; idxNode < initIndiv.getSize(); idxNode++) {
            const Node::GPNode& node = initIndiv.getGPNode(idxNode);
            if(idxNode < initIndiv.getSize() - 1) {
                
                ASSERT_EQ(node.getSize(), 3) << "Node does not have the right size";

                // Node value 0
                ASSERT_TRUE(std::holds_alternative<size_t>(node.getValue(0))) << "Node value is not size_t";
                ASSERT_GE(std::get<size_t>(node.getValue(0)), 5) << "Value should be above or equal to 5";
                ASSERT_LT(std::get<size_t>(node.getValue(0)), 10) << "Value should be below to 10";
            
                // Node value 1
                ASSERT_TRUE(std::holds_alternative<double>(node.getValue(1))) << "Node value is not double";
                ASSERT_GE(std::get<double>(node.getValue(1)), -2.0) << "Value should be above or equal to -2.0";
                ASSERT_LT(std::get<double>(node.getValue(1)), 3.0) << "Value should be below or equal to 3.0";
            
                // Node value 2
                ASSERT_TRUE(std::holds_alternative<std::reference_wrapper<const Evolution::Individual>>(node.getValue(2))) << "Node value is not indiv";
                const Evolution::Individual& indivSampled = std::get<std::reference_wrapper<const Evolution::Individual>>(node.getValue(2));
                ASSERT_TRUE(indivSampled == indiv0 || indivSampled == indiv1) << "Value should be either indiv0 or indiv1";
            } else {
                ASSERT_EQ(node.getSize(), 1) << "Node does not have the right size";
                // Node value 0
                ASSERT_TRUE(std::holds_alternative<double>(node.getValue(0))) << "Node value is not double";
                ASSERT_GE(std::get<double>(node.getValue(0)), -2.0) << "Value should be above or equal to -2.0";
                ASSERT_LT(std::get<double>(node.getValue(0)), 3.0) << "Value should be below or equal to 3.0";
            }
        }
    }
}

TEST_F(MutationTest, mutateNode)
{
    Evolution::Mutation mutation;

    auto config0(std::make_shared<Node::NodeValueConfiguration>(std::make_pair(size_t(5), size_t(10))));
    auto valueTemplate0(std::make_shared<Node::NodeValueTemplate>(config0));

    auto config1(std::make_shared<Node::NodeValueConfiguration>(std::make_pair(-2.0, 3.0)));
    auto valueTemplate1(std::make_shared<Node::NodeValueTemplate>(config1));

    Evolution::Individual indiv0; 
    Evolution::Individual indiv1;
    std::vector<Node::NodeValue> vectValues = {indiv0, indiv1};
    auto config2(std::make_shared<Node::NodeValueConfiguration>(vectValues));
    auto valueTemplate2(std::make_shared<Node::NodeValueTemplate>(config2));

    Node::NodeTemplate nodeTemplate({valueTemplate0, valueTemplate1, valueTemplate2, valueTemplate0});


    std::vector<Node::NodeValue> vect{size_t(6), 1.0, indiv0, size_t(8)};
    size_t globalNbChanges;
    for(size_t idxRepeat = 0; idxRepeat < nbRepeats; idxRepeat++) {
        Node::GPNode node(vect);
        
        ASSERT_NO_THROW(mutation.mutateNode(node, nodeTemplate, rng)) << "Mutating a node failed";
        size_t nbValueChanged = 0;
        for(size_t idx = 0; idx < node.getSize(); idx++) {
            const Node::NodeValue& value = node.getValue(idx);
            if(value != vect.at(idx)) {
                nbValueChanged++;
            }
        }
        ASSERT_EQ(nbValueChanged, 1) << "Mutating a node should change a single value";
    }

    vect.pop_back();
    Node::GPNode node(vect);
    ASSERT_THROW(mutation.mutateNode(node, nodeTemplate, rng), std::runtime_error) << "Mutating should fail with different sizes";
}

TEST_F(MutationTest, mutateIndividual)
{
    //ASSERT_FALSE(true) << "TODO";
}
