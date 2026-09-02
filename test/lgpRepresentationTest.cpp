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
#include <cmath>

#include "representations/lgpRepresentation.h"

#include "instructions/lambdaInstruction.h"

class LGPRepresentationTest : public ::testing::Test
{
  protected:
    Instructions::Set set;
    Data::DataRequirement inputType;
    Data::DataRequirement outputType;

    virtual void SetUp()
    {   
        auto add = [](double a, double b) -> double { return a + b; };
        auto minus = [](double a, double b) -> double { return a - b; };
        auto times = [](double a, double b) -> double { return a * b; };
        auto div = [](double a, double b) -> double { return a / b; };
        
        set.add(*(new Instructions::LambdaInstruction<double, double, double>(add)));
        set.add(*(new Instructions::LambdaInstruction<double, double, double>(minus)));
        set.add(*(new Instructions::LambdaInstruction<double, double, double>(times)));
        set.add(*(new Instructions::LambdaInstruction<double, double, double>(div)));

        inputType = Data::DataRequirement::array1d<double>(4);
        outputType = Data::DataRequirement::scalar<double>();
    }

    virtual void TearDown()
    {
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
        delete (&set.getInstruction(2));
        delete (&set.getInstruction(3));
    }
};

TEST_F(LGPRepresentationTest, Constructor)
{
    Representations::LGPRepresentation* representation;

    ASSERT_NO_THROW(representation = new Representations::LGPRepresentation(set, 8, 5, 10)) << "Constructor of Representation failed.";

    ASSERT_NO_THROW(representation->cloneUniquePtr()) << "Cloning should not fail";

    ASSERT_NO_THROW(delete representation) << "Destructor of Representation failed.";
}

TEST_F(LGPRepresentationTest, getGenotypeTemplate)
{
    Representations::LGPRepresentation representation(set, 8, 5, 10);
    std::unique_ptr<const Node::GenotypeTemplate> genotypeTemplate;

    ASSERT_THROW(representation.getGenotypeTemplate(), std::runtime_error) << "Should throw with unset input sources";
    representation.setDimensions({inputType}, outputType);
    
    ASSERT_NO_THROW(genotypeTemplate = std::move(representation.getGenotypeTemplate())) << "Getting genotypeTemplate should not have fail";
    
    ASSERT_EQ(genotypeTemplate->size(), 1) << "Template should have a single nodeTemplate";
    ASSERT_EQ(genotypeTemplate->getRangeAt(0).first, 5) << "Minimal range of the template should be 5";
    ASSERT_EQ(genotypeTemplate->getRangeAt(0).second, 10) << "Maximal range of the template should be 10";

    std::shared_ptr<const Node::NodeTemplate> nodeTemplate = genotypeTemplate->getNodeTemplateAt(0);
    ASSERT_EQ(nodeTemplate->size(), 6) << "Node template should be of size 6 with current input sources";
    std::vector<size_t> expectedRanges{8, 4, 2, 8, 2, 8};

    for(size_t idx = 0; idx < nodeTemplate->size(); idx++) {
        const std::shared_ptr<const Node::NodeValueTemplate>& nodeValueTemplate = nodeTemplate->getValueTemplateAt(idx);
        ASSERT_EQ(nodeValueTemplate->size(), 1) << "Template should be of size 1";

        ASSERT_TRUE(std::holds_alternative<Node::NodeValueRange>(*nodeValueTemplate->getconfigurationAt(0))) << "Configuration should be a valueRange";
        const Node::NodeValueRange& range = std::get<Node::NodeValueRange>(*nodeValueTemplate->getconfigurationAt(0));
        bool isSize_tPair = std::holds_alternative<std::pair<size_t, size_t>>(range);
        ASSERT_TRUE(isSize_tPair) << "range should be a pair of size_t";

        std::pair<size_t, size_t> pairRange = std::get<std::pair<size_t, size_t>>(range);
        ASSERT_EQ(pairRange.first, 0) << "Lower range should always be 0";
        ASSERT_EQ(pairRange.second, expectedRanges.at(idx)) << "Expected upper range is incorrect";
    }
}

