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

#include "evolution/representation.h"
#include "evolution/population.h"
#include "dimensions/numericRange.h"

#include "learn/fakeRepresentation.h"

// Set all file in comment

class RepresentationTest : public ::testing::Test
{
  protected:

    Dimensions::Requirement inputType;
    Dimensions::Requirement outputType;
    virtual void SetUp()
    {
        inputType = Dimensions::Requirement::array1d<double>(4);
        outputType = Dimensions::Requirement::scalar<double>();
    }

    virtual void TearDown()
    {
    }
};



TEST_F(RepresentationTest, Constructor)
{
    Representations::FakeRepresentation* representation;

    ASSERT_NO_THROW(representation = new Representations::FakeRepresentation({inputType}, outputType, 1, 5)) << "Constructor of Representation failed.";

    ASSERT_NO_THROW(delete representation) << "Destructor of Representation failed.";
}



TEST_F(RepresentationTest, getSet)
{
    Representations::FakeRepresentation representation({inputType}, outputType, 10);

    ASSERT_EQ(representation.getMinNbNodes(), 10) << "MinNbNodes value got unexpected value";
    ASSERT_EQ(representation.getMaxNbNodes(), 10) << "MinNbNodes value got unexpected value";
    ASSERT_EQ(representation.getRepresentationName(), "FakeRepresentation") << "Param value got unexpected value";
    ASSERT_EQ(representation.getRepresentationColor(), "#FFFFFF") << "Param value got unexpected value";

    Representations::FakeRepresentation customRep({inputType}, outputType, 1, 5, "CustomRep", "#123456");
    
    ASSERT_EQ(customRep.getMinNbNodes(), 1) << "MinNbNodes value got unexpected value";
    ASSERT_EQ(customRep.getMaxNbNodes(), 5) << "MinNbNodes value got unexpected value";
    ASSERT_EQ(customRep.getRepresentationName(), "CustomRep") << "Param value got unexpected value";
    ASSERT_EQ(customRep.getRepresentationColor(), "#123456") << "Param value got unexpected value";
    ASSERT_NO_THROW(customRep.getGenotypeConstraint()) << "Getting genotype constraint should not fail";
}

TEST_F(RepresentationTest, setDimensions)
{

    std::vector<Dimensions::Requirement> inputSources {
        Dimensions::Requirement::array1d<double>(4, Dimensions::NumericRange<double>::atLeast(1)),
        Dimensions::Requirement::array1d<double>(8),
    };
    Dimensions::Requirement outputSource = Dimensions::Requirement::scalar<double>(Dimensions::NumericRange<double>::between(-1, 1));

    Representations::FakeRepresentation representation(inputSources, outputSource, 10);

    ASSERT_EQ(representation.getDimensionFlow().getInputDimensions().size(), 2) << "Number of input sources set is wrong";
    ASSERT_TRUE(representation.getDimensionFlow().getInputDimensions().at(0) == inputSources.at(0)) << "source is wrong";
    ASSERT_TRUE(representation.getDimensionFlow().getInputDimensions().at(1) == inputSources.at(1)) << "source is wrong";
    ASSERT_EQ(representation.getDimensionFlow().getOutputDimension(), outputSource) << "source is wrong";

    std::string summary = 
"=== Dimension Flow Summary ===\n\nInputs (2):\n  * 'double[4]' in Numeric Range: [1, +inf] of type: double\n  * 'double[8]' in unconstrained\n\nPipeline (1 layers):\n  Layer 1: FakeRepresentation\n    Input (0): 'double[4]' in Numeric Range: [1, +inf] of type: double\n    Input (1): 'double[8]' in unconstrained\n    Output: 'double scalar' in Numeric Range: [-1, 1] of type: double\n    Compatible: YES\n\nOverall: VALID\n";

    ASSERT_EQ(representation.summary(), summary) << "Summary should be equal";
}

TEST_F(RepresentationTest, addOutputFunction) 
{
    Representations::FakeRepresentation representation({inputType}, outputType, 10);

    ASSERT_NO_THROW(representation.addOutputFunction(std::make_unique<Dimensions::ActivationFunctions::ArgMax<double>>(representation.getDimensionFlow().getOutputDimension()))) << "Should not fail to add function";
    ASSERT_THROW(representation.addOutputFunction(std::make_unique<Dimensions::ActivationFunctions::Tanh<double>>(inputType)), std::runtime_error) << "Should not fail to add function";

}

