#include <gtest/gtest.h>
#include <vector>

#include "dataHandlers/dataHandler.h"
#include "dataHandlers/primitiveTypeArray.h"
#include "instructions/set.h"
#include "instructions/addPrimitiveType.h"
#include "environment.h"

TEST(Environment, Constructor){
	const size_t size1{ 24 };
	const size_t size2{ 32 };
	std::vector<std::reference_wrapper<DataHandlers::DataHandler>> vect;
	Instructions::Set set;

	DataHandlers::PrimitiveTypeArray<double> d1 (size1);
	DataHandlers::PrimitiveTypeArray<int> d2(size2);

	vect.push_back(d1);
	vect.push_back(d2);

	ASSERT_NO_THROW({
		Environment e(set,vect,8);
	});
}