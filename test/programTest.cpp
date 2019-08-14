#include <gtest/gtest.h>
#include <vector>

#include "instructions/set.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/multByConstParam.h"
#include "dataHandlers/dataHandler.h"
#include "dataHandlers/primitiveTypeArray.h"
#include "program/program.h"

class ProgramTest : public ::testing::Test {
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

TEST_F(ProgramTest, Constructor) {
	Program::Program* p;
	ASSERT_NO_THROW({
		p = new Program::Program(*e); }) << "Something went wrong when constructing a Program with a valid Environment.";

	ASSERT_NO_THROW({
		delete p;
		}) << "Something went wrong when destructing a Program with a valid Environment and empty lines.";

	Environment e2(set, {}, 8); // empty dataHandler should be a problem.
	ASSERT_THROW({
		p = new Program::Program(e2); }, std::domain_error) << "Something went unexpectedly right when constructing a Program with an invalid Environment.";

}

TEST_F(ProgramTest, AddEmptyLineAndDestruction) {
	Program::Program* p = new Program::Program(*e);
	ASSERT_NO_THROW(p->addNewLine();) << "Inserting a single empty line in an empty program should not be an issue at insertion.";

	ASSERT_NO_THROW(delete p;) << "Destructing a non empty program should not be an issue.";
}

TEST_F(ProgramTest, basicAccessor) {
	Program::Program p(*e);
	// Expected answer:
	// n = 8
	// i = 2
	// p = 1
	// nbSrc = 3
	// largestAddressSpace = 32
	// m = 2
	// ceil(log2(n)) + ceil(log2(i)) + m * (ceil(log2(nb_{ src })) + ceil(log2(largestAddressSpace)) + p * sizeof(Param) * 8
	// ceil(log2(8)) + ceil(log2(2)) + 2 * (ceil(log2(3)) + ceil(log2(32)) + 1 * 4 * 8
	//            3  +             1 + 2 * (            2 +             5) +  32 
	ASSERT_EQ(p.getLineSize(), 50) << "Program Line size is incorrect. Expected value is 50 for (n=8,i=2,p=1,nbSrc=3,largAddrSpace=32,m=2). ";
}

TEST_F(ProgramTest, getProgramNbLines) {
	Program::Program p(*e);
	ASSERT_EQ(p.getNbLines(), 0) << "Empty program nb lines should be 0.";
	p.addNewLine();
	ASSERT_EQ(p.getNbLines(), 1) << "A single line was just added to the Program.";
}