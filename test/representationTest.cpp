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


// Set all file in comment

class RepresentationTest : public ::testing::Test
{
  protected:

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

class FakeRepresentation : public Evolution::Representation 
{
    public: 
        std::unique_ptr<Evolution::Representation> cloneUniquePtr() const {
            return std::make_unique<FakeRepresentation>(*this);
        }
        
        FakeRepresentation(size_t nbNodesMin, size_t nbNodesMax=0, std::string representationName = "FakeRepresentation", std::string representationColor = "#000000"): Representation(nbNodesMin, nbNodesMax, representationName, representationColor) {};

        std::unique_ptr<const Node::GenotypeTemplate> getGenotypeTemplate() const override { return nullptr;}; 

        bool isValid(const Evolution::Individual& indiv) const {
            return true;
        }

        std::vector<double> executeIndividual(
            const Evolution::Individual& indiv, const std::vector<std::reference_wrapper<const Data::DataHandler>>& inputSources) const {
            return {0.0};
        }
};


TEST_F(RepresentationTest, Constructor)
{
    FakeRepresentation* representation;

    ASSERT_NO_THROW(representation = new FakeRepresentation(1, 5)) << "Constructor of Representation failed.";

    ASSERT_NO_THROW(delete representation) << "Destructor of Representation failed.";
}



TEST_F(RepresentationTest, getSet)
{
    FakeRepresentation representation(10);

    ASSERT_EQ(representation.getMinNbNodes(), 10) << "MinNbNodes value got unexpected value";
    ASSERT_EQ(representation.getMaxNbNodes(), 10) << "MinNbNodes value got unexpected value";
    ASSERT_EQ(representation.getRepresentationName(), "FakeRepresentation") << "Param value got unexpected value";
    ASSERT_EQ(representation.getRepresentationColor(), "#000000") << "Param value got unexpected value";

    FakeRepresentation customRep(1, 5, "CustomRep", "#123456");
    
    ASSERT_EQ(customRep.getMinNbNodes(), 1) << "MinNbNodes value got unexpected value";
    ASSERT_EQ(customRep.getMaxNbNodes(), 5) << "MinNbNodes value got unexpected value";
    ASSERT_EQ(customRep.getRepresentationName(), "CustomRep") << "Param value got unexpected value";
    ASSERT_EQ(customRep.getRepresentationColor(), "#123456") << "Param value got unexpected value";
}

TEST_F(RepresentationTest, setInputDimensions)
{
    FakeRepresentation representation(10);

    std::vector<std::reference_wrapper<const Data::DataHandler>> inputSources;
    inputSources.push_back(*(new Data::PrimitiveTypeArray<double>(4)));
    inputSources.push_back(*(new Data::PrimitiveTypeArray<double>(8)));
    inputSources.push_back(*(new Data::PrimitiveTypeArray<double>(6)));

    ASSERT_NO_THROW(representation.setInputDimensions(inputSources)) << "Setting input dimensions failed";

    ASSERT_EQ(representation.getNbInputSources(), 3) << "Number of input sources set is wrong";
    ASSERT_EQ(representation.getMaxInputSourceIdx(), 8) << "max index of input source set is wrong";
    
    delete (&(inputSources.at(0).get()));
    delete (&(inputSources.at(1).get()));
    delete (&(inputSources.at(2).get()));
}