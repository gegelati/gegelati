#include <gtest/gtest.h>

#include "instructions/set.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/multByConstParam.h"
#include "dataHandlers/dataHandler.h"
#include "dataHandlers/primitiveTypeArray.h"
#include "program/program.h"
#include "program/line.h"

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

	vect.at(0).get().updateHash();
	vect.at(1).get().updateHash();
	ASSERT_NO_THROW(hash = Archive::getCombinedHash(vect)) << "Combination of several DataHandler hash failed.";

	// change data in one dataHandler
	DataHandlers::PrimitiveTypeArray<int>& d = (DataHandlers::PrimitiveTypeArray<int>&)vect.at(1).get();
	d.setDataAt(typeid(PrimitiveType<int>), 2, PrimitiveType<int>(1337));

	// Update both hash (only one is needed)
	vect.at(0).get().updateHash();
	vect.at(1).get().updateHash();
	// Compare hashes.
	ASSERT_NE(Archive::getCombinedHash(vect), hash);
}

TEST_F(ArchiveTest, AddRecordingTests) {
	Archive archive(3);
	vect.at(0).get().updateHash();
	vect.at(1).get().updateHash();

	// Add a fictive recording
	ASSERT_NO_THROW(archive.addRecording(p, vect, 1.3)) << "Adding a recording to the empty archive failed.";

	ASSERT_EQ(archive.getNbRecordings(), 1) << "Number or recordings in the archive is incorrect.";
	ASSERT_EQ(archive.getNbDataHandlers(), 1) << "Number or dataHandlers copied in the archive is incorrect.";

	// Add other recordings with the same DataHandlers
	ASSERT_NO_THROW(archive.addRecording(p, vect, 0.3)) << "Adding a recording to the non-empty archive failed.";
	ASSERT_EQ(archive.getNbRecordings(), 2) << "Number or recordings in the archive is incorrect.";
	ASSERT_EQ(archive.getNbDataHandlers(), 1) << "Number or dataHandlers copied in the archive is incorrect.";

	// Add another recording with a new environment
	// change data in one dataHandler
	DataHandlers::PrimitiveTypeArray<int>& d = (DataHandlers::PrimitiveTypeArray<int>&)vect.at(1).get();
	d.setDataAt(typeid(PrimitiveType<int>), 2, PrimitiveType<int>(1337));
	vect.at(1).get().updateHash();
	ASSERT_NO_THROW(archive.addRecording(p, vect, 0.2)) << "Adding a recording to the non-empty archive failed.";
	ASSERT_EQ(archive.getNbRecordings(), 3) << "Number or recordings in the archive is incorrect.";
	ASSERT_EQ(archive.getNbDataHandlers(), 2) << "Number or dataHandlers copied in the archive is incorrect.";

	// Reach the archive size limit.
	ASSERT_NO_THROW(archive.addRecording(p, vect, 0.5)) << "Adding a recording to the full archive failed.";
	ASSERT_EQ(archive.getNbRecordings(), 3) << "Number or recordings in the archive is incorrect.";
	ASSERT_EQ(archive.getNbDataHandlers(), 2) << "Number or dataHandlers copied in the archive is incorrect.";

	// Evict a recording again, and its DataHandler copy.
	ASSERT_NO_THROW(archive.addRecording(p, vect, 1.5)) << "Adding a recording to the full archive failed.";
	ASSERT_EQ(archive.getNbRecordings(), 3) << "Number or recordings in the archive is incorrect.";
	ASSERT_EQ(archive.getNbDataHandlers(), 1) << "Number or dataHandlers copied in the archive is incorrect.";
}

TEST_F(ArchiveTest, IsUnique) {
	Archive archive(4);
	vect.at(0).get().updateHash();
	vect.at(1).get().updateHash();

	// Add a few fictive recordings
	archive.addRecording(p, vect, 1.0);
	archive.addRecording(p, vect, 1.5);
	DataHandlers::PrimitiveTypeArray<int>& d = (DataHandlers::PrimitiveTypeArray<int>&)vect.at(1).get();
	d.setDataAt(typeid(PrimitiveType<int>), 2, PrimitiveType<int>(1337));
	vect.at(1).get().updateHash();
	archive.addRecording(p, vect, 2.0);
	archive.addRecording(p, vect, 2.3);

	ASSERT_FALSE(archive.isUnique(vect, 2.0)) << "Values corresponding to a recording within the Archive is not detected as such.";
	ASSERT_TRUE(archive.isUnique(vect, 2.5)) << "Values corresponding to a recording not within the Archive is not detected as such.";
	ASSERT_FALSE(archive.isUnique(vect, 2.5, 0.21)) << "Values corresponding to a recording not in the Archive, but within the error margin tau, is not detected as such.";
}

TEST_F(ArchiveTest, DataHandlersAccessors) {
	Archive archive(4);
	vect.at(0).get().updateHash();
	vect.at(1).get().updateHash();

	// Add a few fictive recordings
	archive.addRecording(p, vect, 1.0);
	archive.addRecording(p, vect, 1.5);
	DataHandlers::PrimitiveTypeArray<int>& d = (DataHandlers::PrimitiveTypeArray<int>&)vect.at(1).get();
	d.setDataAt(typeid(PrimitiveType<int>), 2, PrimitiveType<int>(1337));
	vect.at(1).get().updateHash();
	archive.addRecording(p, vect, 2.0);
	archive.addRecording(p, vect, 2.3);

	ASSERT_TRUE(archive.hasDataHandlers(vect)) << "Data handler should be detected as present within the archive.";
	d.setDataAt(typeid(PrimitiveType<int>), 2, PrimitiveType<int>(666));
	vect.at(1).get().updateHash();
	ASSERT_FALSE(archive.hasDataHandlers(vect)) << "Data handler should be detected as not present within the archive.";
	auto dhandlers = archive.getDataHandlers();
	ASSERT_EQ(dhandlers.size(), 2);
}