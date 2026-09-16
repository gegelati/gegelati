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

#include "evaluation/scoreMetric.h"

#include "individual.h"
#include "learn/fakeRepresentation.h"
#include "util/counterReset.h"


// Set all file in comment

class IndividualTest : public ::testing::Test
{
  protected:
    Representations::FakeRepresentation fakeRep;
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
    Individual* individual1;
    Individual* individual2;
    std::unique_ptr<GraphBased::Genotype> genotype = GraphBased::Genotype::singleNodeGenotype(std::make_unique<GraphBased::GPNode>(std::vector<double>{1.0, 2.0, 3.0}));

    ASSERT_NO_THROW(individual1 = new Individual(fakeRep)) << "Constructor of Individual failed.";
    ASSERT_NO_THROW(individual2 = new Individual(fakeRep, std::move(genotype))) << "Constructor of Individual failed.";

    ASSERT_NO_THROW(delete individual1) << "Destructor of Individual failed.";
    ASSERT_NO_THROW(delete individual2) << "Destructor of Individual failed.";
}

TEST_F(IndividualTest, SetGetGenotype)
{
    std::unique_ptr<GraphBased::Genotype> genotypeInit = GraphBased::Genotype::singleNodeGenotype(std::make_unique<GraphBased::GPNode>(std::vector<double>{1.0, 2.0, 3.0}));
    Individual individual(fakeRep, std::move(genotypeInit->cloneUniquePtr()));
    const GraphBased::Genotype* genotype;
    ASSERT_NO_THROW(genotype = &individual.getGenotype()) << "Getting genotype failed";


    ASSERT_EQ(individual.getSize(), genotype->getFullSize()) << "Individual size should be size of full genotype";
    ASSERT_TRUE(individual.getGenotype() == *genotypeInit) << "Genotypes should be equals";

    std::unique_ptr<GraphBased::Genotype> genotypeSet = GraphBased::Genotype::singleNodeGenotype(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{4, 5, 6}));
    ASSERT_NO_THROW(individual.setGenotype(std::move(genotypeSet->cloneUniquePtr()))) << "Should not throw";
    ASSERT_TRUE(individual.getGenotype() != *genotypeInit) << "Genotypes should not be equals anymore";
    ASSERT_TRUE(individual.getGenotype() == *genotypeSet) << "Genotypes should be equals";
}

TEST_F(IndividualTest, cloneIndividual)
{       
    std::unique_ptr<GraphBased::Genotype> genotypeSet = GraphBased::Genotype::singleNodeGenotype(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{4, 5, 6}));
    Individual individual(fakeRep);

    std::unique_ptr<Individual> copyIndivUnique;
    ASSERT_NO_THROW(copyIndivUnique = std::move(individual.cloneUniquePtr())) << "Copying individual failed";

    ASSERT_EQ(copyIndivUnique->getSize(), individual.getSize()) << "Copy was not effective";
    ASSERT_TRUE(copyIndivUnique->getGenotype() == individual.getGenotype()) << "Copy was not effective";
    ASSERT_TRUE(&copyIndivUnique->getRepresentation() == &individual.getRepresentation()) << "Copy was not effective";

    std::shared_ptr<Individual> copyIndivShared;
    ASSERT_NO_THROW(copyIndivShared = std::move(individual.cloneSharedPtr())) << "Copying individual failed";

    ASSERT_EQ(copyIndivShared->getSize(), individual.getSize()) << "Copy was not effective";
    ASSERT_TRUE(copyIndivShared->getGenotype() == individual.getGenotype()) << "Copy was not effective";
    ASSERT_TRUE(&copyIndivShared->getRepresentation() == &individual.getRepresentation()) << "Copy was not effective";
}

TEST_F(IndividualTest, results) 
{
    Individual individual(fakeRep);
    const Individual& constIndiv = individual;

    ASSERT_NO_THROW(constIndiv.addEvaluationRun(        
        std::make_unique<Evaluation::EvaluationRun>(
            std::make_unique<Evaluation::ScoreMetric>(8)
        ),
        10
    )) << "Adding an evaluationRun to the result failed";

    const Evaluation::EvaluationResult* result;
    ASSERT_NO_THROW(result = &constIndiv.getEvaluationResult()) << "Getting evaluation result failed";

    ASSERT_EQ(result->getSize(), 1) << "Result should have size 1";
}

TEST_F(IndividualTest, validAndExecute)
{
    std::unique_ptr<GraphBased::Genotype> genotypeSet = GraphBased::Genotype::singleNodeGenotype(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{4, 5, 6}));
    Individual individual(fakeRep, std::move(genotypeSet));

    ASSERT_FALSE(individual.isValid()) << "Should not be valid";
    Data::DataValue source = Data::DataValue::scalar<int>(1);
    ASSERT_THROW(individual.execute({source.view()}), std::runtime_error) << "Should throw with invalid individual";

    std::unique_ptr<GraphBased::Genotype> genotypeSet2 = GraphBased::Genotype::singleNodeGenotype(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{4}));
    individual.setGenotype(std::move(genotypeSet2));

    ASSERT_TRUE(individual.isValid()) << "Should be valid now";
    ASSERT_NO_THROW(individual.execute({source.view()})) << "Should not throw with valid individual";
    ASSERT_EQ(individual.execute({source.view()}).getScalar<double>(), 1.0) << "Value should be 1.0";
}

TEST_F(IndividualTest, IDCounter)
{
    ASSERT_EQ(Individual::getIndividualIDCounter(), 0) << "Individual ID counter should be 0 at the beginning.";

    Individual individual1(fakeRep);
    Individual individual2(fakeRep);
    Individual individual3(fakeRep);

    ASSERT_EQ(individual1.getIndividualID(), 0) << "Individual ID should be 0.";
    ASSERT_EQ(individual2.getIndividualID(), 1) << "Individual ID should be 1.";
    ASSERT_EQ(individual3.getIndividualID(), 2) << "Individual ID should be 2.";

    ASSERT_EQ(Individual::getIndividualIDCounter(), 3) << "Individual ID counter should be 3 after creating three individuals.";

    individual1.setIndividualID(100);
    ASSERT_EQ(individual1.getIndividualID(), 100) << "Setting Individual ID failed.";

    ASSERT_EQ(Individual::getIndividualIDCounter(), 101) << "Individual ID counter should be 101 after setting the first individual's ID.";
    
    // Check <, = and != operators
    ASSERT_FALSE(individual1 == individual2) << "operator == failed.";
    ASSERT_TRUE(individual1 != individual2) << "operator != failed.";
    
    ASSERT_TRUE(individual2 < individual1) << "operator < failed.";
    ASSERT_TRUE(individual1 > individual2) << "operator > failed.";

    ASSERT_TRUE(individual2 <= individual1) << "operator <= failed.";
    ASSERT_TRUE(individual1 >= individual2) << "operator >= failed.";
}