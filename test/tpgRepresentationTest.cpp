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

#include "evolution/individual.h"

#include "instructions/lambdaInstruction.h"


class TPGRepresentationTest : public ::testing::Test
{
  protected:
    Instructions::Set set;
    Data::PrimitiveTypeArray<double>* inputSource;

    Evolution::Representation* memberRepresentation;
    Evolution::Population* memberPopulation;

    Evolution::Population* tpgPopulation;

    virtual void SetUp()
    {   
        auto add = [](double a, double b) -> double { return a + b; };
        auto minus = [](double a, double b) -> double { return a - b; };
        auto times = [](double a, double b) -> double { return a * b; };
        auto div = [](double a, double b) -> double { return a / b; };
        
        set.add(*(new Instructions::LambdaInstruction<double, double>(add)));
        set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));
        set.add(*(new Instructions::LambdaInstruction<double, double>(times)));
        set.add(*(new Instructions::LambdaInstruction<double, double>(div)));

        inputSource = new Data::PrimitiveTypeArray<double>(4);

        memberRepresentation = new Representations::LGPRepresentation(set, 8, 1, 10);
        memberRepresentation->setInputDimensions({*inputSource});
        memberPopulation = new Evolution::Population();
        tpgPopulation = new Evolution::Population();
        for(size_t idx = 0; idx < 100; idx++) {
            memberPopulation->addIndividual();
            tpgPopulation->addIndividual();
        }
    }

    virtual void TearDown()
    {
        delete inputSource;
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
        delete (&set.getInstruction(2));
        delete (&set.getInstruction(3));
        delete memberRepresentation;
        delete memberPopulation;
    }
};

TEST_F(TPGRepresentationTest, Constructor)
{
    Representations::TPGRepresentation* representation;

    ASSERT_NO_THROW(representation = new Representations::TPGRepresentation(*memberRepresentation, *memberPopulation, 2, 10)) << "Constructor of Representation failed.";

    ASSERT_NO_THROW(representation->cloneUniquePtr()) << "Cloning should not fail";

    ASSERT_NO_THROW(delete representation) << "Destructor of Representation failed.";
}


TEST_F(TPGRepresentationTest, Cloning)
{
    Representations::TPGRepresentation representation(*memberRepresentation, *memberPopulation, 2, 10);

    std::unique_ptr<Evolution::Representation> clone1;
    ASSERT_NO_THROW(clone1 = std::move(representation.cloneUniquePtr())) << "Cloning should not fail";

    representation.setTangledPopulation(*tpgPopulation);
    ASSERT_FALSE(clone1->hasTangledPopulation()) << "Clone 1 should not have a tangled population";

    std::unique_ptr<Evolution::Representation> clone2;
    ASSERT_NO_THROW(clone2 = std::move(representation.cloneUniquePtr())) << "Cloning should not fail";
    ASSERT_TRUE(clone2->hasTangledPopulation()) << "Clone 2 should have a tangled population";
    ASSERT_TRUE(clone2->getTangledPopulation().value().get() == representation.getTangledPopulation().value().get()) << "Tangled population should be the same after cloning";
}

TEST_F(TPGRepresentationTest, setInputDimensions)
{
    // Todo later
}

