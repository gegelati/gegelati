#include <gtest/gtest.h>

#include "data/constant.h"

TEST(ConstantTest, Cast)
{
    Data::Constant c{1};

    ASSERT_EQ((int32_t)c, 1)
        << "int32_t casted value of constant is incorrect.";
    ASSERT_EQ((double)c, 1.0)
        << "double casted value of constant is incorrect.";
}

TEST(ConstantTest, OperatorEqual)
{
    Data::Constant c0{1}, c1{2}, c3{1};

    ASSERT_NE(c0, c1) << "Constant with different values should be non-equal.";
    ASSERT_EQ(c0, c3) << "Constant with identical values should be equal.";
}