TEST_F(RepresentationTest, isValid)
{
    Dimensions::Requirement customOutput = Dimensions::Requirement::scalar<size_t>(Dimensions::NumericRange<size_t>::between(1, 1));
    Dimensions::Requirement customInput = Dimensions::Requirement::array1d<double>(4, Dimensions::NumericRange<double>::between(-10, 10));
    Representations::FakeRepresentation representation({customInput}, customOutput);
    std::unique_ptr<Node::GenotypeConstraint>& genotypeConstraint = representation.getGenotypeConstraintMut();
    Evolution::Genotype genotype;

    genotypeConstraint = nullptr;
    ASSERT_FALSE(representation.isValid(genotype)) << "Should not be valid";

    genotypeConstraint = std::make_unique<Node::GenotypeConstraint>();
    ASSERT_TRUE(representation.isValid(genotype)) << "Should be valid with empty genotypeConstraint";

    Node::NodeConstraint constraint0;
    constraint0.addConstraint(Dimensions::UnconstrainedData());
    genotypeConstraint->addNodeConstraint(constraint0, 2);
    ASSERT_FALSE(representation.isValid(genotype)) << "Should not be valid";

    genotype.addNodeGroup(std::make_unique<Node::NodeGroup>());
    Node::NodeGroup& group = genotype.getNodeGroup(0);
    group.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{1}));
    ASSERT_FALSE(representation.isValid(genotype)) << "Should be not valid";
    group.addNode(std::make_unique<Node::GPNode>(std::vector<double>{2.6}));
    ASSERT_TRUE(representation.isValid(genotype)) << "Should be valid";
    group.addNode(std::make_unique<Node::GPNode>(std::vector<float>{1.2f}));
    ASSERT_FALSE(representation.isValid(genotype)) << "Should be not valid";

    group.removeNode(2);
    group.removeNode(1);
    group.addNode(std::make_unique<Node::GPNode>(std::vector<double>{2.6, 2.5}));
    ASSERT_FALSE(representation.isValid(genotype)) << "Should be not valid";
    
    group.removeNode(1);
    group.addNode(std::make_unique<Node::GPNode>(std::vector<double>{2.6}));
    ASSERT_TRUE(representation.isValid(genotype)) << "Should be valid again";

    Node::NodeConstraint constraint1;
    constraint1.addConstraint(Dimensions::NumericRange<size_t>::between(1, 1));
    genotypeConstraint->addNodeConstraint(constraint1, 1, 5);
    ASSERT_FALSE(representation.isValid(genotype)) << "Should be not valid anymore";

    genotype.addNodeGroup(std::make_unique<Node::NodeGroup>());
    Node::NodeGroup& group2 = genotype.getNodeGroup(1);
    group2.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{1}));
    ASSERT_TRUE(representation.isValid(genotype)) << "Should be valid again";
    
    group2.addNode(std::make_unique<Node::GPNode>(std::vector<size_t>{2}));
    ASSERT_FALSE(representation.isValid(genotype)) << "Should be valid again";
    group2.removeNode(1);


    /*** INDIVIDUAL COMPATIBILITY ***/

    std::shared_ptr<Evolution::Individual> individual = std::make_shared<Evolution::Individual>(representation);
    individual->setGenotype(genotype.cloneUniquePtr());

    ASSERT_TRUE(individual->isValid()) << "Individual should be valid!!";

    group2.addNode(std::make_unique<Node::GPNode>(std::vector<std::shared_ptr<const Evolution::Individual>>{individual}));
    ASSERT_TRUE(representation.isValid(genotype)) << "Should be valid again";
    group2.removeNode(1);


    // Create boring genotype for wrong subIndividual
    std::unique_ptr<Evolution::Genotype> boringGenotype = std::make_unique<Evolution::Genotype>();
    std::unique_ptr<Node::NodeGroup> boringGroup = std::make_unique<Node::NodeGroup>();
    boringGroup->addNode(std::make_unique<Node::GPNode>(std::vector<double>{1}));
    boringGenotype->addNodeGroup(boringGroup->cloneUniquePtr());

    {
        // Wrong output type of individual
        Dimensions::Requirement customOutput1 = Dimensions::Requirement::scalar<size_t>(Dimensions::NumericRange<size_t>::between(0, 2));
        Representations::FakeRepresentation representation2({customInput}, customOutput1);
        std::shared_ptr<Evolution::Individual> individual2 = std::make_shared<Evolution::Individual>(representation2);
        individual2->setGenotype(boringGenotype->cloneUniquePtr());
        ASSERT_TRUE(individual2->isValid()) << "Individual should be valid!!";
    
        group2.addNode(std::make_unique<Node::GPNode>(std::vector<std::shared_ptr<const Evolution::Individual>>{individual2}));
        ASSERT_FALSE(representation.isValid(genotype)) << "Should be not valid";
        group2.removeNode(1);
    }

    {
        // Too big but good output type of individual
        Dimensions::Requirement customOutput1 = Dimensions::Requirement::array1d<size_t>(3, Dimensions::NumericRange<size_t>::between(1, 1));
        Representations::FakeRepresentation representation2({customInput}, customOutput1);
        std::shared_ptr<Evolution::Individual> individual2 = std::make_shared<Evolution::Individual>(representation2);
        individual2->setGenotype(boringGenotype->cloneUniquePtr());
        ASSERT_TRUE(individual2->isValid()) << "Individual should be valid!!";
        group2.addNode(std::make_unique<Node::GPNode>(std::vector<std::shared_ptr<const Evolution::Individual>>{individual2}));
        ASSERT_TRUE(representation.isValid(genotype)) << "Should be not valid";
        group2.removeNode(1);
    }

    // Wrong intput type of individuals
    Dimensions::Requirement customInput1 = Dimensions::Requirement::array1d<double>(8);
    Representations::FakeRepresentation representation3({customInput1}, customOutput);
    std::shared_ptr<Evolution::Individual> individual3 = std::make_shared<Evolution::Individual>(representation3);
    individual3->setGenotype(boringGenotype->cloneUniquePtr());
    ASSERT_TRUE(individual3->isValid()) << "Individual should be valid!!";

    group2.addNode(std::make_unique<Node::GPNode>(std::vector<std::shared_ptr<const Evolution::Individual>>{individual3}));
    ASSERT_FALSE(representation.isValid(genotype)) << "Should be not valid";
    group2.removeNode(1);

    
    // Good intput type of individuals, even if smaller
    Dimensions::Requirement customInput2 = Dimensions::Requirement::array1d<double>(3, Dimensions::NumericRange<double>::between(-20, 20));
    Representations::FakeRepresentation representation4({customInput2}, customOutput);
    std::shared_ptr<Evolution::Individual> individual4 = std::make_shared<Evolution::Individual>(representation4);
    individual4->setGenotype(boringGenotype->cloneUniquePtr());
    individual4->updateValidity();
    ASSERT_TRUE(individual4->isValid()) << "Individual should be valid!!";

    group2.addNode(std::make_unique<Node::GPNode>(std::vector<std::shared_ptr<const Evolution::Individual>>{individual4}));
    ASSERT_TRUE(representation.isValid(genotype)) << "Should be valid";
    group2.removeNode(1);

    // wrong range should not work
    Dimensions::Requirement customInput3 = Dimensions::Requirement::array1d<double>(4, Dimensions::NumericRange<double>::between(-5, 5));
    Representations::FakeRepresentation representation5({customInput3}, customOutput);
    std::shared_ptr<Evolution::Individual> individual5 = std::make_shared<Evolution::Individual>(representation5);
    individual5->setGenotype(boringGenotype->cloneUniquePtr());
    individual5->updateValidity();
    ASSERT_TRUE(individual5->isValid()) << "Individual should be valid!!";

    group2.addNode(std::make_unique<Node::GPNode>(std::vector<std::shared_ptr<const Evolution::Individual>>{individual5}));
    ASSERT_FALSE(representation.isValid(genotype)) << "Should be valid";
    group2.removeNode(1);

    boringGenotype->addNodeGroup(boringGroup->cloneUniquePtr());
    individual4->setGenotype(boringGenotype->cloneUniquePtr());
    ASSERT_FALSE(individual4->isValid()) << "Individual should not be valid!!";

    group2.addNode(std::make_unique<Node::GPNode>(std::vector<std::shared_ptr<const Evolution::Individual>>{individual4}));
    ASSERT_FALSE(representation.isValid(genotype)) << "Should not be valid";
    group2.removeNode(1);
}

