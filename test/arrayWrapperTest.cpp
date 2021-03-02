#include <gtest/gtest.h>

#include "data/arrayWrapper.h"
#include "data/dataHandler.h"
#include "data/primitiveTypeArray.h"

TEST(ArrayWrapperTest, Constructor)
{
    std::vector<double> values{0.0, 1.1, 2.2};
    ASSERT_NO_THROW({
        Data::DataHandler* d = new Data::ArrayWrapper<double>();
        delete d;
    }) << "Call to ArrayWrapper constructor failed.";
}

TEST(ArrayWrapperTest, SetPointer)
{
    Data::ArrayWrapper<double> d(3, nullptr);
    std::vector<double> values{0.0, 1.1, 2.2};
    std::vector<double> values2{0.0, 1.1};

    ASSERT_NO_THROW(d.setPointer(&values))
        << "Setting a pointer with a valid vector size should not fail.";
    ASSERT_THROW(d.setPointer(&values2), std::domain_error)
        << "Setting a pointer with a size different from the one of the "
           "ArrayWrapper should fail.";
}

TEST(ArrayWrapperTest, CanProvideTemplateType)
{
    Data::DataHandler* d = new Data::ArrayWrapper<double>(4);

    ASSERT_TRUE(d->canHandle(typeid(double)))
        << "ArrayWrapper<double>() wrongfully say it can not provide "
           "double data.";
    ASSERT_FALSE(d->canHandle(typeid(int)))
        << "ArrayWrapper<double>() wrongfully say it can provide int "
           "data.";
    ASSERT_FALSE(d->canHandle(typeid(Data::UntypedSharedPtr)))
        << "ArrayWrapper<double>() wrongfully say it can provide "
           "UntypedSharedPtr data.";
    delete d;
}

TEST(ArrayWrapperTest, CanProvideArray)
{
    Data::DataHandler* d = new Data::ArrayWrapper<double>(4);

    ASSERT_TRUE(d->canHandle(typeid(double[2])))
        << "ArrayWrapper<double>(4) wrongfully say it can not provide "
           "std::array<double, 2> data.";
    ASSERT_FALSE(d->canHandle(typeid(double[5])))
        << "ArrayWrapper<double>(4) wrongfully say it can provide "
           "std::array<double, 5> data.";
    ASSERT_FALSE(d->canHandle(typeid(int[3])))
        << "ArrayWrapper<double>(4) wrongfully say it can provide "
           "std::array<int, 3> data.";
    delete d;
}

TEST(ArrayWrapperTest, GetAddressesAccessed)
{
    Data::ArrayWrapper<float> d(100);

    std::vector<size_t> accessedAddresses;
    ASSERT_NO_THROW(accessedAddresses =
                        d.getAddressesAccessed(typeid(float), 25))
        << "No exception should be thrown with a valid type at a valid "
           "address.";
    ASSERT_EQ(accessedAddresses.size(), 1)
        << "Only one address should be accessed with native type at a valid "
           "address.";
    ASSERT_EQ(accessedAddresses.at(0), 25)
        << "Address accessed does not correspond to the requested one.";

    ASSERT_NO_THROW(accessedAddresses =
                        d.getAddressesAccessed(typeid(float[3]), 50))
        << "No exception should be thrown with a valid type at a valid "
           "address.";
    ASSERT_EQ(accessedAddresses.size(), 3)
        << "Only one address should be accessed with native type at a valid "
           "address.";
    for (int i = 0; i < 3; i++) {
        ASSERT_EQ(accessedAddresses.at(i), 50 + i)
            << "Address accessed does not correspond to the requested one.";
    }

    ASSERT_NO_THROW(accessedAddresses =
                        d.getAddressesAccessed(typeid(double), 75))
        << "No exception should be thrown with an invalid type at a valid "
           "address.";
    ASSERT_EQ(accessedAddresses.size(), 0)
        << "No address should be accessed with and invalid type at a valid "
           "address.";

    ASSERT_NO_THROW(accessedAddresses =
                        d.getAddressesAccessed(typeid(float[25]), 90))
        << "No exception should be thrown with a valid type at an invalid "
           "address.";
    ASSERT_EQ(accessedAddresses.size(), 0)
        << "No address should be accessed with and valid type at an invalid "
           "address.";
}

