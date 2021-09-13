/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
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

#include <gtest/gtest.h>

#include "data/dataHandler.h"
#include "data/primitiveTypeArray.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/lambdaInstruction.h"
#include "instructions/set.h"
#include "mutator/rng.h"
#include "program/line.h"
#include "program/program.h"

#include "archive.h"

class ArchiveTest : public ::testing::Test
{
  protected:
    const size_t size1{24};
    const size_t size2{32};
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
    Instructions::Set set;
    Environment* e;
    Program::Program* p;

    virtual void SetUp()
    {
        vect.push_back(
            *(new Data::PrimitiveTypeArray<double>((unsigned int)size1)));
        vect.push_back(
            *(new Data::PrimitiveTypeArray<int>((unsigned int)size2)));

        set.add(*(new Instructions::AddPrimitiveType<int>()));

        auto minus = [](double a, double b) -> double { return a - b; };
        set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));

        e = new Environment(set, vect, 8, 5);
        p = new Program::Program(*e);
    }

    virtual void TearDown()
    {
        delete p;
        delete e;
        delete (&(vect.at(0).get()));
        delete (&(vect.at(1).get()));
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
    }
};

TEST_F(ArchiveTest, ConstructorDestructor)
{
    Archive* a;
    ASSERT_NO_THROW(a = new Archive();)
        << "Default construction of an Archive failed";

    ASSERT_NO_THROW(delete a;) << "Destruction of an empty Archive failed.";
}

TEST_F(ArchiveTest, CombineHash)
{
    size_t hash;

    ASSERT_NO_THROW(hash = Archive::getCombinedHash(vect))
        << "Combination of several DataHandler hash failed.";

    // change data in one dataHandler
    Data::PrimitiveTypeArray<int>& d =
        (Data::PrimitiveTypeArray<int>&)vect.at(1).get();
    d.setDataAt(typeid(int), 2, 1337);

    // Compare hashes.
    ASSERT_NE(Archive::getCombinedHash(vect), hash);
}

TEST_F(ArchiveTest, AddRecordingTests)
{
    // For these test, force archivingProbability to 1
    Archive archive(3, 1.0);

    // Add a fictive recording
    ASSERT_NO_THROW(archive.addRecording(p, vect, 1.3))
        << "Adding a recording to the empty archive failed.";

    ASSERT_EQ(archive.getNbRecordings(), 1)
        << "Number or recordings in the archive is incorrect.";
    ASSERT_EQ(archive.getNbDataHandlers(), 1)
        << "Number or dataHandlers copied in the archive is incorrect.";

    // Add other recordings with the same DataHandlers
    Program::Program p2(*e);
    ASSERT_NO_THROW(archive.addRecording(&p2, vect, 0.3))
        << "Adding a recording to the non-empty archive failed.";
    ASSERT_EQ(archive.getNbRecordings(), 2)
        << "Number or recordings in the archive is incorrect.";
    ASSERT_EQ(archive.getNbDataHandlers(), 1)
        << "Number or dataHandlers copied in the archive is incorrect.";

    // Add another recording with a new environment
    // change data in one dataHandler
    Data::PrimitiveTypeArray<int>& d =
        (Data::PrimitiveTypeArray<int>&)vect.at(1).get();

    d.setDataAt(typeid(int), 2, 1337);
    ASSERT_NO_THROW(archive.addRecording(p, vect, 0.2))
        << "Adding a recording to the non-empty archive failed.";
    ASSERT_EQ(archive.getNbRecordings(), 3)
        << "Number or recordings in the archive is incorrect.";
    ASSERT_EQ(archive.getNbDataHandlers(), 2)
        << "Number or dataHandlers copied in the archive is incorrect.";

    // Reach the archive size limit.
    ASSERT_NO_THROW(archive.addRecording(&p2, vect, 0.5))
        << "Adding a recording to the full archive failed.";
    ASSERT_EQ(archive.getNbRecordings(), 3)
        << "Number or recordings in the archive is incorrect.";
    ASSERT_EQ(archive.getNbDataHandlers(), 2)
        << "Number or dataHandlers copied in the archive is incorrect.";

    // Evict a recording again, and its DataHandler copy.
    Program::Program p3(*e);
    ASSERT_NO_THROW(archive.addRecording(&p3, vect, 1.5))
        << "Adding a recording to the full archive failed.";
    ASSERT_EQ(archive.getNbRecordings(), 3)
        << "Number or recordings in the archive is incorrect.";
    ASSERT_EQ(archive.getNbDataHandlers(), 1)
        << "Number or dataHandlers copied in the archive is incorrect.";
}

TEST_F(ArchiveTest, AddRecordingWithProbabilityTests)
{
    // For these test, force archivingProbability to 0.5
    // Use a known seed
    Archive archive(10, 0.5, 0);

    // Add a few fictive recording
    for (int i = 0; i < 10; i++) {
        const_cast<Data::PrimitiveTypeArray<int>&>(
            dynamic_cast<const Data::PrimitiveTypeArray<int>&>(
                vect.at(1).get()))
            .setDataAt(typeid(int), 0, i);
        ASSERT_NO_THROW(archive.addRecording(p, vect, (double)i))
            << "Adding a recording to the archive failed.";
    }
    ASSERT_EQ(archive.getNbRecordings(), 4)
        << "Number or recordings in the archive is incorrect with a known "
           "seed.";
}

