#include <gtest/gtest.h>

#include <algorithm>
#include <initializer_list>

#include "data/dataHandler.h"
#include "data/primitiveTypeArray.h"
#include "data/primitiveTypeArray2D.h"

namespace {
template <typename T>
Data::UntypedSharedPtr makeArray(const std::initializer_list<T>& values)
{
    T* data = new T[values.size()];
    std::copy(values.begin(), values.end(), data);
    return Data::UntypedSharedPtr{
        std::make_shared<Data::UntypedSharedPtr::Model<const T[]>>(data)};
}
}

TEST(DataWriteTest, WritesDifferentScalarTypes)
{
    Data::PrimitiveTypeArray<int> ints(1);
    Data::PrimitiveTypeArray<float> floats(1);
    Data::PrimitiveTypeArray<double> doubles(1);

    ints.setDataAt(typeid(int), 0, Data::UntypedSharedPtr{new int(7)});
    floats.setDataAt(typeid(float), 0,
                     Data::UntypedSharedPtr{new float(1.5f)});
    doubles.setDataAt(typeid(double), 0,
                      Data::UntypedSharedPtr{new double(2.5)});

    EXPECT_EQ(*ints.getDataAt(typeid(int), 0).getSharedPointer<const int>(),
              7);
    EXPECT_FLOAT_EQ(
        *floats.getDataAt(typeid(float), 0).getSharedPointer<const float>(),
        1.5f);
    EXPECT_DOUBLE_EQ(
        *doubles.getDataAt(typeid(double), 0)
             .getSharedPointer<const double>(),
        2.5);
}

TEST(DataWriteTest, WritesOneDimensionalArray)
{
    Data::PrimitiveTypeArray<double> values(5);
    values.setDataAt(typeid(double[3]), 1,
                     makeArray<double>({10.0, 20.0, 30.0}));

    for (size_t index = 0; index < 5; index++) {
        const double expected = index == 0 ? 0.0 : index == 1 ? 10.0
                                                  : index == 2 ? 20.0
                                                               : index == 3 ? 30.0 : 0.0;
        EXPECT_DOUBLE_EQ(
            *values.getDataAt(typeid(double), index)
                 .getSharedPointer<const double>(),
            expected);
    }
}

TEST(DataWriteTest, WritesTwoDimensionalWindow)
{
    Data::PrimitiveTypeArray2D<int> values(4, 3);
    values.setDataAt(typeid(int[2][2]), 2,
                     makeArray<int>({1, 2, 3, 4}));

    const int expected[] = {0, 0, 1, 2, 0, 0, 3, 4, 0, 0, 0, 0};
    for (size_t index = 0; index < 12; index++) {
        EXPECT_EQ(*values.getDataAt(typeid(int), index)
                       .getSharedPointer<const int>(),
                  expected[index]);
    }
}