TEST_F(TPGRepresentationTest, getGenotypeTemplate)
{
    Representations::TPGRepresentation representation(*memberRepresentation, *memberPopulation, 5, 10);
    std::unique_ptr<const Node::GenotypeTemplate> genotypeTemplate;

    ASSERT_THROW(representation.getGenotypeTemplate(), std::runtime_error) << "Should throw with unset input sources";
    representation.setInputDimensions({*inputSource});
    representation.setTangled(false);
    ASSERT_THROW(representation.getGenotypeTemplate(), std::runtime_error) << "Should throw with not define as tangled";
    representation.setTangled(true);
    ASSERT_THROW(representation.getGenotypeTemplate(), std::runtime_error) << "Should throw with no tangled population set";
    representation.setTangledPopulation(*tpgPopulation);

    ASSERT_NO_THROW(genotypeTemplate = std::move(representation.getGenotypeTemplate())) << "Getting genotypeTemplate should not have fail";
    
    ASSERT_EQ(genotypeTemplate->size(), 1) << "Template should have a single nodeTemplate";
    ASSERT_EQ(genotypeTemplate->getRangeAt(0).first, 5) << "Minimal range of the template should be 5";
    ASSERT_EQ(genotypeTemplate->getRangeAt(0).second, 10) << "Maximal range of the template should be 10";

    std::shared_ptr<const Node::NodeTemplate> nodeTemplate = genotypeTemplate->getNodeTemplateAt(0);
    ASSERT_EQ(nodeTemplate->size(), 2) << "Node template should be of size 2";

    /// CHECK MEMBER TEMPLATE
    const std::shared_ptr<const Node::NodeValueTemplate>& nodeValueTemplate0 = nodeTemplate->getValueTemplateAt(0);
    ASSERT_EQ(nodeValueTemplate0->size(), 1) << "Template should be of size 1";

    ASSERT_TRUE(std::holds_alternative<std::vector<std::weak_ptr<const Evolution::Individual>>>(*nodeValueTemplate0->getconfigurationAt(0))) << "Configuration should be a vector of weak ptrs";
    const std::vector<std::weak_ptr<const Evolution::Individual>>& values = std::get<std::vector<std::weak_ptr<const Evolution::Individual>>>(*nodeValueTemplate0->getconfigurationAt(0));
    
    ASSERT_EQ(values.size(), memberPopulation->size()) << "Value vector size should be the same as the member population";
    std::set<std::reference_wrapper<const Evolution::Individual>> memberIndiv = memberPopulation->getIndividuals();
    auto itMember = memberIndiv.begin();
    for(size_t idx = 0; idx < values.size(); idx++) {
        ASSERT_TRUE(*values.at(idx).lock() == *itMember) << "Order of individuals should be conserved";
        itMember++;
    }

    /// CHECK ACTION/TANGLED TEMPLATE
    const std::shared_ptr<const Node::NodeValueTemplate>& nodeValueTemplate1 = nodeTemplate->getValueTemplateAt(1);
    ASSERT_EQ(nodeValueTemplate1->size(), 2) << "Template should be of size 2";

    /// CHECK ACTION CONFIG
    ASSERT_TRUE(std::holds_alternative<Node::NodeValueRange>(*nodeValueTemplate1->getconfigurationAt(0))) << "Configuration should be a valueRange";
    const Node::NodeValueRange& range = std::get<Node::NodeValueRange>(*nodeValueTemplate1->getconfigurationAt(0));
    bool isSize_tPair = std::holds_alternative<std::pair<size_t, size_t>>(range);
    std::pair<size_t, size_t> pairRange = std::get<std::pair<size_t, size_t>>(range);
    ASSERT_EQ(pairRange.first, 0) << "Lower range should always be 0";
    ASSERT_EQ(pairRange.second, 3) << "Expected upper range is incorrect";

    /// CHECK TANGLED CONFIG
    ASSERT_TRUE(std::holds_alternative<std::vector<std::weak_ptr<const Evolution::Individual>>>(*nodeValueTemplate1->getconfigurationAt(1))) << "Configuration should be a vector of weak ptr";
    const std::vector<std::weak_ptr<const Evolution::Individual>>& valuesT = std::get<std::vector<std::weak_ptr<const Evolution::Individual>>>(*nodeValueTemplate1->getconfigurationAt(1));
    ASSERT_EQ(valuesT.size(), tpgPopulation->size()) << "Value vector size should be the same as the tpg population";
    std::set<std::reference_wrapper<const Evolution::Individual>> tpgIndivs = tpgPopulation->getIndividuals();
    auto itTpg = tpgIndivs.begin();
    for(size_t idx = 0; idx < valuesT.size(); idx++) {
        ASSERT_TRUE(*valuesT.at(idx).lock() == *itTpg) << "Order of individuals should be conserved";
        itTpg++;
    }
}

