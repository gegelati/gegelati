/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

#include <gtest/gtest.h>
#include <vector>

#include "data/dataHandler.h"
#include "data/primitiveTypeArray.h"
#include "environment.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/lambdaInstruction.h"
#include "instructions/multByConstant.h"
#include "instructions/set.h"

TEST(EnvironmentTest, Constructor)
{
    const size_t size1{24};
    const size_t size2{32};
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
    Instructions::Set set;

    set.add(*(new Instructions::AddPrimitiveType<int>()));
    set.add(*(new Instructions::AddPrimitiveType<double>()));
    set.add(*(new Instructions::MultByConstant<int>()));

    Data::PrimitiveTypeArray<double> d1(size1);
    Data::PrimitiveTypeArray<int> d2(size2);

    vect.push_back(d1);
    vect.push_back(d2);

    ASSERT_NO_THROW({ Environment e(set, vect, 8, 5); });

    ASSERT_THROW(
        // Empty dataHandlers
        Environment e2(set, {}, 8, 0);, std::domain_error)
        << "Something went unexpectedly right when constructing an Environment "
           "with an invalid Environment.";
}

TEST(EnvironmentTest, ConstructorWithInvalidInstruction)
{
    const size_t size1{24};
    const size_t size2{32};
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
    Instructions::Set set;

    set.add(*(new Instructions::AddPrimitiveType<int>()));
    set.add(*(new Instructions::AddPrimitiveType<double>()));

    Data::PrimitiveTypeArray<double> d1(size1);
    Data::PrimitiveTypeArray<int> d2(size2);

    vect.push_back(d1);
    vect.push_back(d2);

    // Add an invalid instruction to the set to test the filtering mechanism
    set.add(*(new Instructions::AddPrimitiveType<bool>()));

    Environment* e3 = NULL;
    ASSERT_NO_THROW(e3 = new Environment(set, vect, 8, 5))
        << "Constructing an Environemnt with an invalid Instruction should not "
           "throw an exception.";
    if (e3 != NULL) {
        ASSERT_EQ(e3->getInstructionSet().getNbInstructions(), 2)
            << "After removing the non-usable instruction, only 2 instructions "
               "should remain.";
        delete e3;
    }
}

TEST(EnvironmentTest, computeLineSize)
{
    const size_t size1{24};
    const size_t size2{32};
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
    Instructions::Set set;
    Environment* e;
    vect.push_back(
        *(new Data::PrimitiveTypeArray<double>((unsigned int)size1)));
    vect.push_back(*(new Data::PrimitiveTypeArray<float>((unsigned int)size2)));

    set.add(*(new Instructions::AddPrimitiveType<float>()));
    auto minus = [](double a, double b) -> double { return a - b; };
    set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));
    e = new Environment(set, vect, 8, 5);

    // Expected answer:
    // n = 8
    // i = 2
    // nbSrc = 3
    // largestAddressSpace = 32
    // m = 2
    // ceil(log2(n)) + ceil(log2(i)) + m * (ceil(log2(nb_{ src })) +
    // ceil(log2(largestAddressSpace))=
    // ceil(log2(8)) + ceil(log2(2)) + 2 * (ceil(log2(3)) + ceil(log2(32)) + 1 *
    // 2 * 8
    //            3  +             1 + 2 * (            2 +             5) = 18
    ASSERT_EQ(e->getLineSize(), 18)
        << "Program Line size is incorrect. Expected value is 18 for "
           "(n=8,i=2,nbSrc=3,largAddrSpace=32,m=2). ";
}
TEST(EnvironmentTest, Size_tAttributeAccessors)
{
    const size_t size1{24};
    const size_t size2{32};
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
    Instructions::Set set;

    Data::PrimitiveTypeArray<double> d1(size1);
    Data::PrimitiveTypeArray<int> d2(size2);

    Instructions::AddPrimitiveType<int> iAdd; // Two operands, No Parameter
    auto minus = [](double a, double b) -> double { return a - b; };

    set.add(iAdd);
    set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));
	set.add(*(new Instructions::MultByConstant<double>()));


    vect.push_back(d1);
    vect.push_back(d2);

    Environment e(set, vect, 8, 5);

    ASSERT_EQ(e.getNbRegisters(), 8)
        << "Number of registers of the Environment does not correspond to the "
           "one given during construction.";
    ASSERT_EQ(e.getNbConstant(), 5)
        << "Number of Constants of the Environment does not correspond to the "
           "one given during construction.";
    ASSERT_EQ(e.getNbInstructions(), 3)
        << "Number of instructions of the Environment does not correspond to "
           "the content of the set given during construction.";
    ASSERT_EQ(e.getMaxNbOperands(), 2)
        << "Maximum number of operands of the Environment does not correspond "
           "to the instruction set given during construction.";
    ASSERT_EQ(e.getNbDataSources(), 4)
        << "Number of data sources does not correspond to the number of "
           "DataHandler (+1 for registers) given during construction.";
    ASSERT_EQ(e.getLargestAddressSpace(), size2)
        << "Largest address space of the Environment does not corresponds to "
           "the dataHandlers or registers given during construction.";
}