TEST(ArrayWrapperTest, GetAddressSpaceTemplateType)
{
    Data::DataHandler* d = new Data::ArrayWrapper<long>(64); // Array of 64 long
    ASSERT_EQ(d->getAddressSpace(typeid(long)), 64)
        << "Address space size for type long in ArrayWrapper<long>(64) "
           "is not 64";
    ASSERT_EQ(d->getAddressSpace(typeid(int)), 0)
        << "Address space size for type int in ArrayWrapper<long>(64) is "
           "not 0";

    delete d;
}

TEST(ArrayWrapperTest, GetAddressSpaceArray)
{
    Data::DataHandler* d = new Data::ArrayWrapper<long>(64); // Array of 64 long
    ASSERT_EQ(d->getAddressSpace(typeid(long[50])), 15)
        << "Address space size for type std::array<long, 50> in "
           "ArrayWrapper<long>(64) is not 15";
    ASSERT_EQ(d->getAddressSpace(typeid(double[50])), 0)
        << "Address space size for type std::array<double, 50> in "
           "ArrayWrapper<long>(64) is not 0";

    delete d;
}

TEST(ArrayWrapperTest, GetDataAtNativeType)
{
    const size_t size{3};
    std::vector<float> values{0.0f, 1.1f, 2.2f};
    Data::ArrayWrapper<float>* d = new Data::ArrayWrapper<float>(size, &values);

    for (int i = 0; i < size; i++) {
        const float a =
            *(d->getDataAt(typeid(float), i).getSharedPointer<const float>());
        ASSERT_EQ((float)a, values.at(i))
            << "Data at valid address and type can not be accessed.";
    }

#ifndef NDEBUG
    ASSERT_THROW(d->getDataAt(typeid(float), size), std::out_of_range)
        << "Address exceeding the addressSpace should cause an exception.";
#else
    // No alternative test to put here.. out of range access to memory _may_
    // happen without being detected.
#endif

#ifndef NDEBUG
    ASSERT_THROW(d->getDataAt(typeid(double), 0), std::invalid_argument)
        << "Requesting a non-handled type, even at a valid location, should "
           "cause an exception.";
#else
    ASSERT_THROW(
        d->getDataAt(typeid(double), 0).getSharedPointer<const double>(),
        std::out_of_range)
        << "In NDEBUG mode, a pointer with invalid type will be returned when "
           "requesting a non-handled type, even at a valid location.";
#endif

    // test null ptr container
    d->setPointer(nullptr);
    ASSERT_THROW(d->getDataAt(typeid(float), 0).getSharedPointer<const float>(),
                 std::runtime_error)
        << "Accessing data within a ArrayWrapper associated to a nullptr "
           "should fail.";

    delete d;
}

TEST(ArrayWrapperTest, GetDataAtArray)
{
    const size_t size{8};
    std::vector<int> values{0, 1, 2, 3, 4, 5, 6, 7};
    const size_t sizeArray = 3;
    Data::ArrayWrapper<int>* d = new Data::ArrayWrapper<int>(size, &values);

    // Get data as arrays
    for (int i = 0; i < size - sizeArray + 1; i++) {
        std::shared_ptr<const int> sptr =
            d->getDataAt(typeid(int[sizeArray]), i)
                .getSharedPointer<const int[]>();
        const int* a = (sptr.get());
        ASSERT_NE(a, nullptr) << "Retrieved data is a null_ptr";
        for (int idx = 0; idx < sizeArray; idx++) {
            ASSERT_EQ(a[idx], i + idx)
                << "Value given in the array do not correspond to the one "
                   "stored in the array.";
        }
    }

#ifndef NDEBUG
    ASSERT_THROW(d->getDataAt(typeid(int[sizeArray]), size - 1),
                 std::out_of_range)
        << "Address exceeding the addressSpace should cause an exception.";
#else
    // No alternative test to put here.. out of range access to memory _may_
    // happen without being detected.
#endif

#ifndef NDEBUG
    ASSERT_THROW(d->getDataAt(typeid(long[sizeArray]), 0),
                 std::invalid_argument)
        << "Requesting a non-handled type, even at a valid location, should "
           "cause an exception.";
#else
    ASSERT_THROW(d->getDataAt(typeid(long[sizeArray]), 0)
                     .getSharedPointer<const long[sizeArray]>(),
                 std::out_of_range)
        << "In NDEBUG mode, a pointer with invalid type will be returned when "
           "requesting a non-handled type, even at a valid location.";
#endif

    delete d;
}