TEST_F(LGPRepresentationTest, isValid)
{
    Representations::LGPRepresentation representation(set, 8, 5, 10);
    Evolution::Individual indiv;

    Evolution::Genotype& genotype = indiv.getMutableGenotype();
    genotype.addNodeGroup();
    Node::NodeGroup& group = genotype.getMutableNodeGroup(0);

    ASSERT_THROW(representation.isValid(indiv), std::runtime_error) << "Should throw with unset input sources";
    representation.setDimensions({inputType}, outputType);

    for(size_t i = 0; i < 4; i++) {
        group.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{0, 0, 0, 0, 0, 0}));
    }

    ASSERT_FALSE(representation.isValid(indiv)) << "Individual should not be valid with 4 nodes";

    group.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{7, 3, 1, 7, 1, 7}));
    ASSERT_TRUE(representation.isValid(indiv)) << "Individual should be valid with 5 nodes";

    group.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{8, 3, 1, 7, 1, 7}));
    ASSERT_FALSE(representation.isValid(indiv)) << "Individual should not be valid with wrong node";
    group.removeNode(indiv.getSize() - 1);

    group.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{8, 3, 1, 7, 1}));
    ASSERT_FALSE(representation.isValid(indiv)) << "Individual should not be valid with wrong node";
    group.removeNode(indiv.getSize() - 1);
    
    group.addNode(std::make_unique<Node::GPNode>(std::vector<double>{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}));
    ASSERT_FALSE(representation.isValid(indiv)) << "Individual should not be valid with wrong node";
    group.removeNode(indiv.getSize() - 1);


    for(size_t i = 0; i < 6; i++) {
        group.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{0, 0, 0, 0, 0, 0}));
    }

    ASSERT_EQ(indiv.getSize(), 11) << "Individual size should now be 11";
    ASSERT_FALSE(representation.isValid(indiv)) << "Individual should not be valid with 11 nodes";
}


TEST_F(LGPRepresentationTest, executeIndividual)
{
    Data::DataValue inputSource = Data::DataValue::array1d<double[4]>({1.0, 1.5, 2.0, -1.0});

    Representations::LGPRepresentation representation(set, 8, 5, 10);
    representation.setDimensions({inputType}, outputType);

    Evolution::Individual indiv;
    Evolution::Genotype& genotype = indiv.getMutableGenotype();
    genotype.addNodeGroup();
    Node::NodeGroup& group = genotype.getMutableNodeGroup(0);
    
    group.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{1, 2, 1, 5, 1, 2}));// R[1] = S[1] * S[2] = 3.0
    group.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{2, 0, 0, 3, 1, 0}));// R[2] = R[3] + S[0] = 1.0
    group.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{2, 3, 0, 2, 0, 2}));// R[2] = R[2] / R[2] = 1.0 / 1.0 = 1.0
    group.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{0, 1, 1, 2, 1, 1}));// R[0] = S[2] - S[1] = 0.5
    group.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{0, 0, 0, 0, 0, 2}));// R[0] = R[0] - R[2] = 0.5 + 1 = 1.5

    ASSERT_TRUE(representation.isValid(indiv)) << "Individual should be valid";

    ASSERT_NO_THROW(representation.executeIndividual(indiv, {inputSource.view()})) << "Execution of individual failed.";
    Data::DataValue output = representation.executeIndividual(indiv, {inputSource.view()});
    ASSERT_EQ(output.getScalar<double>(), 1.5) << "Value is not correct.";

    // R[0] = R[0] + R[0] = -1, but set as intron
    group.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{0, 0, 0, 0, 0, 0}, true));
    ASSERT_NO_THROW(output = representation.executeIndividual(indiv, {inputSource.view()})) << "Execution of individual failed.";
    ASSERT_EQ(output.getScalar<double>(), 1.5) << "Value is not correct.";
}