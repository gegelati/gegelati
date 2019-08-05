#include <gtest/gtest.h>

#include "dataHandlers/dataHandler.h"
#include "dataHandlers/primitiveTypeArray.h"

TEST(DataHandlers, Constructor) {
	DataHandlers::DataHandler* d;
	ASSERT_NO_THROW({
		d = new DataHandlers::PrimitiveTypeArray<double>();
		}) << "Call to PrimitiveTypeArray constructor failed.";
	delete d;
}

TEST(DataHandlers, PrimitiveDataArrayCanProvide) {
	DataHandlers::DataHandler * d = new DataHandlers::PrimitiveTypeArray<double>();

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