TEST_F(RepresentationTest, execute) 
{
    Representations::FakeRepresentation representation({inputType}, outputType, 10);
    Evolution::Genotype genotype;

    Data::DataValue input1 = Data::DataValue::zeros<double>(4);
    Data::DataValue input2 = Data::DataValue::zeros<double>(2);
    ASSERT_NO_THROW(representation.execute(genotype, {input1.view()})) << "Should not have throw";
    ASSERT_EQ(representation.execute(genotype, {input1.view()}), Data::DataValue::scalar<double>(1)) << "Value is not equal";

    ASSERT_THROW(representation.execute(genotype, {input1.view(), input2.view()}), std::runtime_error) << "Should have throw";
    ASSERT_THROW(representation.execute(genotype, {input2.view()}), std::runtime_error) << "Should have throw";

    
    Data::DataValue input3 = Data::DataValue::zeros<double>(10, 2);
    ASSERT_THROW(representation.execute(genotype, {input3.view()}), std::runtime_error) << "Should have throw";
    
    representation.addOutputFunction(std::make_unique<Dimensions::ActivationFunctions::Tanh<double>>(representation.getDimensionFlow().getOutputDimension()));
    ASSERT_NO_THROW(representation.execute(genotype, {input1.view()})) << "Should not have throw";
    ASSERT_EQ(representation.execute(genotype, {input1.view()}).getScalar<double>(), std::tanh(1.0)) << "Value is not equal";
}