TEST(ArrayWrapperTest, GetLargestAddressSpace)
{
    Data::DataHandler* d =
        new Data::ArrayWrapper<float>(20); // Array of 20 float
    ASSERT_EQ(d->getLargestAddressSpace(), 20)
        << "Largest address space size for type in "
           "ArrayWrapper<float>(20) is not 20 as expected.";

    delete d;
}

TEST(ArrayWrapperTest, resetData)
{
    // For code coverage.
    Data::DataHandler* d =
        new Data::ArrayWrapper<float>(20); // Array of 20 float
    ASSERT_NO_THROW(d->resetData())
        << "Reseting data on an ArrayWrapper should not do anything.";

    delete d;
}

TEST(ArrayWrapperTest, ScaleLocation)
{
    Data::DataHandler* d =
        new Data::ArrayWrapper<float>(20); // Array of 20 float
    ASSERT_EQ(d->scaleLocation(25, typeid(float)), 5)
        << "Scaled location is wrong.";
    ASSERT_EQ(d->scaleLocation(25, typeid(const float[5])), 9)
        << "Scaled location is wrong.";

    delete d;
}

TEST(ArrayWrapperTest, Hash)
{
    // Create a DataHandler
    const size_t size{8};
    const size_t address{3};
    const double doubleValue{42.0};
    std::vector<double> values(8);

    Data::ArrayWrapper<double> d(size, &values);

    // Get hash
    size_t hash = 0;
    ASSERT_NO_THROW(hash = d.getHash());
    // change the content of the array
    values.at(address) = doubleValue;
    d.invalidateCachedHash();
    ASSERT_NE(hash, d.getHash());

    // Tesh Hash of nullptr
    d.setPointer(nullptr);
    ASSERT_EQ(d.getHash(), 0);
}

TEST(ArrayWrapperTest, CanHandleConstants)
{
    Data::DataHandler* d = new Data::ArrayWrapper<int>(4);
    ASSERT_FALSE(d->canHandle(typeid(Data::Constant)))
        << "ArrayWrapper<double>() wrongfully say it can provide "
           "Data::Constant "
           "data.";
    delete d;
}

TEST(ArrayWrapperTest, Clone)
{
    // Create a DataHandler
    const size_t size{8};
    const size_t address{3};
    const double doubleValue{42.0};
    std::vector<double> values(size);

    // create a first one to increase the DataHandler::count
    Data::ArrayWrapper<int> d0(12);
    Data::ArrayWrapper<double> d(size, &values);
    // change the content of the array
    values.at(address) = doubleValue;
    d.invalidateCachedHash();
    // Hash was voluntarily not computed before clone.

    // Create a clone
    Data::DataHandler* dClone = NULL;
    ASSERT_NO_THROW(dClone = d.clone();)
        << "Cloning a ArrayWrapper<double> failed.";

    // Extra if to remove warnings on further use of dClone.
    if (dClone == NULL)
        FAIL() << "Cloning of ArrayWrapper returned a NULL Pointer.";

    // Check ID
    ASSERT_EQ(dClone->getId(), d.getId())
        << "Cloned and original ArrayWrapper do not have the same ID as "
           "expected.";
    // Check the polymorphic type.
    ASSERT_EQ(typeid(*dClone), typeid(Data::PrimitiveTypeArray<double>))
        << "Type of clone ArrayWrapper is not a PrimitiveTypeArray as "
           "expected.";
    // Compute the hashes
    ASSERT_EQ(dClone->getHash(), d.getHash())
        << "Hash of clone and original DataHandler differ.";

    // Change data in the original to make sure the two dHandlers are decoupled.
    size_t hash = dClone->getHash();
    values.at(address + 1) = doubleValue + 1.0;
    d.invalidateCachedHash();
    ASSERT_NE(dClone->getHash(), d.getHash())
        << "Hash of clone and original DataHandler should differ after "
           "modification of data in the original.";
    ASSERT_EQ(dClone->getHash(), hash)
        << "Hash of the clone dataHandler should remain unchanged after "
           "modification of data within the original DataHandler.";
    delete dClone;

    // Check nullptr clone also
    d.setPointer(nullptr);
    ASSERT_NO_THROW(dClone = d.clone())
        << "Cloning an ArrayWrapper pointing to a nullptr should not fail.";
    if (dClone == NULL)
        FAIL() << "Cloning of ArrayWrapper returned a NULL Pointer.";
    delete dClone;
}