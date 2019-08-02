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

	ASSERT_TRUE(d->canProvide(typeid(PrimitiveType<double>))) << "PrimitiveTypeArray<double>() wrongfully say it can not provide PrimitiveType<double> data.";
	ASSERT_FALSE(d->canProvide(typeid(PrimitiveType<int>))) << "PrimitiveTypeArray<double>() wrongfully say it can provide PrimitiveType<int> data.";
	ASSERT_FALSE(d->canProvide(typeid(SupportedType))) << "PrimitiveTypeArray<double>() wrongfully say it can provide SupportedType data.";
}