TEST(EnvironmentTest, GetFakeRegisters)
{
    const size_t size1{24};
    const size_t size2{32};
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
    Instructions::Set set;

    Data::PrimitiveTypeArray<double> d1(size1);
    Data::PrimitiveTypeArray<int> d2(size2);

    Instructions::AddPrimitiveType<int> iAdd; // Two operands, No Parameter
    auto minus = [](double a, double b) -> double { return a - b; };

    set.add(iAdd);
    set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));

    vect.push_back(d1);
    vect.push_back(d2);

    Environment e(set, vect, 8, 5);

    ASSERT_NO_THROW(e.getFakeDataSources().at(0))
        << "Couldn't access the fake registers of the environment.";
    ASSERT_EQ(
        e.getFakeDataSources().at(0).get().getAddressSpace(typeid(double)), 8)
        << "Address space for double type id is incorrect in fake registers.";
    ASSERT_EQ(typeid(e.getFakeDataSources().at(0).get()),
              typeid(Data::PrimitiveTypeArray<double>))
        << "Unexpected type for fake registers of the environment.";
}

TEST(EnvironmentTest, InstructionSetAccessor)
{
    const size_t size1{24};
    const size_t size2{32};
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
    Instructions::Set set;

    Data::PrimitiveTypeArray<double> d1(size1);
    Data::PrimitiveTypeArray<float> d2(size2);

    Instructions::AddPrimitiveType<float> iAdd; // Two operands, No Parameter
    auto minus = [](double a, double b) -> double { return a - b; };

    set.add(iAdd);
    set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));
	set.add(*(new Instructions::MultByConstant<double>));

    vect.push_back(d1);
    vect.push_back(d2);

    Environment e(set, vect, 8, 5);

    const Instructions::Set& setCpy = e.getInstructionSet();
    ASSERT_NE(&setCpy, &set)
        << "Set returned by the environment is the same as the one given to "
           "the constructor instead of a copy.";
    ASSERT_EQ(setCpy.getNbInstructions(), set.getNbInstructions())
        << "Number of instruction in the Set returned by the accessor differs "
           "from the one given at construction.";
    for (unsigned int i = 0; i < set.getNbInstructions(); i++) {
        ASSERT_EQ(&setCpy.getInstruction(i), &set.getInstruction(i))
            << "Instruction referenced in the copied Set should be identical "
               "to the ones referenced in the Set given at construction.";
    }
}

TEST(EnvironmentTest, DataSourceAccessor)
{
    const size_t size1{24};
    const size_t size2{32};
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
    Instructions::Set set;

    Data::PrimitiveTypeArray<double> d1(size1);
    Data::PrimitiveTypeArray<int> d2(size2);

    Instructions::AddPrimitiveType<int> iAdd; // Two operands, No Parameter
    auto minus = [](double a, double b) -> double { return a - b; };

    set.add(iAdd);
    set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));

    vect.push_back(d1);
    vect.push_back(d2);

    Environment e(set, vect, 8, 5);

    auto& dataSourcesCpy = e.getDataSources();
    ASSERT_NE(&dataSourcesCpy, &vect)
        << "Vector returned by the environment is the same as the one given to "
           "the constructor instead of a copy.";
    ASSERT_EQ(dataSourcesCpy.size(), vect.size())
        << "Number of DataHandler in the vector returned by the accessor "
           "differs from the one given at construction.";
    for (unsigned int i = 0; i < dataSourcesCpy.size(); i++) {
        ASSERT_EQ(&dataSourcesCpy.at(i).get(), &vect.at(i).get())
            << "Instruction referenced in the copied Set should be identical "
               "to the ones referenced in the Set given at construction.";
    }
}