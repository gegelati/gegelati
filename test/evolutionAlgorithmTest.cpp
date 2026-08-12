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

#include "instructions/set.h"
#include "instructions/lambdaInstruction.h"
#include "evolution/evolutionAlgorithm.h"
#include "representations/lgpRepresentation.h"


// Set all file in comment

class EvolutionAlgorithmTest : public ::testing::Test
{
  protected:
    Instructions::Set set;
    Data::PrimitiveTypeArray<double>* inputSource;
    Evolution::Representation* representation;

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
    
        representation = new Representations::LGPRepresentation(set, 8, 10);
    }

    virtual void TearDown()
    {
    }
};


TEST_F(EvolutionAlgorithmTest, Constructor)
{
    Evolution::EvolutionAlgorithm* ea;

    ASSERT_NO_THROW(ea = new Evolution::EvolutionAlgorithm(*representation)) << "Constructor of EA failed.";

    ASSERT_EQ(ea->getRepresentation().getMaxNbNodes(), representation->getMaxNbNodes()) << "Constructor should have copied the representation";

    ASSERT_NO_THROW(delete ea) << "Destructor of EA failed.";
}

TEST_F(EvolutionAlgorithmTest, initializePopulation)
{
    Evolution::EvolutionAlgorithm ea(*representation);
    representation->setInputDimensions({*inputSource});

    ASSERT_NO_THROW(ea.initializePopulation()) << "Initialization of population failed.";

    ASSERT_EQ(ea.getPopulation().size(), 100) << "Population size is wrong after initialization.";
}