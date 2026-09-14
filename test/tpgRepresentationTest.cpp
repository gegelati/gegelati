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

#include "representations/tpgRepresentation.h"
#include "representations/lgpRepresentation.h"
#include "dimensions/numericRange.h"

#include "evolution/individual.h"

#include "instructions/lambdaInstruction.h"

class TPGRepresentationTest : public ::testing::Test
{
  protected:
    Instructions::Set set;

    Evolution::Representation* memberRepresentation;

    Dimensions::Requirement inputType;

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

        inputType = Dimensions::Requirement::array1d<double>(4);

        memberRepresentation = new Representations::LGPRepresentation({inputType}, 1, set, 8, 1, 10);
    }

    virtual void TearDown()
    {
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
        delete (&set.getInstruction(2));
        delete (&set.getInstruction(3));
        delete memberRepresentation;
    }
};

TEST_F(TPGRepresentationTest, Constructor)
{
    Representations::TPGRepresentation* representation;
    Representations::TPGRepresentation* representation2;

    ASSERT_NO_THROW(representation = new Representations::TPGRepresentation({inputType}, 3, 2, 10)) << "Constructor of Representation failed.";

    ASSERT_NO_THROW(representation->cloneUniquePtr()) << "Cloning should not fail";

    ASSERT_NO_THROW(delete representation) << "Destructor of Representation failed.";
}

TEST_F(TPGRepresentationTest, Cloning)
{
    Representations::TPGRepresentation representation({inputType}, 3, 2, 10);

    std::unique_ptr<Evolution::Representation> clone1;
    ASSERT_NO_THROW(clone1 = std::move(representation.cloneUniquePtr())) << "Cloning should not fail";
}

TEST_F(TPGRepresentationTest, setInputDimensions)
{
    // Todo later
}

TEST_F(TPGRepresentationTest, getSetGenotypeTemplate)
{
    Representations::TPGRepresentation representation({inputType}, 3, 5, 10);
    std::unique_ptr<Node::GenotypeTemplate> genotypeTemplate;
    RNG::RNG rng;

    std::vector<std::shared_ptr<const Evolution::Individual>> members{
        std::make_shared<const Evolution::Individual>(*memberRepresentation),
        std::make_shared<const Evolution::Individual>(*memberRepresentation),
        std::make_shared<const Evolution::Individual>(*memberRepresentation)
    };
    std::vector<std::shared_ptr<const Evolution::Individual>> tangled{
        std::make_shared<const Evolution::Individual>(representation),
        std::make_shared<const Evolution::Individual>(representation)
    };
    representation.setAvailableMembers(members);
    representation.setAvailableTangledIndiv(tangled);
    
    ASSERT_NO_THROW(genotypeTemplate = std::move(representation.getGenotypeTemplate())) << "Getting genotypeTemplate should not have fail";
    
    ASSERT_EQ(genotypeTemplate->size(), 1) << "Template should have a single nodeTemplate";
    ASSERT_EQ(genotypeTemplate->getRangeAt(0).first, 5) << "Minimal range of the template should be 5";
    ASSERT_EQ(genotypeTemplate->getRangeAt(0).second, 10) << "Maximal range of the template should be 10";

    Node::NodeTemplate& nodeTemplate = genotypeTemplate->getNodeTemplateAt(0);
    ASSERT_EQ(nodeTemplate.size(), 2) << "Node template should be of size 2";

    /// CHECK MEMBER TEMPLATE
    const Dimensions::Constraint& memberConstraint = nodeTemplate.getConstraintAt(0);
    ASSERT_TRUE(memberConstraint == Dimensions::NumericRange<double>::unbounded()) << "Constraint should double unbounded";

    auto* memberGenerator = 
        dynamic_cast<Dimensions::ListUniformGenerator<std::shared_ptr<const Evolution::Individual>>*>(&nodeTemplate.getGeneratorAt(0));
    ASSERT_TRUE(memberGenerator != nullptr) << "Generator should be a listUniformGenerator of shared individuals";
    for (size_t idx = 0; idx < 100; idx++) {
        std::shared_ptr<const Evolution::Individual> sampled = memberGenerator->sample(rng).getScalar<std::shared_ptr<const Evolution::Individual>>();
        ASSERT_TRUE((*sampled == *members.at(0)) ||
                    (*sampled == *members.at(1)) ||
                    (*sampled == *members.at(2))) << "Representation of member should be good"; 
    }


    /// CHECK ACTION/TANGLED TEMPLATE
    const Dimensions::Constraint& destinationConstraint = nodeTemplate.getConstraintAt(1);
    ASSERT_TRUE(destinationConstraint == Dimensions::NumericRange<size_t>::between(0, 2)) << "Constraint should size_t in 0/nbActions";

    /// CHECK ACTION CONFIG
    auto* destinationGenerator = 
        dynamic_cast<Dimensions::MultiGenerator*>(&nodeTemplate.getGeneratorAt(1));
    ASSERT_TRUE(destinationGenerator != nullptr) << "Generator should be a multiGenerator";
    for (size_t idx = 0; idx < 100; idx++) {
        Data::DataValue sampled = destinationGenerator->sample(rng);
        if(sampled.getElementType() == typeid(size_t)) {
            ASSERT_LT(sampled.getScalar<size_t>(), 3) << "Value should be stricly lower than 3";
        } else if (sampled.getElementType() == typeid(std::shared_ptr<const Evolution::Individual>)) {
            
            std::shared_ptr<const Evolution::Individual> sampledIndiv = sampled.getScalar<std::shared_ptr<const Evolution::Individual>>();
            ASSERT_TRUE((*sampledIndiv == *tangled.at(0)) ||
                        (*sampledIndiv == *tangled.at(1))) << "Representation of member should be good"; 
        } else {
            ASSERT_FALSE(true) << "Type is wrong";
        }
    }

}

