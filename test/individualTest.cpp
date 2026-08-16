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

#include "evolution/individual.h"
#include "util/counterReset.h"


// Set all file in comment

class IndividualTest : public ::testing::Test
{
  protected:
    virtual void SetUp()
    {
        CounterReset::counterReset();
    }

    virtual void TearDown()
    {
    }
};


TEST_F(IndividualTest, Constructor)
{
    Evolution::Individual* individual;

    ASSERT_NO_THROW(individual = new Evolution::Individual()) << "Constructor of Individual failed.";

    ASSERT_NO_THROW(delete individual) << "Destructor of Individual failed.";
}

TEST_F(IndividualTest, addRemoveNodes)
{
    Evolution::Individual individual;

    ASSERT_EQ(individual.getSize(), 0) << "Getting size of the Individual failed.";

    ASSERT_NO_THROW(individual.addGPNode(std::make_unique<Node::GPNode>(std::vector<double>{1.0, 2.0, 3.0}))) << "Adding GPNode to the Individual failed.";

    ASSERT_EQ(individual.getSize(), 1) << "Getting size of the Individual failed.";

    ASSERT_NO_THROW(individual.addGPNode(std::make_unique<Node::GPNode>(std::vector<size_t>{4, 5, 6}), size_t(0))) << "Adding GPNode to the Individual failed.";

    ASSERT_EQ(individual.getSize(), 2) << "Getting size of the Individual failed.";
    ASSERT_EQ(individual.getGPNode(0).getValues().at(0), Node::NodeValue(size_t{4})) << "Getting values of the GPNode failed.";
    ASSERT_EQ(individual.getMutableGPNode(1).getValues().at(2), Node::NodeValue(3.0)) << "Getting values of the GPNode failed.";

    ASSERT_THROW(individual.addGPNode(std::make_unique<Node::GPNode>(std::vector<double>{7.0, 8.0, 9.0}), size_t(3)), std::runtime_error) << "Adding GPNode to the Individual should have failed.";
    ASSERT_THROW(individual.getGPNode(2), std::runtime_error) << "Getting GPNode of the Individual should have failed.";
    ASSERT_THROW(individual.getMutableGPNode(2), std::runtime_error) << "Getting GPNode of the Individual should have failed.";

    ASSERT_NO_THROW(individual.removeGPNode(0)) << "Removing GPNode to the Individual failed.";
    ASSERT_EQ(individual.getSize(), 1) << "Getting size of the Individual failed.";
    ASSERT_EQ(individual.getMutableGPNode(0).getValues().at(2), Node::NodeValue(3.0)) << "Getting values of the GPNode failed.";
    
    ASSERT_THROW(individual.removeGPNode(1), std::runtime_error) << "Removing GPNode of the Individual should have failed.";
}

TEST_F(IndividualTest, SetGetIntron)
{
    Evolution::Individual individual;

    ASSERT_NO_THROW(individual.addGPNode(std::make_unique<Node::GPNode>(std::vector<double>{1.0, 2.0, 3.0}), false)) << "Adding GPNode to the Individual failed.";
    ASSERT_NO_THROW(individual.addGPNode(std::make_unique<Node::GPNode>(std::vector<size_t>{4, 5, 6}), false)) << "Adding GPNode to the Individual failed.";

    ASSERT_NO_THROW(individual.setIsIntronNode(0, true)) << "Setting intron property of the Individual failed.";

    ASSERT_EQ(individual.getAreIntronNodes(), std::vector<bool>({true, false})) << "Getting intron property of the Individual failed.";
    ASSERT_EQ(individual.getIsIntronNode(0), true) << "Getting intron property of the Individual failed.";
    ASSERT_EQ(individual.getIsIntronNode(1), false) << "Getting intron property of the Individual failed.";


    ASSERT_THROW(individual.getIsIntronNode(2), std::runtime_error) << "Getting intron property of the Individual should have failed.";
    ASSERT_THROW(individual.setIsIntronNode(2, true), std::runtime_error) << "Setting intron property of the Individual should have failed.";
}

