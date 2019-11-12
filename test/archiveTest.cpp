#include <gtest/gtest.h>

#include "instructions/set.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/multByConstParam.h"
#include "dataHandlers/dataHandler.h"
#include "dataHandlers/primitiveTypeArray.h"
#include "program/program.h"
#include "program/line.h"
#include "mutator/rng.h"

#include "archive.h"

class ArchiveTest : public ::testing::Test {
protected:
	const size_t size1{ 24 };
	const size_t size2{ 32 };
	std::vector<std::reference_wrapper<DataHandlers::DataHandler>> vect;
	Instructions::Set set;
	Environment* e;
	Program::Program* p;

	virtual void SetUp() {
		vect.push_back(*(new DataHandlers::PrimitiveTypeArray<double>((unsigned int)size1)));
		vect.push_back(*(new DataHandlers::PrimitiveTypeArray<int>((unsigned int)size2)));

		set.add(*(new Instructions::AddPrimitiveType<float>()));
		set.add(*(new Instructions::MultByConstParam<double, float>()));

		e = new Environment(set, vect, 8);
		p = new Program::Program(*e);
	}

	virtual void TearDown() {
		delete p;
		delete e;
		delete (&(vect.at(0).get()));
		delete (&(vect.at(1).get()));
		delete (&set.getInstruction(0));
		delete (&set.getInstruction(1));
	}
};

TEST_F(ArchiveTest, ConstructorDestructor) {
	Archive* a;
	ASSERT_NO_THROW(a = new Archive();) << "Default construction of an Archive failed";

	ASSERT_NO_THROW(delete a;) << "Destruction of an empty Archive failed.";
}

TEST_F(ArchiveTest, CombineHash) {
	size_t hash;

	ASSERT_NO_THROW(hash = Archive::getCombinedHash(vect)) << "Combination of several DataHandler hash failed.";

	// change data in one dataHandler
	DataHandlers::PrimitiveTypeArray<int>& d = (DataHandlers::PrimitiveTypeArray<int>&)vect.at(1).get();
	d.setDataAt(typeid(PrimitiveType<int>), 2, PrimitiveType<int>(1337));

	// Compare hashes.
	ASSERT_NE(Archive::getCombinedHash(vect), hash);
}

TEST_F(ArchiveTest, AddRecordingTests) {
	// For these test, force archivingProbability to 1
	Archive archive(3, 1.0);

	// Add a fictive recording
	ASSERT_NO_THROW(archive.addRecording(p, vect, 1.3)) << "Adding a recording to the empty archive failed.";

	ASSERT_EQ(archive.getNbRecordings(), 1) << "Number or recordings in the archive is incorrect.";
	ASSERT_EQ(archive.getNbDataHandlers(), 1) << "Number or dataHandlers copied in the archive is incorrect.";

	// Add other recordings with the same DataHandlers
	Program::Program p2(*e);
	ASSERT_NO_THROW(archive.addRecording(&p2, vect, 0.3)) << "Adding a recording to the non-empty archive failed.";
	ASSERT_EQ(archive.getNbRecordings(), 2) << "Number or recordings in the archive is incorrect.";
	ASSERT_EQ(archive.getNbDataHandlers(), 1) << "Number or dataHandlers copied in the archive is incorrect.";

	// Add another recording with a new environment
	// change data in one dataHandler
	DataHandlers::PrimitiveTypeArray<int>& d = (DataHandlers::PrimitiveTypeArray<int>&)vect.at(1).get();
	d.setDataAt(typeid(PrimitiveType<int>), 2, PrimitiveType<int>(1337));
	ASSERT_NO_THROW(archive.addRecording(p, vect, 0.2)) << "Adding a recording to the non-empty archive failed.";
	ASSERT_EQ(archive.getNbRecordings(), 3) << "Number or recordings in the archive is incorrect.";
	ASSERT_EQ(archive.getNbDataHandlers(), 2) << "Number or dataHandlers copied in the archive is incorrect.";

	// Reach the archive size limit.
	ASSERT_NO_THROW(archive.addRecording(&p2, vect, 0.5)) << "Adding a recording to the full archive failed.";
	ASSERT_EQ(archive.getNbRecordings(), 3) << "Number or recordings in the archive is incorrect.";
	ASSERT_EQ(archive.getNbDataHandlers(), 2) << "Number or dataHandlers copied in the archive is incorrect.";

	// Evict a recording again, and its DataHandler copy.
	Program::Program p3(*e);
	ASSERT_NO_THROW(archive.addRecording(&p3, vect, 1.5)) << "Adding a recording to the full archive failed.";
	ASSERT_EQ(archive.getNbRecordings(), 3) << "Number or recordings in the archive is incorrect.";
	ASSERT_EQ(archive.getNbDataHandlers(), 1) << "Number or dataHandlers copied in the archive is incorrect.";
}

