
#include <gtest/gtest.h>

#include "data/dataHandler.h"
#include "data/pointerWrapper.h"
#include "data/primitiveTypeArray.h"

TEST(PointerWrapperTest, Constructor)
{
    ASSERT_NO_THROW({
        Data::DataHandler* d = new Data::PointerWrapper<double>();
        delete d;
    }) << "Call to PointerWrapper constructor failed.";
}

TEST(PointerWrapperTest, SetPointer)
{
    Data::PointerWrapper<double> d;
    double v0;

    ASSERT_NO_THROW(d.setPointer(&v0))
        << "Setting a pointer with a valid value should not fail.";
}

TEST(PointerWrapperTest, CanProvideTemplateType)
{
    Data::DataHandler* d = new Data::PointerWrapper<double>();

    ASSERT_TRUE(d->canHandle(typeid(double)))
        << "PointerWrapper<double>() wrongfully say it can not provide "
           "double data.";
    ASSERT_FALSE(d->canHandle(typeid(int)))
        << "PointerWrapper<double>() wrongfully say it can provide int "
           "data.";
    ASSERT_FALSE(d->canHandle(typeid(Data::UntypedSharedPtr)))
        << "PointerWrapper<double>() wrongfully say it can provide "
           "UntypedSharedPtr data.";
    delete d;
}

TEST(PointerWrapperTest, GetAddressesAccessed)
{
    Data::PointerWrapper<float> d;

    std::vector<size_t> accessedAddresses;
    ASSERT_NO_THROW(accessedAddresses =
                        d.getAddressesAccessed(typeid(float), 0))
        << "No exception should be thrown with a valid type at a valid "
           "address.";
    ASSERT_EQ(accessedAddresses.size(), 1)
        << "Only one address should be accessed with native type at a valid "
           "address.";
    ASSERT_EQ(accessedAddresses.at(0), 0)
        << "Address accessed does not correspond to the requested one.";

    ASSERT_NO_THROW(accessedAddresses =
                        d.getAddressesAccessed(typeid(double), 0))
        << "No exception should be thrown with an invalid type at a valid "
           "address.";
    ASSERT_EQ(accessedAddresses.size(), 0)
        << "No address should be accessed with and invalid type at a valid "
           "address.";
}

TEST(PointerWrapperTest, GetAddressSpace)
{
    Data::DataHandler* d = new Data::PointerWrapper<long>();

    ASSERT_EQ(d->getAddressSpace(typeid(long)), 1)
        << "Address space size for type long in PointerWrapper<long> "
           "is not 1";
    ASSERT_EQ(d->getAddressSpace(typeid(int)), 0)
        << "Address space size for type int in PointerWrapper<long> is "
           "not 0";

    delete d;
}

TEST(PointerWrapperTest, GetDataAtNativeType)
{
    float val = 1.2f;
    Data::PointerWrapper<float>* d = new Data::PointerWrapper<float>(&val);

    const float a =
        *(d->getDataAt(typeid(float), 0).getSharedPointer<const float>());
    ASSERT_EQ((float)a, val)
        << "Data at valid address and type can not be accessed.";

#ifndef NDEBUG
    ASSERT_THROW(d->getDataAt(typeid(float), 1), std::out_of_range)
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
        std::runtime_error)
        << "In NDEBUG mode, a pointer with invalid type will be returned when "
           "requesting a non-handled type, even at a valid location.";
#endif

    // test null ptr container
    d->setPointer(nullptr);
    ASSERT_THROW(d->getDataAt(typeid(float), 0).getSharedPointer<const float>(),
                 std::runtime_error)
        << "Accessing data within a PointerWrapper associated to a nullptr "
           "should fail.";

    delete d;
}

TEST(PointerWrapperTest, GetLargestAddressSpace)
{
    Data::DataHandler* d = new Data::PointerWrapper<float>();
    ASSERT_EQ(d->getLargestAddressSpace(), 1)
        << "Largest address space size for type in "
           "PointerWrapper<float> is not 1 as expected.";

    delete d;
}

TEST(PointerWrapperTest, resetData)
{
    // For code coverage.
    Data::DataHandler* d = new Data::PointerWrapper<float>();
    ASSERT_NO_THROW(d->resetData())
        << "Reseting data on an PointerWrapper should not do anything.";

    delete d;
}

TEST(PointerWrapperTest, ScaleLocation)
{
    Data::DataHandler* d = new Data::PointerWrapper<float>;
    ASSERT_EQ(d->scaleLocation(25, typeid(float)), 0)
        << "Scaled location is wrong.";

    delete d;
}

TEST(PointerWrapperTest, Hash)
{
    // Create a DataHandler
    double val = 1.2f;
    const double doubleValue{42.0};

    Data::PointerWrapper<double> d(&val);

    // Get hash
    size_t hash = 0;
    ASSERT_NO_THROW(hash = d.getHash());
    // change the content of the array
    val = doubleValue;
    ASSERT_NE(hash, d.getHash());

    // Tesh Hash of nullptr
    d.setPointer(nullptr);
    ASSERT_EQ(d.getHash(), 0);
}

TEST(PointerWrapperTest, Clone)
{
    // Create a DataHandler
    double val = 1.2f;
    const double doubleValue{42.0};

    // create a first one to increase the DataHandler::count
    Data::PointerWrapper<int> d0;

    // Create the actual dataHandler
    Data::PointerWrapper<double> d(&val);
    // change the content of the array
    val = doubleValue;
    // Hash was voluntarily not computed before clone.

    // Create a clone
    Data::DataHandler* dClone = NULL;
    ASSERT_NO_THROW(dClone = d.clone();)
        << "Cloning a PointerWrapper<double> failed.";

    // Extra if to remove warnings on further use of dClone.
    if (dClone == NULL)
        FAIL() << "Cloning of PointerWrapper returned a NULL Pointer.";

    // Check ID
    ASSERT_EQ(dClone->getId(), d.getId())
        << "Cloned and original PointerWrapper do not have the same ID as "
           "expected.";
    // Check the polymorphic type.
    ASSERT_EQ(typeid(*dClone), typeid(Data::PrimitiveTypeArray<double>))
        << "Type of clone PointerWrapper is not a PrimitiveTypeArray as "
           "expected.";
    // Compute the hashes
    ASSERT_EQ(dClone->getHash(), d.getHash())
        << "Hash of clone and original DataHandler differ.";

    // Change data in the original to make sure the two dHandlers are decoupled.
    size_t hash = dClone->getHash();
    val = doubleValue + 1.0;

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
        << "Cloning an PointerWrapper pointing to a nullptr should not fail.";
    if (dClone == NULL)
        FAIL() << "Cloning of PointerWrapper returned a NULL Pointer.";
    delete dClone;
}

#ifdef CODE_GENERATION
TEST(PointerWrapperTest, getNativeType)
{
    Data::DataHandler* d = new Data::PointerWrapper<double>();

    ASSERT_EQ(d->getNativeType(), typeid(double))
        << "Fail to retrieve typeid(double) from a PointerWrapper<double>.";

    delete d;
}

TEST(PointerWrapperTest, getDimensionSize)
{
    size_t size = 1;
    Data::DataHandler* d = new Data::PointerWrapper<double>();

    ASSERT_EQ(d->getDimensionsSize(), std::vector<size_t>({size}))
        << "Fail to retrieve a std::vector holding the size of the "
           "PointerWrapper.";

    delete d;
}
#endif