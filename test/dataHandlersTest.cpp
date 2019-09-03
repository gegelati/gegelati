#include <gtest/gtest.h>

#include "dataHandlers/dataHandler.h"
#include "dataHandlers/primitiveTypeArray.h"

TEST(DataHandlersTest, Constructor) {
	ASSERT_NO_THROW({
		DataHandlers::DataHandler * d = new DataHandlers::PrimitiveTypeArray<double>();
		delete d;
		}) << "Call to PrimitiveTypeArray constructor failed.";
}

TEST(DataHandlersTest, PrimitiveDataArrayCanProvide) {
	DataHandlers::DataHandler* d = new DataHandlers::PrimitiveTypeArray<double>();

	ASSERT_TRUE(d->canHandle(typeid(PrimitiveType<double>))) << "PrimitiveTypeArray<double>() wrongfully say it can not provide PrimitiveType<double> data.";
	ASSERT_FALSE(d->canHandle(typeid(PrimitiveType<int>))) << "PrimitiveTypeArray<double>() wrongfully say it can provide PrimitiveType<int> data.";
	ASSERT_FALSE(d->canHandle(typeid(SupportedType))) << "PrimitiveTypeArray<double>() wrongfully say it can provide SupportedType data.";

	delete d;
}

TEST(DataHandlersTest, PrimitiveDataArrayGetHandledTypes) {
	DataHandlers::DataHandler* d = new DataHandlers::PrimitiveTypeArray<int>();

	auto vect = d->getHandledTypes();
	ASSERT_EQ(vect.size(), 1) << "Size of data type set handled by PrimitiveTypeArray<double> is incorrect.";
	ASSERT_EQ(std::count(vect.begin(), vect.end(), typeid(PrimitiveType<int>)), 1) << "Vector of handled types returned by PrimitiveTypeArray<int> does not contain expectesd.";

	delete d;
}

TEST(DataHandlersTest, PrimitiveDataArrayAddressSpace) {
	DataHandlers::DataHandler* d = new DataHandlers::PrimitiveTypeArray<long>(64); // Array of 64 long
	ASSERT_EQ(d->getAddressSpace(typeid(PrimitiveType<long>)), 64) << "Address space size for type PrimitiveType<long> in PrimitiveTypeArray<long>(64) is not 64";
	ASSERT_EQ(d->getAddressSpace(typeid(PrimitiveType<int>)), 0) << "Address space size for type PrimitiveType<int> in PrimitiveTypeArray<long>(64) is not 0";

	delete d;
}

TEST(DataHandlersTest, PrimitiveDataArrayLargestAddressSpace) {
	DataHandlers::DataHandler* d = new DataHandlers::PrimitiveTypeArray<float>(20); // Array of 64 long
	ASSERT_EQ(d->getLargestAddressSpace(), 20) << "Largest address space size for type in PrimitiveTypeArray<float>(20) is not 20 as expected.";

	delete d;
}

TEST(DataHandlersTest, PrimitiveDataArrayGetDataAt) {
	const size_t size{ 32 };
	DataHandlers::DataHandler* d = new DataHandlers::PrimitiveTypeArray<float>(size);

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
	DataHandlers::PrimitiveTypeArray<double>* d = new DataHandlers::PrimitiveTypeArray<double>(size);

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

	DataHandlers::PrimitiveTypeArray<double> d(size);

	// Get hash
	size_t hash = 0;
	ASSERT_NO_THROW(hash = d.updateHash());
	// change the content of the array
	d.setDataAt(typeid(PrimitiveType<double>), address, PrimitiveType<double>(doubleValue));
	ASSERT_NO_THROW(d.updateHash());
	ASSERT_NE(hash, d.getHash());
}