TEST_F(ArchiveTest, AddRecordingWithProbabilityTests) {
	// For these test, force archivingProbability to 0.5
	Archive archive(10, 0.5);

	// Use a known seed
	Mutator::RNG::setSeed(0);

	// Add a few fictive recording
	for (int i = 0; i < 10; i++) {
		((DataHandlers::PrimitiveTypeArray<int>&)(vect.at(1).get())).setDataAt(typeid(PrimitiveType<int>), 0, i);
		ASSERT_NO_THROW(archive.addRecording(p, vect, (double)i)) << "Adding a recording to the archive failed.";
	}
	ASSERT_EQ(archive.getNbRecordings(), 6) << "Number or recordings in the archive is incorrect with a known seed.";
}

TEST_F(ArchiveTest, IsUnique) {
	Archive archive(4);

	// Add a few fictive recordings
	archive.addRecording(p, vect, 1.0);
	archive.addRecording(p, vect, 1.5);
	DataHandlers::PrimitiveTypeArray<int>& d = (DataHandlers::PrimitiveTypeArray<int>&)vect.at(1).get();
	d.setDataAt(typeid(PrimitiveType<int>), 2, PrimitiveType<int>(1337));
	archive.addRecording(p, vect, 2.0);
	archive.addRecording(p, vect, 2.3);

	ASSERT_TRUE(archive.isRecordingExisting(archive.getCombinedHash(vect), p)) << "Values corresponding to a recording within the Archive is not detected as such.";
	d.setDataAt(typeid(PrimitiveType<int>), 2, PrimitiveType<int>(42));
	ASSERT_FALSE(archive.isRecordingExisting(archive.getCombinedHash(vect), p)) << "Values corresponding to a recording not within the Archive is not detected as such.";
}

TEST_F(ArchiveTest, areProgramResultsUnique) {
	Archive archive(4);
	size_t hash1 = archive.getCombinedHash(vect);
	DataHandlers::PrimitiveTypeArray<int>& d = (DataHandlers::PrimitiveTypeArray<int>&)vect.at(1).get();

	// Add a few fictive recordings with p
	archive.addRecording(p, vect, 1.0);
	d.setDataAt(typeid(PrimitiveType<int>), 2, PrimitiveType<int>(1337));
	size_t hash2 = archive.getCombinedHash(vect);
	archive.addRecording(p, vect, 1.5);

	// Add a few fictive recordings with p2
	Program::Program p2(*e);
	archive.addRecording(&p2, vect, 2.0);
	d.setDataAt(typeid(PrimitiveType<int>), 2, PrimitiveType<int>(1337));
	size_t hash3 = archive.getCombinedHash(vect);
	archive.addRecording(&p2, vect, 2.5);

	// results are entirely different
	ASSERT_TRUE(archive.areProgramResultsUnique({ {hash1, 3.0}, {hash2, 3.5} })) << "Unique fake program bidding behavior not detected as such.";
	ASSERT_FALSE(archive.areProgramResultsUnique({ {hash1, 0.0}, {hash2, 2.0}, {hash3, 2.5} })) << "Equal fake program bidding behavior not detected as such.";
	ASSERT_FALSE(archive.areProgramResultsUnique({ {hash1, 1.2}, {hash2, 1.3}, {hash3, 3.5} }, 0.21)) << "Within margin fake program bidding behavior not detected as such.";

}

TEST_F(ArchiveTest, DataHandlersAccessors) {
	Archive archive(4);

	// Add a few fictive recordings
	archive.addRecording(p, vect, 1.0);
	archive.addRecording(p, vect, 1.5);
	DataHandlers::PrimitiveTypeArray<int>& d = (DataHandlers::PrimitiveTypeArray<int>&)vect.at(1).get();
	d.setDataAt(typeid(PrimitiveType<int>), 2, PrimitiveType<int>(1337));
	archive.addRecording(p, vect, 2.0);
	archive.addRecording(p, vect, 2.3);

	ASSERT_TRUE(archive.hasDataHandlers(archive.getCombinedHash(vect))) << "Data handler should be detected as present within the archive.";
	d.setDataAt(typeid(PrimitiveType<int>), 2, PrimitiveType<int>(666));
	ASSERT_FALSE(archive.hasDataHandlers(archive.getCombinedHash(vect))) << "Data handler should be detected as not present within the archive.";
	auto dhandlers = archive.getDataHandlers();
	ASSERT_EQ(dhandlers.size(), 2);
}

TEST_F(ArchiveTest, Clear) {
	Archive archive(4);

	ASSERT_NO_THROW(archive.clear()) << "Clearing an empty archive should not fail.";

	// Add a few fictive recordings
	archive.addRecording(p, vect, 1.0);
	archive.addRecording(p, vect, 1.5);

	ASSERT_NO_THROW(archive.clear()) << "Clearing a non-empty archive should not fail.";
	ASSERT_EQ(archive.getNbRecordings(), 0) << "Number or recordings in the archive is incorrect.";
	ASSERT_EQ(archive.getNbDataHandlers(), 0) << "Number or dataHandlers copied in the archive is incorrect.";
}