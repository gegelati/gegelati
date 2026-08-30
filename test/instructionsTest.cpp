#include <gtest/gtest.h>

#include <functional>

#include "data/dataValue.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/lambdaInstruction.h"
#include "instructions/multByConstant.h"
#include "instructions/set.h"

TEST(InstructionsTest, ConstructorDestructorCall)
{
    ASSERT_NO_THROW({
        Instructions::Instruction* instruction =
            new Instructions::AddPrimitiveType<double>();
        delete instruction;
    });
}

TEST(InstructionsTest, OperandListAndNbParam)
{
    Instructions::AddPrimitiveType<double> instruction;
    ASSERT_EQ(instruction.getNbOperands(), 2);
    ASSERT_EQ(instruction.getOperandTypes().at(0).get(), typeid(double));
    ASSERT_EQ(instruction.getOperandTypes().at(1).get(), typeid(double));
}

TEST(InstructionsTest, CheckArgumentTypes)
{
    Instructions::AddPrimitiveType<double> instruction;
    double first = 2.5;
    double second = 5.6;
    int wrong = 5;

    EXPECT_TRUE(instruction.checkOperandTypes(
        {Data::DataView::scalar(first), Data::DataView::scalar(second)}));
    EXPECT_FALSE(instruction.checkOperandTypes(
        {Data::DataView::scalar(first)}));
    EXPECT_FALSE(instruction.checkOperandTypes(
        {Data::DataView::scalar(first), Data::DataView::scalar(wrong)}));
}

TEST(InstructionsTest, Execute)
{
    Instructions::AddPrimitiveType<double> instruction;
    double first = 2.6;
    double second = 5.5;
    int wrong = 3;

    const Data::DataValue result = instruction.execute(
        {Data::DataView::scalar(first), Data::DataView::scalar(second)});
    EXPECT_DOUBLE_EQ(result.getScalar<double>(), 8.1);
    EXPECT_THROW(instruction.execute(
                     {Data::DataView::scalar(first),
                      Data::DataView::scalar(wrong)}),
                 std::invalid_argument);
}

TEST(InstructionsTest, SetStoresInstructions)
{
    Instructions::Set set;
    Instructions::AddPrimitiveType<double> first;
    Instructions::AddPrimitiveType<float> second;

    EXPECT_TRUE(set.add(first));
    EXPECT_TRUE(set.add(second));
    EXPECT_EQ(set.getNbInstructions(), 2);
    EXPECT_EQ(&set.getInstruction(1), &second);
}

#ifdef CODE_GENERATION
TEST(InstructionsTest, ConstructorsWithPrintTemplates)
{
    Instructions::AddPrimitiveType<double> add("$0 = $1 + $2;");
    Instructions::MultByConstant<int> multiply("$0 = $1 * $2;");
    Instructions::LambdaInstruction<double, double> lambda(
        std::function<double(double, double)>{
            [](double first, double second) { return first - second; }},
        "$0 = $1 - $2;");

    EXPECT_TRUE(add.isPrintable());
    EXPECT_TRUE(multiply.isPrintable());
    EXPECT_TRUE(lambda.isPrintable());
}
#endif
