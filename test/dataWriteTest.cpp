#include <gtest/gtest.h>

#include <algorithm>
#include <initializer_list>

#include "data/dataHandler.h"
#include "data/primitiveTypeArray.h"
#include "data/primitiveTypeArray2D.h"

TEST(DataWriteTest, WritesDifferentScalarTypes)
{
    Data::PrimitiveTypeArray<int> ints(1);
    Data::PrimitiveTypeArray<float> floats(1);
    Data::PrimitiveTypeArray<double> doubles(1);

    ints.setDataAt(typeid(int), 0, Data::DataView::scalar(int(7)));
    floats.setDataAt(typeid(float), 0,
                     Data::DataView::scalar(float(1.5f)));
    doubles.setDataAt(typeid(double), 0,
                      Data::DataView::scalar(double(2.5)));

    EXPECT_EQ(ints.getDataAt(typeid(int), 0).getScalar<int>(),
              7);
    EXPECT_FLOAT_EQ(
        floats.getDataAt(typeid(float), 0).getScalar<float>(),
        1.5f);
    EXPECT_DOUBLE_EQ(
        doubles.getDataAt(typeid(double), 0)
             .getScalar<double>(),
        2.5);
}

TEST(DataWriteTest, WritesOneDimensionalArray)
{
    Data::PrimitiveTypeArray<double> values(5);
    const double data[] = {10.0, 20.0, 30.0};

    values.setDataAt(typeid(double[3]), 1, Data::DataView::array(data, Data::DataShape{3}));

    for (size_t index = 0; index < 5; index++) {
        const double expected = index == 0 ? 0.0 : index == 1 ? 10.0
                                                  : index == 2 ? 20.0
                                                               : index == 3 ? 30.0 : 0.0;
        EXPECT_DOUBLE_EQ(
            values.getDataAt(typeid(double), index)
                 .getScalar<double>(),
            expected);
    }
}

TEST(DataWriteTest, WritesTwoDimensionalWindow)
{
    Data::PrimitiveTypeArray2D<int> values(4, 3);
    int data[2 * 2] = {1, 2, 3, 4};
    values.setDataAt(typeid(int[2][2]), 2,
                     Data::DataView::array(data, Data::DataShape{2, 2}));

    const int expected[] = {0, 0, 1, 2, 0, 0, 3, 4, 0, 0, 0, 0};
    for (size_t index = 0; index < 12; index++) {
        EXPECT_EQ(values.getDataAt(typeid(int), index)
                       .getScalar<int>(),
                  expected[index]);
    }
}