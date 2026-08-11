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

#include "evolution/population.h"
#include "util/counterReset.h"


// Set all file in comment

class PopulationTest : public ::testing::Test
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


TEST_F(PopulationTest, Constructor)
{
    Evolution::Population* population;

    ASSERT_NO_THROW(population = new Evolution::Population()) << "Constructor of Population failed.";

    ASSERT_NO_THROW(delete population) << "Destructor of Population failed.";
}

TEST_F(PopulationTest, createIndividual)
{       
    Evolution::Population population;

    ASSERT_NO_THROW(population.createIndividual()) << "Creating Individual in Population failed.";
    ASSERT_EQ(population.size(), 1) << "Size of the Population should be 1.";
    
    const Evolution::Individual& individual = population.createIndividual();
    ASSERT_EQ(population.size(), 2) << "Size of the Population should be 2.";
    ASSERT_TRUE(population.containsIndividual(individual)) << "Population should contain the individual.";

    std::vector<std::reference_wrapper<const Evolution::Individual>> individuals;
    ASSERT_NO_THROW(individuals = population.getIndividuals()) << "Getting Individuals from Population failed.";
    ASSERT_EQ(individuals.size(), 2) << "Size of the Individuals vector should be 2.";
}
TEST_F(PopulationTest, copyIndividual)
{       
    Evolution::Population population;
    const Evolution::Individual& individual = population.createIndividual();

    Evolution::Individual* mutIndiv;
    ASSERT_NO_THROW(mutIndiv = &population.getMutableIndividual(individual)) << "Getting individual should not fail.";
    mutIndiv->addGPNode(std::make_unique<Node::GPNode>(std::vector<double>{1.0, 2.0, 3.0}));
    mutIndiv->addGPNode(std::make_unique<Node::GPNode>(std::vector<double>{4.0, 5.0, 6.0}));
    mutIndiv->addGPNode(std::make_unique<Node::GPNode>(std::vector<double>{7.0, 8.0, 9.0}));

    const Evolution::Individual* copyIndiv;
    ASSERT_NO_THROW(copyIndiv = &population.copyIndividual(individual)) << "Copying individual failed";

    ASSERT_EQ(copyIndiv->getSize(), 3) << "Copy was not effective";
    ASSERT_EQ(copyIndiv->getGPNode(1).getValue(1), Node::NodeValue{5.0}) << "Copy was not effective";
}
TEST_F(PopulationTest, deleteIndividual)
{       
    Evolution::Population population;
    const Evolution::Individual& individual1 = population.createIndividual();
    const Evolution::Individual& individual2 = population.createIndividual();
    const Evolution::Individual& individual3 = population.createIndividual();

    ASSERT_EQ(population.size(), 3) << "Size of the Population should be 3.";

    ASSERT_NO_THROW(population.deleteIndividual(individual2)) << "Delete individual failed";

    ASSERT_EQ(population.size(), 2) << "Size of the Population should be 2.";

    
    Evolution::Population population2;
    const Evolution::Individual& individual4 = population2.createIndividual();
    ASSERT_THROW(population.deleteIndividual(individual4), std::runtime_error) << "Delete individual should fail";
}

TEST_F(PopulationTest, emptyIndividual)
{       

    Evolution::Population population;
    const Evolution::Individual& individual = population.createIndividual();
    Evolution::Individual& mutIndiv = population.getMutableIndividual(individual);

    mutIndiv.addGPNode(std::make_unique<Node::GPNode>(std::vector<double>{1.0, 2.0, 3.0}));
    mutIndiv.addGPNode(std::make_unique<Node::GPNode>(std::vector<double>{4.0, 5.0, 6.0}));
    mutIndiv.addGPNode(std::make_unique<Node::GPNode>(std::vector<double>{7.0, 8.0, 9.0}));

    ASSERT_NO_THROW(population.emptyIndividual(individual)) << "Emptying individual failed";
    ASSERT_EQ(individual.getSize(), 0) << "Size of the individual after empty should be 0.";
}
TEST_F(PopulationTest, clearIndividuals)
{       
    Evolution::Population population;
    const Evolution::Individual& individual1 = population.createIndividual();
    const Evolution::Individual& individual2 = population.createIndividual();
    const Evolution::Individual& individual3 = population.createIndividual();

    ASSERT_EQ(population.size(), 3) << "Size of the Population should be 3.";

    ASSERT_NO_THROW(population.clearIndividuals()) << "Clear Individuals failed";
    ASSERT_EQ(population.size(), 0) << "Size of the Population should be 0.";

}

TEST_F(PopulationTest, IDCounter)
{
    ASSERT_EQ(Evolution::Population::getPopulationIDCounter(), 0) << "Population ID counter should be 0 at the beginning.";

    Evolution::Population population1;
    Evolution::Population population2;
    Evolution::Population population3;

    ASSERT_EQ(population1.getPopulationID(), 0) << "Population ID should be 0.";
    ASSERT_EQ(population2.getPopulationID(), 1) << "Population ID should be 1.";
    ASSERT_EQ(population3.getPopulationID(), 2) << "Population ID should be 2.";

    ASSERT_EQ(Evolution::Population::getPopulationIDCounter(), 3) << "Population ID counter should be 3 after creating three populations.";

    population1.setPopulationID(100);
    ASSERT_EQ(population1.getPopulationID(), 100) << "Setting Population ID failed.";

    ASSERT_EQ(Evolution::Population::getPopulationIDCounter(), 101) << "Population ID counter should be 101 after setting the first population's ID.";

    
    // Check <, = and != operators
    ASSERT_TRUE(population1 != population2) << "operator != failed.";
    ASSERT_TRUE(population2 < population1) << "operator < failed.";
    ASSERT_FALSE(population1 == population2) << "operator == failed.";
}