TEST_F(TPGRepresentationTest, isValid)
{
    Representations::TPGRepresentation representation(*memberRepresentation, *memberPopulation, 5, 10);
    Evolution::Individual indiv;
    Evolution::Genotype& genotype = indiv.getMutableGenotype();
    Node::NodeGroup& group = genotype.addNodeGroup();

    
    // Create member individuals
    const std::shared_ptr<const Evolution::Individual>& badRepMemberPtr = memberPopulation->getIndividualPtrs().at(0).lock();
    const std::shared_ptr<const Evolution::Individual>& goodMemberPtr = memberPopulation->getIndividualPtrs().at(1).lock();

    Evolution::Individual& goodMemberMut = memberPopulation->getMutableIndividual(*goodMemberPtr);
    Evolution::Genotype& goodMemberGenotype = goodMemberMut.getMutableGenotype();
    Node::NodeGroup& goodMemberGroup = goodMemberGenotype.addNodeGroup();
    for(size_t i = 0; i < 8; i++) {
        goodMemberGroup.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{0, 0, 0, 0, 0, 0}));
    }
    
    ASSERT_THROW(representation.isValid(indiv), std::runtime_error) << "Should throw with unset input sources";
    representation.setInputDimensions({*inputSource});

    representation.setTangled(false);
    ASSERT_THROW(representation.isValid(indiv), std::runtime_error) << "Should throw with not define as tangled";
    representation.setTangled(true);
    ASSERT_THROW(representation.isValid(indiv), std::runtime_error) << "Should throw with no tangled population set";
    representation.setTangledPopulation(*tpgPopulation);

    ASSERT_NO_THROW(representation.isValid(indiv)) << "Should not throw anymore";

    for(size_t i = 0; i < 4; i++) {
        group.addNode(std::make_unique<Node::GPNode>(std::vector<Node::NodeValue>{goodMemberPtr, size_t(0)}));
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
    inputSource->setDataAt(typeid(double), 0, 1.0);
    inputSource->setDataAt(typeid(double), 1, 1.5);
    inputSource->setDataAt(typeid(double), 2, 2.0);
    inputSource->setDataAt(typeid(double), 3, -1.0);
    std::vector<std::reference_wrapper<const Data::DataHandler>> inputSources{*inputSource};

    std::set<std::reference_wrapper<const Evolution::Individual>> memberPop = memberPopulation->getIndividuals();
    auto itMember = memberPop.begin();

    // Fill lgp members.
    Evolution::Individual& member0 = memberPopulation->getMutableIndividual(*itMember);
    Evolution::Genotype& memberGenotype0 = member0.getMutableGenotype();
    Node::NodeGroup& memberGroup0 = memberGenotype0.addNodeGroup();
    
    memberGroup0.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{1, 2, 1, 5, 1, 2}));// R[1] = S[1] * S[2] = 3.0
    memberGroup0.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{0, 0, 0, 3, 1, 0}));// R[0] = R[3] + S[0] = 1.0
    ASSERT_TRUE(memberRepresentation->isValid(member0)) << "Member should be equal";

    itMember++;
    Evolution::Individual& member1 = memberPopulation->getMutableIndividual(*itMember);
    Evolution::Genotype& memberGenotype1 = member1.getMutableGenotype();
    Node::NodeGroup& memberGroup1 = memberGenotype1.addNodeGroup();
    
    memberGroup1.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{0, 2, 1, 0, 1, 0}));// R[0] = S[0] * S[0] = 1.0
    memberGroup1.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{0, 1, 0, 0, 1, 3}));// R[0] = R[0] - S[3] = 2.0
    ASSERT_TRUE(memberRepresentation->isValid(member1)) << "Member should be equal";


    itMember++;
    Evolution::Individual& member2 = memberPopulation->getMutableIndividual(*itMember);
    Evolution::Genotype& memberGenotype2 = member2.getMutableGenotype();
    Node::NodeGroup& memberGroup2 = memberGenotype2.addNodeGroup();
    
    memberGroup2.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{4, 1, 1, 2, 1, 0}));// R[4] = S[2] - S[0] = 1.0
    memberGroup2.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{0, 0, 0, 4, 1, 3}));// R[0] = R[4] + S[3] = 0.0
    ASSERT_TRUE(memberRepresentation->isValid(member2)) << "Member should be equal";


    Representations::TPGRepresentation representation(*memberRepresentation, *memberPopulation, 2, 10);
    representation.setTangledPopulation(*tpgPopulation);
    representation.setInputDimensions(inputSources);

    // Tangled Individual
    std::shared_ptr<Evolution::Individual> tangledIndiv = std::make_shared<Evolution::Individual>();
    Evolution::Genotype& tangledGenotype = tangledIndiv->getMutableGenotype();
    Node::NodeGroup& tangledGroup = tangledGenotype.addNodeGroup();
    
    tangledGroup.addNode(std::make_unique<Node::GPNode>(std::vector<Node::NodeValue>{memberPopulation->getIndividualPtrs().at(0).lock(), size_t(0)}));
    tangledGroup.addNode(std::make_unique<Node::GPNode>(std::vector<Node::NodeValue>{memberPopulation->getIndividualPtrs().at(1).lock(), size_t(1)}));
    tangledGroup.addNode(std::make_unique<Node::GPNode>(std::vector<Node::NodeValue>{memberPopulation->getIndividualPtrs().at(2).lock(), size_t(2)}));

    Evolution::Individual indiv;
    Evolution::Genotype& genotype = indiv.getMutableGenotype();
    Node::NodeGroup& group = genotype.addNodeGroup();
    
    group.addNode(std::make_unique<Node::GPNode>(std::vector<Node::NodeValue>{memberPopulation->getIndividualPtrs().at(1).lock(), tangledIndiv}));
    group.addNode(std::make_unique<Node::GPNode>(std::vector<Node::NodeValue>{memberPopulation->getIndividualPtrs().at(2).lock(), size_t(0)}));
    group.addNode(std::make_unique<Node::GPNode>(std::vector<Node::NodeValue>{memberPopulation->getIndividualPtrs().at(0).lock(), size_t(2)}));

    ASSERT_TRUE(representation.isValid(indiv)) << "Individual should be valid";

    double output;
    ASSERT_NO_THROW(output = representation.executeIndividual(*tangledIndiv, inputSources).at(0)) << "Execution of individual failed.";
    ASSERT_EQ(output, 1.0) << "Value is not correct.";
    ASSERT_NO_THROW(output = representation.executeIndividual(indiv, inputSources).at(0)) << "Execution of individual failed.";
    ASSERT_EQ(output, 1.0) << "Value is not correct.";
}