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

#include "representations/lgpRepresentation.h"


// Set all file in comment

class LGPRepresentationTest : public ::testing::Test
{
  protected:
    Instructions::Set set;

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(LGPRepresentationTest, Constructor)
{
    Representations::LGPRepresentation* representation;

    ASSERT_NO_THROW(representation = new Representations::LGPRepresentation(set, 8, 5, 10)) << "Constructor of Representation failed.";

    ASSERT_NO_THROW(delete representation) << "Destructor of Representation failed.";
}

TEST_F(LGPRepresentationTest, isValid)
{
    Representations::LGPRepresentation representation(set, 8, 5, 10);

    Evolution::Individual indiv;
    for(size_t i = 0; i < 4; i++) {
        indiv.addGPNode(std::make_unique<Node::GPNode>(std::vector<size_t>{0, 0, 0, 0, 0, 0}));
    }

    ASSERT_FALSE(representation.isValid(indiv)) << "Individual should not be valid with 4 nodes";

    indiv.addGPNode(std::make_unique<Node::GPNode>(std::vector<size_t>{7, 3, 1, 7, 1, 7}));
    ASSERT_TRUE(representation.isValid(indiv)) << "Individual should be valid with 5 nodes";

    indiv.addGPNode(std::make_unique<Node::GPNode>(std::vector<size_t>{8, 3, 1, 7, 1, 7}));
    ASSERT_FALSE(representation.isValid(indiv)) << "Individual should not be valid with wrong node";
    indiv.removeGPNode(indiv.getSize() - 1);

    
    indiv.addGPNode(std::make_unique<Node::GPNode>(std::vector<double>{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}));
    ASSERT_FALSE(representation.isValid(indiv)) << "Individual should not be valid with wrong node";
    indiv.removeGPNode(indiv.getSize() - 1);


    for(size_t i = 0; i < 6; i++) {
        indiv.addGPNode(std::make_unique<Node::GPNode>(std::vector<size_t>{0, 0, 0, 0, 0, 0}));
    }

    ASSERT_EQ(indiv.getSize(), 11) << "Individual size should now be 11";
    ASSERT_FALSE(representation.isValid(indiv)) << "Individual should not be valid with 11 nodes";
}