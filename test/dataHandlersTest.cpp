#include <gtest/gtest.h>

#include "data/dataHandler.h"
#include "data/primitiveTypeArray.h"

TEST(DataHandlersTest, Constructor) {
	ASSERT_NO_THROW({
		Data::DataHandler * d = new Data::PrimitiveTypeArray<double>();
		delete d;
		}) << "Call to PrimitiveTypeArray constructor failed.";
}

TEST(DataHandlersTest, ID) {
	Data::PrimitiveTypeArray<double> d0;
	Data::PrimitiveTypeArray<int> d1;

	ASSERT_NE(d0.getId(), d1.getId()) << "Id of two DataHandlers created one after the other should not be equal.";
}

TEST(DataHandlersTest, PrimitiveDataArrayCanProvide) {
	Data::DataHandler* d = new Data::PrimitiveTypeArray<double>();

	ASSERT_TRUE(d->canHandle(typeid(PrimitiveType<double>))) << "PrimitiveTypeArray<double>() wrongfully say it can not provide PrimitiveType<double> data.";
	ASSERT_FALSE(d->canHandle(typeid(PrimitiveType<int>))) << "PrimitiveTypeArray<double>() wrongfully say it can provide PrimitiveType<int> data.";
	ASSERT_FALSE(d->canHandle(typeid(SupportedType))) << "PrimitiveTypeArray<double>() wrongfully say it can provide SupportedType data.";

	delete d;
}

TEST(DataHandlersTest, PrimitiveDataArrayGetHandledTypes) {
	Data::DataHandler* d = new Data::PrimitiveTypeArray<int>();

	auto vect = d->getHandledTypes();
	ASSERT_EQ(vect.size(), 1) << "Size of data type set handled by PrimitiveTypeArray<double> is incorrect.";
	ASSERT_EQ(std::count(vect.begin(), vect.end(), typeid(PrimitiveType<int>)), 1) << "Vector of handled types returned by PrimitiveTypeArray<int> does not contain expectesd.";

	delete d;
}

TEST(DataHandlersTest, PrimitiveDataArrayAddressSpace) {
	Data::DataHandler* d = new Data::PrimitiveTypeArray<long>(64); // Array of 64 long
	ASSERT_EQ(d->getAddressSpace(typeid(PrimitiveType<long>)), 64) << "Address space size for type PrimitiveType<long> in PrimitiveTypeArray<long>(64) is not 64";
	ASSERT_EQ(d->getAddressSpace(typeid(PrimitiveType<int>)), 0) << "Address space size for type PrimitiveType<int> in PrimitiveTypeArray<long>(64) is not 0";

	delete d;
}

TEST(DataHandlersTest, PrimitiveDataArrayLargestAddressSpace) {
	Data::DataHandler* d = new Data::PrimitiveTypeArray<float>(20); // Array of 64 long
	ASSERT_EQ(d->getLargestAddressSpace(), 20) << "Largest address space size for type in PrimitiveTypeArray<float>(20) is not 20 as expected.";

	delete d;
}

TEST(DataHandlersTest, PrimitiveDataArrayGetDataAt) {
	const size_t size{ 32 };
	Data::DataHandler* d = new Data::PrimitiveTypeArray<float>(size);

	d->resetData();
	for (int i = 0; i < size; i++) {
		const PrimitiveType<float>& a = (const PrimitiveType<float>&)d->getDataAt(typeid(PrimitiveType<float>), i);
		ASSERT_EQ((float)a, 0.0f) << "Data at valid address and type can not be accessed.";
	}

	ASSERT_THROW(d->getDataAt(typeid(PrimitiveType<float>), size), std::out_of_range) << "Address exceeding the addressSpace should cause an exception.";
	ASSERT_THROW(d->getDataAt(typeid(PrimitiveType<double>), 0), std::invalid_argument) << "Requesting a non-handled type, even at a valid location, should cause an exception.";

	delete d;
}

TEST(DataHandlersTest, PrimitiveDataArraySetDataAt) {
	const size_t size{ 8 };
	const size_t address{ 3 };
	const double doubleValue{ 42.0 };
	Data::PrimitiveTypeArray<double>* d = new Data::PrimitiveTypeArray<double>(size);

	d->resetData();
	PrimitiveType<double> value(doubleValue);
	ASSERT_NO_THROW(d->setDataAt(typeid(value), address, value)) << "Setting data with valid Address and type failed.";

	// Check that data was indeed updated.
	ASSERT_EQ((double)((const PrimitiveType<double>&)d->getDataAt(typeid(PrimitiveType<double>), address)), doubleValue) << "Previously set data did not persist.";

	delete d;
}


TEST(DataHandlersTest, PrimitiveDataArrayHash) {
	// Create a DataHandler
	const size_t size{ 8 };
	const size_t address{ 3 };
	const double doubleValue{ 42.0 };

	Data::PrimitiveTypeArray<double> d(size);

	// Get hash
	size_t hash = 0;
	ASSERT_NO_THROW(hash = d.getHash());
	// change the content of the array
	d.setDataAt(typeid(PrimitiveType<double>), address, PrimitiveType<double>(doubleValue));
	ASSERT_NE(hash, d.getHash());
}

TEST(DataHandlersTest, PrimitiveDataArrayClone) {
	// Create a DataHandler
	const size_t size{ 8 };
	const size_t address{ 3 };
	const double doubleValue{ 42.0 };

	// create a first one to increase the DataHandler::count
	Data::PrimitiveTypeArray<int> d0(12);
	Data::PrimitiveTypeArray<double> d(size);
	// change the content of the array
	d.setDataAt(typeid(PrimitiveType<double>), address, PrimitiveType<double>(doubleValue));
	// Hash was voluntarily not computed before clone.

	// Create a clone
	Data::DataHandler* dClone = NULL;
	ASSERT_NO_THROW(dClone = d.clone();) << "Cloning a PrimitiTypeArray<double> failed.";

	// Extra if to remove warnings on further use of dClone.
	if (dClone == NULL) FAIL() << "Cloning of DataHandler returned a NULL Pointer.";

	// Check ID
	ASSERT_EQ(dClone->getId(), d.getId()) << "Cloned and original dataHandler do not have the same ID as expected.";
	// Check the polymorphic type.
	ASSERT_EQ(typeid(*dClone), typeid(Data::PrimitiveTypeArray<double>)) << "Type of clone DataHandler differes from the original one.";
	// Compute the hashes
	ASSERT_EQ(dClone->getHash(), d.getHash()) << "Hash of clone and original DataHandler differ.";

	// Change data in the original to make sure the two dHandlers are decoupled.
	size_t hash = dClone->getHash();
	d.resetData();
	ASSERT_NE(dClone->getHash(), d.getHash()) << "Hash of clone and original DataHandler should differ after modification of data in the original.";
	ASSERT_EQ(dClone->getHash(), hash) << "Hash of the clone dataHandler should remain unchanged after modification of data within the original DataHandler.";
}