#include <gtest/gtest.h>

#include "dataHandlers/dataHandler.h"
#include "dataHandlers/primitiveTypeArray.h"

TEST(DataHandlers, Constructor) {
	ASSERT_NO_THROW({
		DataHandlers::DataHandler *d = new DataHandlers::PrimitiveTypeArray<double>();
		delete d;
		}) << "Call to PrimitiveTypeArray constructor failed.";
}

TEST(DataHandlers, PrimitiveDataArrayCanProvide) {
	DataHandlers::DataHandler* d = new DataHandlers::PrimitiveTypeArray<double>();

	ASSERT_TRUE(d->canHandle(typeid(PrimitiveType<double>))) << "PrimitiveTypeArray<double>() wrongfully say it can not provide PrimitiveType<double> data.";
	ASSERT_FALSE(d->canHandle(typeid(PrimitiveType<int>))) << "PrimitiveTypeArray<double>() wrongfully say it can provide PrimitiveType<int> data.";
	ASSERT_FALSE(d->canHandle(typeid(SupportedType))) << "PrimitiveTypeArray<double>() wrongfully say it can provide SupportedType data.";

	delete d;
}

TEST(DataHandlers, PrimitiveDataArrayGetHandledTypes) {
	DataHandlers::DataHandler* d = new DataHandlers::PrimitiveTypeArray<int>();


	auto vect = d->getHandledTypes();
	ASSERT_EQ(vect.size(), 1) << "Size of data type set handled by PrimitiveTypeArray<double> is incorrect";
	ASSERT_EQ(std::count(vect.begin(), vect.end(), typeid(PrimitiveType<int>)), 1) << "Vector of handled types returned by PrimitiveTypeArray<int> contains the wrond type.";

	delete d;
}

TEST(DataHandlers, PrimitiveDataArrayAddressSpace) {
	DataHandlers::DataHandler* d = new DataHandlers::PrimitiveTypeArray<long>(64); // Array of 64 long
	ASSERT_EQ(d->getAddressSpace(typeid(PrimitiveType<long>)), 64) << "Address space size for type PrimitiveType<long> in PrimitiveTypeArray<long>(64) is not 64" ;
	ASSERT_EQ(d->getAddressSpace(typeid(PrimitiveType<int>)), 0) << "Address space size for type PrimitiveType<int> in PrimitiveTypeArray<long>(64) is not 0";

	delete d;
}

TEST(DataHandlers, PrimitiveDataArrayGetDataAt) {
	const size_t size{ 32 };
	DataHandlers::DataHandler* d = new DataHandlers::PrimitiveTypeArray<float>(size);

	d->resetData();
	for (int i = 0; i < size; i++) {
		ASSERT_EQ(d->getDataAt(typeid(PrimitiveType<float>), i), PrimitiveType<float>(0)) << "Data at valid address and type can not be accessed.";
	}

	ASSERT_THROW(d->getDataAt(typeid(PrimitiveType<float>), size), std::out_of_range) << "Address exceeding the addressSpace should cause an exception.";
	ASSERT_THROW(d->getDataAt(typeid(PrimitiveType<double>), 0), std::invalid_argument) << "Requesting a non-handled type, even at a valid location, should cause an exception.";

	delete d;
}