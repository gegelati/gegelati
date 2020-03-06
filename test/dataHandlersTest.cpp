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

TEST(DataHandlersTest, PrimitiveDataArrayCanProvideTemplateType) {
	Data::DataHandler* d = new Data::PrimitiveTypeArray<double>(4);

	ASSERT_TRUE(d->canHandle(typeid(double))) << "PrimitiveTypeArray<double>() wrongfully say it can not provide double data.";
	ASSERT_FALSE(d->canHandle(typeid(int))) << "PrimitiveTypeArray<double>() wrongfully say it can provide int data.";
	ASSERT_FALSE(d->canHandle(typeid(Data::UntypedSharedPtr))) << "PrimitiveTypeArray<double>() wrongfully say it can provide UntypedSharedPtr data.";
	delete d;
}

TEST(DataHandlersTest, PrimitiveDataArrayAddressSpaceTemplateType) {
	Data::DataHandler* d = new Data::PrimitiveTypeArray<long>(64); // Array of 64 long
	ASSERT_EQ(d->getAddressSpace(typeid(long)), 64) << "Address space size for type long in PrimitiveTypeArray<long>(64) is not 64";
	ASSERT_EQ(d->getAddressSpace(typeid(int)), 0) << "Address space size for type int in PrimitiveTypeArray<long>(64) is not 0";

	delete d;
}

TEST(DataHandlersTest, PrimitiveDataArrayCanProvideArray) {
	Data::DataHandler* d = new Data::PrimitiveTypeArray<double>(4);

	ASSERT_TRUE(d->canHandle(typeid(double[2]))) << "PrimitiveTypeArray<double>(4) wrongfully say it can not provide std::array<double, 2> data.";
	ASSERT_FALSE(d->canHandle(typeid(double[5]))) << "PrimitiveTypeArray<double>(4) wrongfully say it can provide std::array<double, 5> data.";
	ASSERT_FALSE(d->canHandle(typeid(int[3]))) << "PrimitiveTypeArray<double>(4) wrongfully say it can provide std::array<int, 3> data.";
	delete d;
}

TEST(DataHandlersTest, PrimitiveDataArrayAddressSpaceArray) {
	Data::DataHandler* d = new Data::PrimitiveTypeArray<long>(64); // Array of 64 long
	ASSERT_EQ(d->getAddressSpace(typeid(long[50])), 15) << "Address space size for type std::array<long, 50> in PrimitiveTypeArray<long>(64) is not 15";
	ASSERT_EQ(d->getAddressSpace(typeid(double[50])), 0) << "Address space size for type std::array<double, 50> in PrimitiveTypeArray<long>(64) is not 0";

	delete d;
}

TEST(DataHandlersTest, PrimitiveDataArrayLargestAddressSpace) {
	Data::DataHandler* d = new Data::PrimitiveTypeArray<float>(20); // Array of 64 long
	ASSERT_EQ(d->getLargestAddressSpace(), 20) << "Largest address space size for type in PrimitiveTypeArray<float>(20) is not 20 as expected.";

	delete d;
}

TEST(DataHandlersTest, PrimitiveDataArrayGetDataAtNativeType) {
	const size_t size{ 32 };
	Data::DataHandler* d = new Data::PrimitiveTypeArray<float>(size);

	d->resetData();
	for (int i = 0; i < size; i++) {
		const float a = *(d->getDataAt(typeid(float), i).getSharedPointer<const float>());
		ASSERT_EQ((float)a, 0.0f) << "Data at valid address and type can not be accessed.";
	}

	ASSERT_THROW(d->getDataAt(typeid(float), size), std::out_of_range) << "Address exceeding the addressSpace should cause an exception.";
	ASSERT_THROW(d->getDataAt(typeid(double), 0), std::invalid_argument) << "Requesting a non-handled type, even at a valid location, should cause an exception.";

	delete d;
}

TEST(DataHandlersTest, PrimitiveDataArrayGetDataAtArray) {
	const size_t size{ 8 };
	const size_t sizeArray = 3;
	Data::PrimitiveTypeArray<int>* d = new Data::PrimitiveTypeArray<int>(size);

	// Fill the array
	for (auto idx = 0; idx < size; idx++) {
		d->setDataAt(typeid(int), idx, idx);
	}

	// Get data as arrays
	for (int i = 0; i < size - sizeArray + 1; i++) {
		std::shared_ptr<const int[]> sptr = d->getDataAt(typeid(int[sizeArray]), i).getSharedPointer<const int[]>();
		const int* a = (sptr.get());
		ASSERT_NE(a, nullptr) << "Retrieved data is a null_ptr";
		for (int idx = 0; idx < sizeArray; idx++) {
			ASSERT_EQ(a[idx], i + idx) << "Value given in the array do not correspond to the one stored in the array.";
		}
	}

	ASSERT_THROW(d->getDataAt(typeid(int[sizeArray]), size - 1), std::out_of_range) << "Address exceeding the addressSpace should cause an exception.";
	ASSERT_THROW(d->getDataAt(typeid(long[sizeArray]), 0), std::invalid_argument) << "Requesting a non-handled type, even at a valid location, should cause /an /exception.";

	delete d;
}

TEST(DataHandlersTest, PrimitiveDataArrayGetAddressesAccessed) {
	Data::PrimitiveTypeArray<float>d(100);

	std::vector<size_t> accessedAddresses;
	ASSERT_NO_THROW(accessedAddresses = d.getAddressesAccessed(typeid(float), 25)) << "No exception should be thrown with a valid type at a valid address.";
	ASSERT_EQ(accessedAddresses.size(), 1) << "Only one address should be accessed with native type at a valid address.";
	ASSERT_EQ(accessedAddresses.at(0), 25) << "Address accessed does not correspond to the requested one.";

	ASSERT_NO_THROW(accessedAddresses = d.getAddressesAccessed(typeid(float[3]), 50)) << "No exception should be thrown with a valid type at a valid address.";
	ASSERT_EQ(accessedAddresses.size(), 3) << "Only one address should be accessed with native type at a valid address.";
	for (int i = 0; i < 3; i++) {
		ASSERT_EQ(accessedAddresses.at(i), 50 + i) << "Address accessed does not correspond to the requested one.";
	}

	ASSERT_NO_THROW(accessedAddresses = d.getAddressesAccessed(typeid(double), 75)) << "No exception should be thrown with an invalid type at a valid address.";
	ASSERT_EQ(accessedAddresses.size(), 0) << "No address should be accessed with and invalid type at a valid address.";


	ASSERT_NO_THROW(accessedAddresses = d.getAddressesAccessed(typeid(float[25]), 90)) << "No exception should be thrown with a valid type at an invalid address.";
	ASSERT_EQ(accessedAddresses.size(), 0) << "No address should be accessed with and valid type at an invalid address.";
}

TEST(DataHandlersTest, PrimitiveDataArraySetDataAt) {
	const size_t size{ 8 };
	const size_t address{ 3 };
	const double doubleValue{ 42.0 };
	Data::PrimitiveTypeArray<double>* d = new Data::PrimitiveTypeArray<double>(size);

	d->resetData();
	double value(doubleValue);
	ASSERT_NO_THROW(d->setDataAt(typeid(value), address, value)) << "Setting data with valid Address and type failed.";

	// Check that data was indeed updated.
	ASSERT_EQ((double)*(d->getDataAt(typeid(double), address).getSharedPointer<const double>()), doubleValue) << "Previously set data did not persist.";

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
	d.setDataAt(typeid(double), address, doubleValue);
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
	d.setDataAt(typeid(double), address, doubleValue);
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