TEST_F(IndividualTest, GetGenotype)
{
    Evolution::Individual individual;

    ASSERT_NO_THROW(individual.addGPNode(std::make_unique<Node::GPNode>(std::vector<double>{1.0, 2.0, 3.0}), true)) << "Adding GPNode to the Individual failed.";
    ASSERT_NO_THROW(individual.addGPNode(std::make_unique<Node::GPNode>(std::vector<size_t>{4, 5, 6}), false)) << "Adding GPNode to the Individual failed.";

    std::vector<std::reference_wrapper<const Node::GPNode>> genotype = individual.getGenotype();
    ASSERT_EQ(genotype.size(), 2) << "Getting genotype of the Individual failed.";
    ASSERT_EQ(genotype[0].get().getValues().at(0), Node::NodeValue(1.0)) << "Getting values of the GPNode failed.";
    ASSERT_EQ(genotype[1].get().getValues().at(0), Node::NodeValue(size_t{4})) << "Getting values of the GPNode failed.";

    std::vector<std::reference_wrapper<const Node::GPNode>> effectiveGenotype = individual.getEffectiveGenotype();
    ASSERT_EQ(effectiveGenotype.size(), 1) << "Getting effective genotype of the Individual failed.";
    ASSERT_EQ(effectiveGenotype[0].get().getValues().at(0), Node::NodeValue(size_t{4})) << "Getting values of the GPNode failed.";
}

TEST_F(IndividualTest, hasSameGenotypeAs){
    
    Evolution::Individual individual1;
    Evolution::Individual individual2;

    ASSERT_TRUE(individual1.hasSameGenotypeAs(individual2)) << "Empty individuals should be equal!";

    individual1.addGPNode(std::make_unique<Node::GPNode>(std::vector<double>{1.0, 2.0, 3.0}), false);

    ASSERT_FALSE(individual1.hasSameGenotypeAs(individual2)) << "Should not be equal with different number of nodes";

    individual2.addGPNode(std::make_unique<Node::GPNode>(std::vector<double>{1.0, 2.0}), false);
    ASSERT_FALSE(individual1.hasSameGenotypeAs(individual2)) << "Should not be equal with different sizes of nodes";

    individual2.removeGPNode(0);
    individual2.addGPNode(std::make_unique<Node::GPNode>(std::vector<double>{1.0, 2.0, 4.0}), false);
    ASSERT_FALSE(individual1.hasSameGenotypeAs(individual2)) << "Should not be equal with different values of nodes";
    
    individual2.removeGPNode(0);
    individual2.addGPNode(std::make_unique<Node::GPNode>(std::vector<double>{1.0, 2.0, 3.0}), false);
    ASSERT_TRUE(individual1.hasSameGenotypeAs(individual2)) << "Individuals should be equal";
    
    individual1.addGPNode(std::make_unique<Node::GPNode>(std::vector<size_t>{4, 5, 6}), false);
    individual2.addGPNode(std::make_unique<Node::GPNode>(std::vector<size_t>{4, 5, 6}), true);
    
    ASSERT_TRUE(individual1.hasSameGenotypeAs(individual2)) << "Individuals should be equal";
    ASSERT_TRUE(individual2.hasSameGenotypeAs(individual1)) << "Individuals should be equal both directions";
    ASSERT_FALSE(individual1.hasSameGenotypeAs(individual2, true)) << "Effective genotype should not be equal";
    

}

TEST_F(IndividualTest, IDCounter)
{
    ASSERT_EQ(Evolution::Individual::getIndividualIDCounter(), 0) << "Individual ID counter should be 0 at the beginning.";

    Evolution::Individual individual1;
    Evolution::Individual individual2;
    Evolution::Individual individual3;

    ASSERT_EQ(individual1.getIndividualID(), 0) << "Individual ID should be 0.";
    ASSERT_EQ(individual2.getIndividualID(), 1) << "Individual ID should be 1.";
    ASSERT_EQ(individual3.getIndividualID(), 2) << "Individual ID should be 2.";

    ASSERT_EQ(Evolution::Individual::getIndividualIDCounter(), 3) << "Individual ID counter should be 3 after creating three individuals.";

    individual1.setIndividualID(100);
    ASSERT_EQ(individual1.getIndividualID(), 100) << "Setting Individual ID failed.";

    ASSERT_EQ(Evolution::Individual::getIndividualIDCounter(), 101) << "Individual ID counter should be 101 after setting the first individual's ID.";
    
    // Check <, = and != operators
    ASSERT_FALSE(individual1 == individual2) << "operator == failed.";
    ASSERT_TRUE(individual1 != individual2) << "operator != failed.";
    
    ASSERT_TRUE(individual2 < individual1) << "operator < failed.";
    ASSERT_TRUE(individual1 > individual2) << "operator > failed.";

    ASSERT_TRUE(individual2 <= individual1) << "operator <= failed.";
    ASSERT_TRUE(individual1 >= individual2) << "operator >= failed.";
}