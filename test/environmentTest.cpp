#include <gtest/gtest.h>
#include <vector>

#include "dataHandlers/dataHandler.h"
#include "dataHandlers/primitiveTypeArray.h"
#include "instructions/set.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/multByConstParam.h"
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

TEST(Environment, AttributeAccessors) {
	const size_t size1{ 24 };
	const size_t size2{ 32 };
	std::vector<std::reference_wrapper<DataHandlers::DataHandler>> vect;
	Instructions::Set set;

	DataHandlers::PrimitiveTypeArray<double> d1(size1);
	DataHandlers::PrimitiveTypeArray<int> d2(size2);

	Instructions::AddPrimitiveType<float> iAdd; // Two operands, No Parameter 
	Instructions::MultByConstParam<double, float> iMult; // One operand, One parameter

	set.add(iAdd);
	set.add(iMult);

	vect.push_back(d1);
	vect.push_back(d2);

	Environment e(set,vect,8);

	ASSERT_EQ(e.getNbRegisters(), 8) << "Number of registers of the Environment does not correspond to the one given during construction.";
	ASSERT_EQ(e.getNbInstructions(), 2) << "Number of instructions of the Environment does not correspond to the content of the set given during construction."; 
	ASSERT_EQ(e.getMaxNbOperands(), 2) << "Maximum number of operands of the Environment does not correspond to the instruction set given during construction.";
	ASSERT_EQ(e.getMaxNbParameters(), 1) << "Maximum number of parameters of the Environment does not correspond to the instruction set given during construction.";
	ASSERT_EQ(e.getNbDataSources(), 3) << "Number of data sources does not correspond to the number of DataHandler (+1 for registers) given during construction.";
	ASSERT_EQ(e.getLargestAddressSpace(), size2) << "Largest address space of the Environment does not corresponds to the dataHandlers or registers given during construction.";
}