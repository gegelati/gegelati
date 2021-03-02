#include <gtest/gtest.h>

#include "data/array2DWrapper.h"
#include "data/dataHandler.h"

TEST(Array2DWrapperTest, Constructor)
{
    std::vector<double> values{0.0, 1.1, 2.2, 3.3, 4.4, 5.5};
    ASSERT_NO_THROW({
        Data::DataHandler* d = new Data::Array2DWrapper<double>(2, 3, &values);
        delete d;
    }) << "Call to ArrayWrapper constructor failed.";
}

TEST(Array2DWrapperTest, getAddressesAccessed)
{
    const size_t h = 10;
    const size_t w = 12;
    Data::Array2DWrapper<float> a(w, h);

    // Primitive type
    std::vector<size_t> addr;
    ASSERT_NO_THROW(addr = a.getAddressesAccessed(typeid(float), 50))
        << "Retrieving the vector for a valid primitive type failed.";
    ASSERT_EQ(addr.size(), 1)
        << "Incorrect number of addresses accessed was returned.";
    ASSERT_EQ(addr[0], 50) << "Incorrect address was returned.";

    // 1D array
    ASSERT_NO_THROW(addr = a.getAddressesAccessed(typeid(float[5]), 38))
        << "Retrieving the vector for a valid primitive type failed.";
    ASSERT_EQ(addr.size(), 5)
        << "Incorrect number of addresses accessed was returned.";
    auto baseAddress = ((38) / (w - 5 + 1) * w) + ((38) % (w - 5 + 1));
    for (auto idx = 0; idx < 5; idx++) {
        ASSERT_EQ(addr[idx], baseAddress + idx)
            << "Incorrect address was returned.";
    }

    // 2D array
    ASSERT_NO_THROW(addr = a.getAddressesAccessed(typeid(float[5][3]), 42))
        << "Retrieving the vector for a valid primitive type failed.";
    ASSERT_EQ(addr.size(), 5 * 3)
        << "Incorrect number of addresses accessed was returned.";
    baseAddress = ((42) / (w - 3 + 1) * w) + ((42) % (w - 3 + 1));
    for (auto idxH = 0; idxH < 5; idxH++) {
        for (auto idxW = 0; idxW < 3; idxW++) {
            ASSERT_EQ(addr[idxH * 3 + idxW], baseAddress + (idxH * w) + idxW)
                << "Incorrect address was returned.";
        }
    }
}

TEST(Array2DWrapperTest, getAddressSpace)
{
    size_t h = 3;
    size_t w = 5;
    Data::Array2DWrapper<int> a(w, h);

    // Check primitive type provided by 1D array
    ASSERT_EQ(a.getAddressSpace(typeid(int)), w * h)
        << "Address space of the 2D array of int is not width*height for "
           "typeid(int).";

    ASSERT_EQ(a.getAddressSpace(typeid(int[2])), (w - 2 + 1) * h)
        << "Address space of the 2D array of int is not correct for "
           "typeid(int[2]).";

    // Request a 2D array with valid dimensions
    ASSERT_EQ(a.getAddressSpace(typeid(int[2][4])), (w - 4 + 1) * (h - 2 + 1))
        << "Returned address space for int[2][4] in a 2D int array of size 5x3 "
           "is incorrect.";

    // Request a const 2D array with valid dimensions
    ASSERT_EQ(a.getAddressSpace(typeid(const int[2][4])),
              (w - 4 + 1) * (h - 2 + 1))
        << "Returned address space for int[2][4] in a 2D int array of size 5x3 "
           "is incorrect.";

    // Request a 2D array with invalid dimensions
    ASSERT_EQ(a.getAddressSpace(typeid(int[4][2])), 0)
        << "Returned address space for int[4][2] in a 2D int array of size 5x3 "
           "is incorrect.";

    // Request a 2D array with invalid type
    ASSERT_EQ(a.getAddressSpace(typeid(long[1][1])), 0)
        << "Returned address space for int[4][2] in a 2D int array of size 5x3 "
           "is incorrect.";
}

TEST(Array2DWrapperTest, getDataAt)
{
    const size_t h = 3;
    const size_t w = 5;
    std::vector<int> values(h * w);
    Data::Array2DWrapper<int> a(w, h, &values);

    // Fill the array
    for (auto idx = 0; idx < h * w; idx++) {
        values.at(idx) = idx;
    }
    a.invalidateCachedHash();

    // Check primitive type provided by 1D array
    for (auto idx = 0; idx < h * w; idx++) {
        const int val =
            *((a.getDataAt(typeid(int), idx)).getSharedPointer<const int>());
        ASSERT_EQ(val, idx) << "Value with primitive type is not as expected.";
    }

    // Check 1D array
    for (auto idx = 0; idx < a.getAddressSpace(typeid(int[3])); idx++) {
        std::shared_ptr<const int> valSPtr =
            (a.getDataAt(typeid(int[3]), idx)).getSharedPointer<const int[]>();
        const int* valPtr = valSPtr.get();
        for (auto subIdx = 0; subIdx < 3; subIdx++) {
            const int val = valPtr[subIdx];
            ASSERT_EQ(val, (idx / (w - 3 + 1) * w + idx % (w - 3 + 1)) + subIdx)
                << "Value with primitive type is not as expected.";
        }
    }

    // Check 2D array (returned as a 1D array)
    for (auto idx = 0; idx < a.getAddressSpace(typeid(int[3][2])); idx++) {
        std::shared_ptr<const int> valSPtr =
            (a.getDataAt(typeid(int[2][3]), idx))
                .getSharedPointer<const int[]>();
        const int(*valPtr)[3] = (int(*)[3])valSPtr.get();
        size_t srcIdx = idx / (w - 3 + 1) * w + idx % (w - 3 + 1);
        for (auto subH = 0; subH < 2; subH++) {
            for (auto subW = 0; subW < 3; subW++) {
                const int val = valPtr[subH][subW];
                ASSERT_EQ(val, srcIdx + (subH * w) + subW)
                    << "Value with primitive type is not as expected.";
            }
        }
    }

#ifndef NDEBUG
    ASSERT_THROW(a.getDataAt(typeid(int[h * w]), 1), std::invalid_argument)
        << "Address exceeding the addressSpace should cause an exception.";

    ASSERT_THROW(a.getDataAt(typeid(int[w - 1]), h * (w - 1) + 1),
                 std::out_of_range)
        << "Address exceeding the addressSpace should cause an exception.";
#else
    // No alternative test to put here.. out of range access to memory _may_
    // happen without being detected.
#endif
}