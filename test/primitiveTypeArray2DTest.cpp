#include <gtest/gtest.h>

#include "data/primitiveTypeArray2D.h"

TEST(PrimitiveTypeArray2DTest, Constructor)
{
    Data::PrimitiveTypeArray2D<double>* array;
    ASSERT_NE(array = new Data::PrimitiveTypeArray2D<double>(3, 4), nullptr)
        << "A PrimitiveTypeArray2D<double> could not be built successfully.";
    ASSERT_NO_THROW(delete array)
        << "PrimitiveTypeArray2D could not be deleted.";
}

TEST(PrimitiveTypeArray2DTest, getAddressSpace)
{
    size_t h = 3;
    size_t w = 5;
    Data::PrimitiveTypeArray2D<int> a(w, h);

    // Check primitive type provided by 1D array
    ASSERT_EQ(a.getAddressSpace(typeid(int)), w * h)
        << "Address space of the 2D array of int is not width*height for "
           "typeid(int).";

    ASSERT_EQ(a.getAddressSpace(typeid(int[2])), (w * h) - 2 + 1)
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