/*
TEST_F(TPGRepresentationTest, isValid)
{
    Representations::TPGRepresentation representation({inputType}, 3, 5, 10);
    Evolution::Genotype genotype;
    Node::NodeGroup& group = genotype.addNodeGroup();

    
    // Create member individuals
    std::vector<std::shared_ptr<Evolution::Individual>> members{
        std::make_shared<const Evolution::Individual>(*memberRepresentation),
    };
    const std::shared_ptr<const Evolution::Individual>& badRepMemberPtr = members.at(0).lock();
    const std::shared_ptr<const Evolution::Individual>& goodMemberPtr = memberPopulation->getIndividualPtrs().at(1).lock();

    Evolution::Individual& goodMemberMut = memberPopulation->getMutableIndividual(*goodMemberPtr);
    Evolution::Genotype& goodMemberGenotype = goodMemberMut.getMutableGenotype();
    Node::NodeGroup& goodMemberGroup = goodMemberGenotype.addNodeGroup();
    for(size_t i = 0; i < 8; i++) {
        goodMemberGroup.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{0, 0, 0, 0, 0, 0}));
    }

    representation.setTangled(false);
    ASSERT_THROW(representation.isValid(indiv), std::runtime_error) << "Should throw with not define as tangled";
    representation.setTangled(true);
    ASSERT_THROW(representation.isValid(indiv), std::runtime_error) << "Should throw with no tangled population set";
    representation.setTangledPopulation(*tpgPopulation);

    ASSERT_NO_THROW(representation.isValid(indiv)) << "Should not throw anymore";

    for(size_t i = 0; i < 4; i++) {
        group.addNode(std::make_unique<Node::GPNode>(std::vector<Data::DataValue>{Data::DataValue::scalar(goodMemberPtr), size_t(0)}));
    }

    ASSERT_FALSE(representation.isValid(indiv)) << "Individual should not be valid with 4 nodes";

    group.addNode(std::make_unique<Node::GPNode>(std::vector<Node::NodeValue>{goodMemberPtr, size_t(0)}));
    ASSERT_TRUE(representation.isValid(indiv)) << "Individual should be valid with 5 nodes";

    group.addNode(std::make_unique<Node::GPNode>(std::vector<Node::NodeValue>{goodMemberPtr, size_t(6)}));
    ASSERT_FALSE(representation.isValid(indiv)) << "Individual should not be valid with wrong action node";
    group.removeNode(indiv.getSize() - 1);

    group.addNode(std::make_unique<Node::GPNode>(std::vector<Node::NodeValue>{badRepMemberPtr, size_t(6)}));
    ASSERT_FALSE(representation.isValid(indiv)) << "Individual should not be valid with wrong action node";
    group.removeNode(indiv.getSize() - 1);

    group.addNode(std::make_unique<Node::GPNode>(std::vector<Node::NodeValue>{size_t(6), size_t(6)}));
    ASSERT_FALSE(representation.isValid(indiv)) << "Individual should not be valid with wrong action node";
    group.removeNode(indiv.getSize() - 1);

    group.addNode(std::make_unique<Node::GPNode>(std::vector<Node::NodeValue>{goodMemberPtr, 0.0}));
    ASSERT_FALSE(representation.isValid(indiv)) << "Individual should not be valid with wrong action node";
    group.removeNode(indiv.getSize() - 1);

    
    group.addNode(std::make_unique<Node::GPNode>(std::vector<Node::NodeValue>{goodMemberPtr}));
    ASSERT_FALSE(representation.isValid(indiv)) << "Individual should not be valid with wrong action node";
    group.removeNode(indiv.getSize() - 1);

    for(size_t i = 0; i < 6; i++) {
        group.addNode(std::make_unique<Node::GPNode>(std::vector<Node::NodeValue>{goodMemberPtr, size_t(0)}));
    }

    ASSERT_EQ(indiv.getSize(), 11) << "Individual size should now be 11";
    ASSERT_FALSE(representation.isValid(indiv)) << "Individual should not be valid with 11 nodes";
    for(size_t i = 0; i < 6; i++) {
        group.removeNode(indiv.getSize() - 1);
    }

    // Testing tangled connections.
    std::shared_ptr<Evolution::Individual> tangledIndiv = std::make_shared<Evolution::Individual>();
    Evolution::Genotype& tangledGenotype = tangledIndiv->getMutableGenotype();
    Node::NodeGroup& tangledGroup = tangledGenotype.addNodeGroup();
    for(size_t i = 0; i < 6; i++) {
        tangledGroup.addNode(std::make_unique<Node::GPNode>(std::vector<Node::NodeValue>{goodMemberPtr, size_t(0)}));
    }
    ASSERT_TRUE(representation.isValid(*tangledIndiv)) << "Individual should be valid";

    group.addNode(std::make_unique<Node::GPNode>(std::vector<Node::NodeValue>{goodMemberPtr, tangledIndiv}));
    ASSERT_TRUE(representation.isValid(indiv)) << "Individual should be valid with valid tangled individual";
    group.removeNode(indiv.getSize() - 1);

    group.addNode(std::make_unique<Node::GPNode>(std::vector<Node::NodeValue>{goodMemberPtr, goodMemberPtr}));
    ASSERT_FALSE(representation.isValid(indiv)) << "Individual should not be valid with not valid tangled individual";
    group.removeNode(indiv.getSize() - 1);

    tangledGroup.addNode(std::make_unique<Node::GPNode>(std::vector<Node::NodeValue>{goodMemberPtr, tangledIndiv}));
    ASSERT_FALSE(representation.isValid(*tangledIndiv)) << "Individual should not be valid with itself has tangled individual";
}

TEST_F(TPGRepresentationTest, executeIndividual)
{
    Data::DataValue inputSource = Data::DataValue::array1d<double[4]>({1.0, 1.5, 2.0, -1.0});
    Representations::TPGRepresentation representation({inputType}, 3, 2, 10);


    // create lgp members.
    std::shared_ptr<Evolution::Individual> member0 = std::make_shared<Evolution::Individual>(*memberRepresentation);
    Evolution::Genotype& memberGenotype0 = member0->getMutableGenotype();
    Node::NodeGroup& memberGroup0 = memberGenotype0.addNodeGroup();
    
    memberGroup0.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{1, 2, 1, 5, 1, 2}));// R[1] = S[1] * S[2] = 3.0
    memberGroup0.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{0, 0, 0, 3, 1, 0}));// R[0] = R[3] + S[0] = 1.0
    ASSERT_TRUE(member0->isValid()) << "Member should be valid";

    std::shared_ptr<Evolution::Individual> member1 = std::make_shared<Evolution::Individual>(*memberRepresentation);
    Evolution::Genotype& memberGenotype1 = member1->getMutableGenotype();
    Node::NodeGroup& memberGroup1 = memberGenotype1.addNodeGroup();
    
    memberGroup1.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{0, 2, 1, 0, 1, 0}));// R[0] = S[0] * S[0] = 1.0
    memberGroup1.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{0, 1, 0, 0, 1, 3}));// R[0] = R[0] - S[3] = 2.0
    ASSERT_TRUE(member1->isValid()) << "Member should be valid";


    std::shared_ptr<Evolution::Individual> member2 = std::make_shared<Evolution::Individual>(*memberRepresentation);
    Evolution::Genotype& memberGenotype2 = member2->getMutableGenotype();
    Node::NodeGroup& memberGroup2 = memberGenotype2.addNodeGroup();
    
    memberGroup2.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{4, 1, 1, 2, 1, 0}));// R[4] = S[2] - S[0] = 1.0
    memberGroup2.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{0, 0, 0, 4, 1, 3}));// R[0] = R[4] + S[3] = 0.0
    ASSERT_TRUE(member2->isValid()) << "Member should be valid";

    std::vector<std::shared_ptr<const Evolution::Individual>> memberPop{member0, member1, member2};



    // Tangled Individual
    std::shared_ptr<Evolution::Individual> tangledIndiv = std::make_shared<Evolution::Individual>(representation);
    Evolution::Genotype& tangledGenotype = tangledIndiv->getMutableGenotype();
    Node::NodeGroup& tangledGroup = tangledGenotype.addNodeGroup();
    
    for(size_t i = 0; i < 3; i++) {
        std::unique_ptr<Node::GPNode> node = std::make_unique<Node::GPNode>();
        node->addValue(Data::DataValue::scalar(memberPop.at(i)));
        node->addValue(Data::DataValue::scalar<size_t>(i));
        tangledGroup.addNode(std::move(node));
    }

    Evolution::Individual indiv(representation);
    Evolution::Genotype& genotype = indiv.getMutableGenotype();
    Node::NodeGroup& group = genotype.addNodeGroup();
    
    std::unique_ptr<Node::GPNode> node0 = std::make_unique<Node::GPNode>();
    node0->addValue(Data::DataValue::scalar(memberPop.at(1)));
    node0->addValue(Data::DataValue::scalar<std::shared_ptr<const Evolution::Individual>>(tangledIndiv));
    group.addNode(std::move(node0));
    
    std::unique_ptr<Node::GPNode> node1 = std::make_unique<Node::GPNode>();
    node1->addValue(Data::DataValue::scalar(memberPop.at(2)));
    node1->addValue(Data::DataValue::scalar<size_t>(0));
    group.addNode(std::move(node1));
    
    std::unique_ptr<Node::GPNode> node2 = std::make_unique<Node::GPNode>();
    node2->addValue(Data::DataValue::scalar(memberPop.at(0)));
    node2->addValue(Data::DataValue::scalar<size_t>(2));
    group.addNode(std::move(node2));

    
    representation.setAvailableMembers(memberPop);
    ASSERT_TRUE(tangledIndiv->isValid()) << "Individual should be valid";
    ASSERT_TRUE(indiv.isValid()) << "Individual should be valid";

    size_t output;
    ASSERT_NO_THROW(output = tangledIndiv->execute({inputSource.view()}).getScalar<size_t>()) << "Execution of individual failed.";
    ASSERT_EQ(output, 1) << "Value is not correct.";
    ASSERT_NO_THROW(output = indiv.execute({inputSource.view()}).getScalar<size_t>()) << "Execution of individual failed.";
    ASSERT_EQ(output, 1) << "Value is not correct.";

    std::cout<<representation.summary()<<std::endl;
}
*/