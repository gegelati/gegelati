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

#include "representations/TPG.h"
#include "representations/LGP.h"
#include "dimensions/numericRange.h"

#include "individual.h"

#include "instructions/lambdaInstruction.h"
#include "util/counterReset.h"

class TPGRepresentationTest : public ::testing::Test
{
  protected:
    Instructions::Set set;

    Representations::Representation* memberRepresentation;

    Dimensions::Requirement inputType;

    virtual void SetUp()
    {   
        CounterReset::counterReset();
        auto add = [](double a, double b) -> double { return a + b; };
        auto minus = [](double a, double b) -> double { return a - b; };
        auto times = [](double a, double b) -> double { return a * b; };
        auto div = [](double a, double b) -> double { return a / b; };
        
        set.add(*(new Instructions::LambdaInstruction<double, double, double>(add)));
        set.add(*(new Instructions::LambdaInstruction<double, double, double>(minus)));
        set.add(*(new Instructions::LambdaInstruction<double, double, double>(times)));
        set.add(*(new Instructions::LambdaInstruction<double, double, double>(div)));

        inputType = Dimensions::Requirement::array1d<double>(4);

        memberRepresentation = new Representations::LGP({inputType}, 1, set, 8, 1, 10);
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
    Representations::TPG* representation;
    Representations::TPG* representation2;

    ASSERT_NO_THROW(representation = new Representations::TPG({inputType}, 3, 2)) << "Constructor of Representation failed.";

    ASSERT_NO_THROW(representation->cloneUniquePtr()) << "Cloning should not fail";

    ASSERT_NO_THROW(delete representation) << "Destructor of Representation failed.";
}

TEST_F(TPGRepresentationTest, Cloning)
{
    Representations::TPG representation({inputType}, 3, 2, 10);

    std::unique_ptr<Representations::Representation> clone1;
    ASSERT_NO_THROW(clone1 = std::move(representation.cloneUniquePtr())) << "Cloning should not fail";
}


TEST_F(TPGRepresentationTest, getGenotypeConstraint)
{
    size_t nbActions = 3;
    Representations::TPG representation({inputType}, nbActions, 2, 10);
    const GraphBased::GenotypeConstraint& constraint = representation.getGenotypeConstraint();

    ASSERT_EQ(constraint.size(), 1) << "Size of constraint should be 1";
    ASSERT_EQ(constraint.getRangeAt(0).first, 2) << "Low range should be 2";
    ASSERT_EQ(constraint.getRangeAt(0).second, 10) << "Low range should be 10";

    const GraphBased::NodeConstraint& nodeConstraint = constraint.getNodeConstraintAt(0);
    ASSERT_EQ(nodeConstraint.size(), 2) << "Size should be 2";

    ASSERT_TRUE(nodeConstraint.getConstraintAt(0) == Dimensions::NumericRange<double>::unbounded()) << "Should be compatible";
    ASSERT_TRUE(nodeConstraint.getConstraintAt(1) == Dimensions::NumericRange<size_t>::between(0, nbActions - 1)) << "Should be compatible";
}


TEST_F(TPGRepresentationTest, getGenotypeGenerator)
{
    RNG::RNG rng;

    size_t nbActions = 3;
    Representations::TPG representation({inputType}, nbActions, 2, 10);
    std::unique_ptr<GraphBased::GenotypeGenerator> generator = representation.getGenotypeGenerator();

    ASSERT_EQ(generator->size(), 1) << "Size of constraint should be 1";
    ASSERT_EQ(generator->getRangeAt(0).first, 2) << "Low range should be 2";
    ASSERT_EQ(generator->getRangeAt(0).second, 10) << "Low range should be 10";

    GraphBased::NodeGenerator& nodeGen = generator->getNodeGeneratorAt(0);
    ASSERT_EQ(nodeGen.size(), 2) << "Size should be 2";

    ASSERT_THROW(nodeGen.getGeneratorAt(0).sample(rng), std::runtime_error) << "Should throw since no individual is set";
    for(size_t idx = 0; idx < 1000; idx++) {
        ASSERT_TRUE(nodeGen.getGeneratorAt(1).sample(rng).getScalar<size_t>() < nbActions);
    }

    
    std::vector<std::shared_ptr<const Individual>> members = {
        std::make_shared<Individual>(*memberRepresentation),
        std::make_shared<Individual>(*memberRepresentation),
        std::make_shared<Individual>(*memberRepresentation)
    };

    std::vector<std::shared_ptr<const Individual>> tangled = {
        std::make_shared<Individual>(representation),
        std::make_shared<Individual>(representation),
    };

    representation.setAvailableMembers(members);

    // Should update generator automatically
    representation.setAvailableTangledIndiv(tangled);

    std::unique_ptr<GraphBased::GenotypeGenerator> generatorNew = representation.getGenotypeGenerator();
    GraphBased::NodeGenerator& nodeGenNew = generatorNew->getNodeGeneratorAt(0);
    for(size_t idx = 0; idx < 1000; idx++) {
        std::shared_ptr<const Individual> member = nodeGenNew.getGeneratorAt(0).sample(rng).getScalar<std::shared_ptr<const Individual>>();
        ASSERT_TRUE(member->getIndividualID() < members.size()) << "ID should be 0, 1 or 2";

        Data::DataValue destination = nodeGenNew.getGeneratorAt(1).sample(rng);
        if(destination.getElementType() == typeid(size_t)) {
            ASSERT_TRUE(destination.getScalar<size_t>() < nbActions);
        } else {
            std::shared_ptr<const Individual> member = destination.getScalar<std::shared_ptr<const Individual>>();
            ASSERT_TRUE(member->getIndividualID() - members.size() < tangled.size()) << "ID should be 3 or 4";
        }
    }
}


TEST_F(TPGRepresentationTest, executeIndividual)
{
    Data::DataValue inputSource = Data::DataValue::array1d<double[4]>({1.0, 1.5, 2.0, -1.0});
    Representations::TPG representation({inputType}, 3, 2, 10);


    // create lgp members.
    std::shared_ptr<Individual> member0 = std::make_shared<Individual>(*memberRepresentation);
    std::unique_ptr<GraphBased::Genotype> memberGenotype0 = std::make_unique<GraphBased::Genotype>();
    std::unique_ptr<GraphBased::NodeGroup> memberGroup0 = std::make_unique<GraphBased::NodeGroup>();
    
    memberGroup0->addNode(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{1, 2, 1, 5, 1, 2}));// R[1] = S[1] * S[2] = 3.0
    memberGroup0->addNode(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0, 0, 0, 3, 1, 0}));// R[0] = R[3] + S[0] = 1.0
    memberGenotype0->addNodeGroup(std::move(memberGroup0));
    member0->setGenotype(std::move(memberGenotype0));
    ASSERT_TRUE(member0->isValid()) << "Member should be valid";

    std::shared_ptr<Individual> member1 = std::make_shared<Individual>(*memberRepresentation);
    std::unique_ptr<GraphBased::Genotype> memberGenotype1 = std::make_unique<GraphBased::Genotype>();
    std::unique_ptr<GraphBased::NodeGroup> memberGroup1 = std::make_unique<GraphBased::NodeGroup>();
    
    memberGroup1->addNode(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0, 2, 1, 0, 1, 0}));// R[0] = S[0] * S[0] = 1.0
    memberGroup1->addNode(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0, 1, 0, 0, 1, 3}));// R[0] = R[0] - S[3] = 2.0
    memberGenotype1->addNodeGroup(std::move(memberGroup1));
    member1->setGenotype(std::move(memberGenotype1));
    ASSERT_TRUE(member1->isValid()) << "Member should be valid";


    std::shared_ptr<Individual> member2 = std::make_shared<Individual>(*memberRepresentation);
    std::unique_ptr<GraphBased::Genotype> memberGenotype2 = std::make_unique<GraphBased::Genotype>();
    std::unique_ptr<GraphBased::NodeGroup> memberGroup2 = std::make_unique<GraphBased::NodeGroup>();
    
    memberGroup2->addNode(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{4, 1, 1, 2, 1, 0}));// R[4] = S[2] - S[0] = 1.0
    memberGroup2->addNode(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0, 0, 0, 4, 1, 3}));// R[0] = R[4] + S[3] = 0.0
    memberGenotype2->addNodeGroup(std::move(memberGroup2));
    member2->setGenotype(std::move(memberGenotype2));
    ASSERT_TRUE(member2->isValid()) << "Member should be valid";

    std::vector<std::shared_ptr<const Individual>> memberPop{member0, member1, member2};



    // Tangled Individual
    std::shared_ptr<Individual> tangledIndiv = std::make_shared<Individual>(representation);
    std::unique_ptr<GraphBased::Genotype> tangledGenotype = std::make_unique<GraphBased::Genotype>();
    std::unique_ptr<GraphBased::NodeGroup> tangledGroup = std::make_unique<GraphBased::NodeGroup>();
    
    for(size_t i = 0; i < 3; i++) {
        std::unique_ptr<GraphBased::GPNode> node = std::make_unique<GraphBased::GPNode>();
        node->addValue(Data::DataValue::scalar(memberPop.at(i)));
        node->addValue(Data::DataValue::scalar<size_t>(i));
        tangledGroup->addNode(std::move(node));
    }
    tangledGenotype->addNodeGroup(std::move(tangledGroup));
    tangledIndiv->setGenotype(std::move(tangledGenotype));
    ASSERT_TRUE(tangledIndiv->isValid()) << "tangledIndiv should be valid";

    GraphBased::Genotype genotype;
    std::unique_ptr<GraphBased::NodeGroup> group = std::make_unique<GraphBased::NodeGroup>();
    
    std::unique_ptr<GraphBased::GPNode> node0 = std::make_unique<GraphBased::GPNode>();
    node0->addValue(Data::DataValue::scalar(memberPop.at(1)));
    node0->addValue(Data::DataValue::scalar<std::shared_ptr<const Individual>>(tangledIndiv));
    group->addNode(std::move(node0));
    
    std::unique_ptr<GraphBased::GPNode> node1 = std::make_unique<GraphBased::GPNode>();
    node1->addValue(Data::DataValue::scalar(memberPop.at(2)));
    node1->addValue(Data::DataValue::scalar<size_t>(0));
    group->addNode(std::move(node1));
    
    std::unique_ptr<GraphBased::GPNode> node2 = std::make_unique<GraphBased::GPNode>();
    node2->addValue(Data::DataValue::scalar(memberPop.at(0)));
    node2->addValue(Data::DataValue::scalar<size_t>(2));
    group->addNode(std::move(node2));

    
    genotype.addNodeGroup(std::move(group));
    ASSERT_TRUE(representation.isValid(genotype)) << "Individual should be valid";

    size_t output;
    ASSERT_NO_THROW(output = tangledIndiv->execute({inputSource.view()}).getScalar<size_t>()) << "Execution of individual failed.";
    ASSERT_EQ(output, 1) << "Value is not correct.";
    ASSERT_NO_THROW(output = representation.execute(genotype, {inputSource.view()}).getScalar<size_t>()) << "Execution of individual failed.";
    ASSERT_EQ(output, 1) << "Value is not correct.";

    std::cout<<representation.summary()<<std::endl;

    std::unique_ptr<Representations::Representation> clone = representation.cloneUniquePtr();
    ASSERT_EQ(representation.summary(), clone->summary()) << "Summaries should be equal";
    ASSERT_EQ(representation.execute(genotype, {inputSource.view()}), clone->execute(genotype, {inputSource.view()})) << "Execution returns should be equal";
}