TEST_F(ArchiveTest, At)
{
    // For these test, force archivingProbability to 0.5
    // Use a known seed
    Archive archive(10, 1.0);

    // Add a few fictive recording
    for (int i = 0; i < 5; i++) {
        const_cast<Data::PrimitiveTypeArray<int>&>(
            dynamic_cast<const Data::PrimitiveTypeArray<int>&>(
                vect.at(1).get()))
            .setDataAt(typeid(int), 0, i);
        ASSERT_NO_THROW(archive.addRecording(p, vect, (double)i))
            << "Adding a recording to the archive failed.";
    }

    // Access the 1st recording
    ASSERT_NO_THROW(archive.at(1))
        << "Accessing an existing ArchiveRecording should not fail.";

    // Access the 7th (non existing) recording
    ASSERT_THROW(archive.at(7), std::out_of_range)
        << "Accessing an ArchiveRecording outside the number of recordings "
           "should fail.";
}

TEST_F(ArchiveTest, SetSeed)
{
    // For these test, force archivingProbability to 0.5
    // Use a known seed
    Archive archive(10, 0.5);

    ASSERT_NO_THROW(archive.setRandomSeed(1))
        << "Setting a new seed failed unexpectedly.";

    // Add a few fictive recording
    for (int i = 0; i < 10; i++) {
        const_cast<Data::PrimitiveTypeArray<int>&>(
            dynamic_cast<const Data::PrimitiveTypeArray<int>&>(
                vect.at(1).get()))
            .setDataAt(typeid(int), 0, i);
        ASSERT_NO_THROW(archive.addRecording(p, vect, (double)i))
            << "Adding a recording to the archive failed.";
    }
    // With a seed set to 0, result is available in
    // AddRecordingWithProbabilityTests
    ASSERT_EQ(archive.getNbRecordings(), 7)
        << "Number or recordings in the archive is incorrect with a known "
           "seed.";
}

TEST_F(ArchiveTest, areProgramResultsUnique)
{
    Archive archive(4);
    size_t hash1 = archive.getCombinedHash(vect);
    Data::PrimitiveTypeArray<int>& d =
        const_cast<Data::PrimitiveTypeArray<int>&>(
            dynamic_cast<const Data::PrimitiveTypeArray<int>&>(
                vect.at(1).get()));

    // Add a few fictive recordings with p
    archive.addRecording(p, vect, 1.0);
    d.setDataAt(typeid(int), 2, 1337);
    size_t hash2 = archive.getCombinedHash(vect);
    archive.addRecording(p, vect, 1.5);

    // Add a few fictive recordings with p2
    Program::Program p2(*e);
    archive.addRecording(&p2, vect, 2.0);
    d.setDataAt(typeid(int), 2, 42);
    size_t hash3 = archive.getCombinedHash(vect);
    archive.addRecording(&p2, vect, 2.5);

    // results are entirely different
    std::map<size_t, double> hashesAndResults1 = {{hash1, 3.0}, {hash2, 3.5}};
    ASSERT_TRUE(archive.areProgramResultsUnique(hashesAndResults1))
        << "Unique fake program bidding behavior not detected as such.";
    std::map<size_t, double> hashesAndResults2 = {
        {hash1, 0.0}, {hash2, 2.0}, {hash3, 2.5}};
    ASSERT_FALSE(archive.areProgramResultsUnique(hashesAndResults2))
        << "Equal fake program bidding behavior not detected as such.";
    std::map<size_t, double> hashesAndResults3 = {
        {hash1, 1.2}, {hash2, 1.3}, {hash3, 3.5}};
    ASSERT_FALSE(archive.areProgramResultsUnique(hashesAndResults3, 0.21))
        << "Within margin fake program bidding behavior not detected as such.";
}

TEST_F(ArchiveTest, DataHandlersAccessors)
{
    Archive archive(4);

    // Add a few fictive recordings
    archive.addRecording(p, vect, 1.0);
    archive.addRecording(p, vect, 1.5);
    Data::PrimitiveTypeArray<int>& d =
        const_cast<Data::PrimitiveTypeArray<int>&>(
            dynamic_cast<const Data::PrimitiveTypeArray<int>&>(
                vect.at(1).get()));
    d.setDataAt(typeid(int), 2, 1337);
    archive.addRecording(p, vect, 2.0);
    archive.addRecording(p, vect, 2.3);

    ASSERT_TRUE(archive.hasDataHandlers(archive.getCombinedHash(vect)))
        << "Data handler should be detected as present within the archive.";
    d.setDataAt(typeid(int), 2, 666);
    ASSERT_FALSE(archive.hasDataHandlers(archive.getCombinedHash(vect)))
        << "Data handler should be detected as not present within the archive.";
    auto dhandlers = archive.getDataHandlers();
    ASSERT_EQ(dhandlers.size(), 2);
}

TEST_F(ArchiveTest, Clear)
{
    Archive archive(4);

    ASSERT_NO_THROW(archive.clear())
        << "Clearing an empty archive should not fail.";

    // Add a few fictive recordings
    archive.addRecording(p, vect, 1.0);
    archive.addRecording(p, vect, 1.5);

    ASSERT_NO_THROW(archive.clear())
        << "Clearing a non-empty archive should not fail.";
    ASSERT_EQ(archive.getNbRecordings(), 0)
        << "Number or recordings in the archive is incorrect.";
    ASSERT_EQ(archive.getNbDataHandlers(), 0)
        << "Number or dataHandlers copied in the archive is incorrect.";
}
