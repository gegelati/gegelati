#include <gtest/gtest.h>

#include "data/arrayWrapper.h"
#include "data/dataHandler.h"

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

TEST(ArrayWrapperTest, GetDataAtNativeType)
{
    const size_t size{3};
    std::vector<float> values{0.0f, 1.1f, 2.2f};
    Data::DataHandler* d = new Data::ArrayWrapper<float>(size, &values);

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

    delete d;
}

TEST(ArrayWrapperTest, PrimitiveDataArrayGetDataAtArray)
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