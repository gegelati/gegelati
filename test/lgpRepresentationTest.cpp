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
    Dimensions::Requirement inputType;
    Dimensions::Requirement outputType;

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
        outputType = Dimensions::Requirement::scalar<double>();
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

    ASSERT_NO_THROW(representation = new Representations::LGPRepresentation({inputType}, 1, set, 8, 5, 10)) << "Constructor of Representation failed.";
    ASSERT_THROW(Representations::LGPRepresentation({inputType}, 10, set, 8, 5, 10), std::runtime_error) << "Constructor of Representation should fail.";

    ASSERT_NO_THROW(representation->cloneUniquePtr()) << "Cloning should not fail";

    ASSERT_NO_THROW(delete representation) << "Destructor of Representation failed.";
}

TEST_F(LGPRepresentationTest, getGenotypeConstraint)
{
    size_t nbRegisters = 8;
    Representations::LGPRepresentation representation({inputType}, 1, set, 8, 5, 10);
    const GraphBased::GenotypeConstraint& constraint = representation.getGenotypeConstraint();

    ASSERT_EQ(constraint.size(), 1) << "Size of constraint should be 1";
    ASSERT_EQ(constraint.getRangeAt(0).first, 5) << "Low range should be 5";
    ASSERT_EQ(constraint.getRangeAt(0).second, 10) << "Low range should be 5";

    const GraphBased::NodeConstraint& nodeConstraint = constraint.getNodeConstraintAt(0);
    ASSERT_EQ(nodeConstraint.size(), 6) << "Size should be 6";

    ASSERT_TRUE(nodeConstraint.getConstraintAt(0) == Dimensions::NumericRange<size_t>(0, nbRegisters - 1)) << "Should be compatible";
    ASSERT_TRUE(nodeConstraint.getConstraintAt(1) == Dimensions::NumericRange<size_t>(0, set.getNbInstructions() - 1)) << "Should be compatible";
    ASSERT_TRUE(nodeConstraint.getConstraintAt(2) == Dimensions::NumericRange<size_t>(0, 1)) << "Should be compatible";
    ASSERT_TRUE(nodeConstraint.getConstraintAt(3) == Dimensions::NumericRange<size_t>(0, nbRegisters - 1)) << "Should be compatible";
    ASSERT_TRUE(nodeConstraint.getConstraintAt(4) == Dimensions::NumericRange<size_t>(0, 1)) << "Should be compatible";
    ASSERT_TRUE(nodeConstraint.getConstraintAt(5) == Dimensions::NumericRange<size_t>(0, nbRegisters - 1)) << "Should be compatible";
}

TEST_F(LGPRepresentationTest, getGenotypeGenerator)
{
    Representations::LGPRepresentation representation({inputType}, 1, set, 8, 5, 10);
    RNG::RNG rng;

    std::unique_ptr<GraphBased::GenotypeGenerator> generator = representation.getGenotypeGenerator();
    
    ASSERT_EQ(generator->size(), 1) << "Size of generator should be 1";
    ASSERT_EQ(generator->getRangeAt(0).first, 5) << "Low range should be 5";
    ASSERT_EQ(generator->getRangeAt(0).second, 10) << "Low range should be 5";

    GraphBased::NodeGenerator& nodeGen = generator->getNodeGeneratorAt(0);
    ASSERT_EQ(nodeGen.size(), 6) << "Size should be 6";

    size_t value;
    for(size_t idx = 0; idx < 1000; idx++) {
        ASSERT_TRUE(nodeGen.getGeneratorAt(0).sample(rng).getScalar<size_t>() < 8);
        ASSERT_TRUE(nodeGen.getGeneratorAt(1).sample(rng).getScalar<size_t>() < set.getNbInstructions());
        ASSERT_TRUE(nodeGen.getGeneratorAt(2).sample(rng).getScalar<size_t>() < 2);
        ASSERT_TRUE(nodeGen.getGeneratorAt(3).sample(rng).getScalar<size_t>() < 8);
        ASSERT_TRUE(nodeGen.getGeneratorAt(4).sample(rng).getScalar<size_t>() < 2);
        ASSERT_TRUE(nodeGen.getGeneratorAt(5).sample(rng).getScalar<size_t>() < 8);
    }
}


TEST_F(LGPRepresentationTest, executeIndividual)
{
    Data::DataValue inputSource = Data::DataValue::array1d<double[4]>({1.0, 1.5, 2.0, -1.0});

    Representations::LGPRepresentation representation({inputType}, 1, set, 8, 5, 10);

    Evolution::Genotype genotype;
    std::unique_ptr<GraphBased::NodeGroup> group = std::make_unique<GraphBased::NodeGroup>();
    
    group->addNode(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{1, 2, 1, 5, 1, 2}));// R[1] = S[1] * S[2] = 3.0
    group->addNode(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{2, 0, 0, 3, 1, 0}));// R[2] = R[3] + S[0] = 1.0
    group->addNode(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{2, 3, 0, 2, 0, 2}));// R[2] = R[2] / R[2] = 1.0 / 1.0 = 1.0
    group->addNode(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0, 1, 1, 2, 1, 1}));// R[0] = S[2] - S[1] = 0.5
    group->addNode(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0, 0, 0, 0, 0, 2}));// R[0] = R[0] - R[2] = 0.5 + 1 = 1.5

    genotype.addNodeGroup(group->cloneUniquePtr());

    ASSERT_TRUE(representation.isValid(genotype)) << "Individual should be valid";

    ASSERT_NO_THROW(representation.execute(genotype, {inputSource.view()})) << "Execution of individual failed.";
    Data::DataValue output = representation.execute(genotype, {inputSource.view()});
    ASSERT_EQ(output.getScalar<double>(), 1.5) << "Value is not correct.";

    // R[0] = R[0] + R[0] = -1, but set as intron

    group->addNode(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0, 0, 0, 0, 0, 0}, true));
    genotype.setNodeGroup(group->cloneUniquePtr(), 0);
    ASSERT_NO_THROW(output = representation.execute(genotype, {inputSource.view()})) << "Execution of individual failed.";
    ASSERT_EQ(output.getScalar<double>(), 1.5) << "Value is not correct.";
}

