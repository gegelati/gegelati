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

	virtual void SetUp() {
		vect.push_back(*(new DataHandlers::PrimitiveTypeArray<double>((unsigned int)size1)));
		vect.push_back(*(new DataHandlers::PrimitiveTypeArray<int>((unsigned int)size2)));

		set.add(*(new Instructions::AddPrimitiveType<float>()));
		set.add(*(new Instructions::MultByConstParam<double, float>()));

		e = new Environment(set, vect, 8);
	}

	virtual void TearDown() {
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