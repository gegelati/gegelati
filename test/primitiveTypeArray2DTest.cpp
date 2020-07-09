#include <gtest/gtest.h>

#include "data/primitiveTypeArray2D.h"

TEST(PrimitiveTypeArrayTest, Constructor)
{
    Data::PrimitiveTypeArray2D<double>* array;
    ASSERT_NE(array = new Data::PrimitiveTypeArray2D<double>(3, 4), nullptr)
        << "A PrimitiveTypeArray2D<double> could not be built successfully.";
    ASSERT_NO_THROW(delete array) << "PrimitiveTypeArray2D could not be deleted.";
}