TEST_F(LGPRepresentationTest, compatibilityCheck) 
{

    Evolution::Genotype genotype;
    std::unique_ptr<GraphBased::NodeGroup> group = std::make_unique<GraphBased::NodeGroup>();
    
    group->addNode(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{1, 2, 1, 5, 1, 2}));// R[1] = S[1] * S[2] = 3.0
    group->addNode(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{2, 0, 0, 3, 1, 0}));// R[2] = R[3] + S[0] = 1.0
    group->addNode(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{2, 3, 0, 2, 0, 2}));// R[2] = R[2] / R[2] = 1.0 / 1.0 = 1.0
    group->addNode(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0, 1, 1, 2, 1, 1}));// R[0] = S[2] - S[1] = 0.5
    group->addNode(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0, 0, 0, 0, 0, 2}));// R[0] = R[0] - R[2] = 0.5 + 1 = 1.5

    genotype.addNodeGroup(std::move(group));
    
    Dimensions::Requirement inputType = Dimensions::Requirement::array1d<double>(4, Dimensions::NumericRange<double>::between(-3.0, 3.0));
    Data::DataValue inputSource = Data::DataValue::array1d<double[4]>({1.0, 1.5, 2.0, -1.0});

    Representations::LGPRepresentation representation({inputType}, 5, set, 8, 5, 10);

    std::string text = representation.summary();
    
    EXPECT_NE(text.find("Layer 1: LGP"), std::string::npos);
    EXPECT_NE(text.find("Overall: VALID"), std::string::npos);

    
    Dimensions::Requirement inputTypeEnv = Dimensions::Requirement::array1d<double>(4, Dimensions::NumericRange<double>::between(-1.0, 1.0));
    Dimensions::Requirement outputTypeEnv = Dimensions::Requirement::scalar<double>(Dimensions::NumericRange<double>::between(-1.0, 1.0));
    ASSERT_TRUE(inputTypeEnv.isCompatibleWith(representation.getDimensionFlow().getInputDimensions().at(0))) << "Representation should be compatible with environment output";
    ASSERT_FALSE(representation.getDimensionFlow().isCompatibleWith(outputTypeEnv)) << "Representation should not be compatible with environment output";
    ASSERT_TRUE(representation.execute(genotype, {inputSource.view()}) == Data::DataValue::array1d<double[5]>({1.5, 3.0, 1.0, 0.0, 0.0})) << "Values should be equal";

    representation.addOutputFunction(std::make_unique<Dimensions::ActivationFunctions::Tanh<double>>(representation.getDimensionFlow().getOutputDimension()));
    text = representation.summary();
    EXPECT_NE(text.find("Layer 2: TanH"), std::string::npos);
    EXPECT_NE(text.find("Overall: VALID"), std::string::npos);

    ASSERT_TRUE(inputTypeEnv.isCompatibleWith(representation.getDimensionFlow().getInputDimensions().at(0))) << "Representation should be compatible with environment output";
    ASSERT_TRUE(representation.getDimensionFlow().isCompatibleWith(outputTypeEnv)) << "Representation should now be compatible with environment output";
    ASSERT_TRUE(representation.execute(genotype, {inputSource.view()}) == Data::DataValue::array1d<double[5]>({std::tanh(1.5), std::tanh(3.0), std::tanh(1.0), 0.0, 0.0})) << "Values should be equal";

    representation.addOutputFunction(std::make_unique<Dimensions::ActivationFunctions::ArgMax<double>>(representation.getDimensionFlow().getOutputDimension()));
    text = representation.summary();
    EXPECT_NE(text.find("Layer 3: ArgMax"), std::string::npos);
    EXPECT_NE(text.find("Overall: VALID"), std::string::npos);
    
    ASSERT_TRUE(inputTypeEnv.isCompatibleWith(representation.getDimensionFlow().getInputDimensions().at(0))) << "Representation should be compatible with environment output";
    ASSERT_FALSE(representation.getDimensionFlow().isCompatibleWith(outputTypeEnv)) << "Representation should not be compatible with environment output";
    ASSERT_TRUE(representation.execute(genotype, {inputSource.view()}) == Data::DataValue::scalar<size_t>(1u)) << "Values should be equal";

    std::unique_ptr<Evolution::Representation> clone = representation.cloneUniquePtr();
    ASSERT_EQ(representation.summary(), clone->summary()) << "Summaries should be equal";
    ASSERT_EQ(representation.execute(genotype, {inputSource.view()}), clone->execute(genotype, {inputSource.view()})) << "Execution returns should be equal";
}