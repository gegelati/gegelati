#include <gtest/gtest.h>

#include "data/constant.h"
#include "data/dataValue.h"
#include "instructions/multByConstant.h"

TEST(MultByConstParamTest, ExecutePrimitiveType)
{
    int value = 2;
    Data::Constant constant{5};
    Instructions::MultByConstant<int> instruction;

    const Data::DataValue result = instruction.execute(
        {Data::DataView::scalar(value), Data::DataView::scalar(constant)});

    ASSERT_EQ(result.type(), typeid(int));
    ASSERT_EQ(result.getScalar<int>(), 10);
}

TEST(MultByConstParamTest, RejectsWrongOperandTypes)
{
    double value = 2.0;
    Data::Constant constant{5};
    Instructions::MultByConstant<int> instruction;

    EXPECT_THROW(instruction.execute(
                     {Data::DataView::scalar(value),
                      Data::DataView::scalar(constant)}),
                 std::invalid_argument);
}

#ifdef CODE_GENERATION
TEST(MultByConstParamTest, PrintConstructor)
{
    Instructions::MultByConstant<int> instruction("$0 = $1 * $2;");
    SUCCEED();
}
#endif
