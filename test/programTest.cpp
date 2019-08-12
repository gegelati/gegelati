#include <gtest/gtest.h>
#include <vector>

#include "instructions/set.h"
#include "dataHandlers/dataHandler.h"
#include "dataHandlers/primitiveTypeArray.h"
#include "program.h"

TEST(Program, Constructor) {
	// Create an Environment for the Program (should be moved to Fixture)
	const size_t size1{ 24 };
	const size_t size2{ 32 };
	std::vector<std::reference_wrapper<DataHandlers::DataHandler>> vect;
	Instructions::Set set;

	DataHandlers::PrimitiveTypeArray<double> d1(size1);
	DataHandlers::PrimitiveTypeArray<int> d2(size2);

	vect.push_back(d1);
	vect.push_back(d2);

	Environment e(set, vect, 8);

	Program* p;
	ASSERT_NO_THROW({
		p = new Program(e); }) << "Something went wrong when constructing a Program with a valid Environment.";

	ASSERT_NO_THROW({
		delete p;
		}) << "Something went wrong when destructing a Program with a valid Environment and empty lines.";
}