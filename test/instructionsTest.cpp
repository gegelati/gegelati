#include <gtest/gtest.h>

#include <functional>

#include "oldData/dataValue.h"
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
    ASSERT_EQ(instruction.getOperandTypes().at(0).elementType, &typeid(double));
    ASSERT_EQ(instruction.getOperandTypes().at(1).elementType, &typeid(double));
}

TEST(InstructionsTest, CheckArgumentTypes)
{
    Instructions::AddPrimitiveType<double> instruction;

    Data::DataView dvFirst(2.5);
    Data::DataView dvSecond(5.6);
    Data::DataView dvWrong(5);

    EXPECT_TRUE(instruction.checkOperandTypes(
        {dvFirst, dvSecond}));
    EXPECT_FALSE(instruction.checkOperandTypes(
        {dvFirst}));
    EXPECT_FALSE(instruction.checkOperandTypes(
        {dvFirst, dvWrong}));
}

TEST(InstructionsTest, Execute)
{
    Instructions::AddPrimitiveType<double> instruction;
    double first = 2.6;
    double second = 5.5;
    int wrong = 3;

    const Data::DataValue result = instruction.execute(
        {Data::DataView(first), Data::DataView(second)});
    EXPECT_DOUBLE_EQ(result.getScalar<double>(), 8.1);
    EXPECT_THROW(instruction.execute(
                     {Data::DataView(first),
                      Data::DataView(wrong)}),
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
    Instructions::LambdaInstruction<double, double, double> lambda(
        std::function<double(double, double)>{
            [](double first, double second) { return first - second; }},
        "$0 = $1 - $2;");

    EXPECT_TRUE(add.isPrintable());
    EXPECT_TRUE(multiply.isPrintable());
    EXPECT_TRUE(lambda.isPrintable());
}
#endif

TEST(InstructionTest, handleInputTypes)
{
    auto unary = [](double a) -> double { return a; };
    Instructions::LambdaInstruction<double, double> iUnary(unary);

    ASSERT_TRUE(iUnary.handleInputTypes({Data::DataType::scalar<double>()}));
    ASSERT_TRUE(iUnary.handleInputTypes({Data::DataType::scalar<float>(), Data::DataType::scalar<double>()}));
    ASSERT_FALSE(iUnary.handleInputTypes({Data::DataType::scalar<float>()}));
    ASSERT_FALSE(iUnary.handleInputTypes({}));

    auto binary = [](double a, double b) -> double { return a + b; };
    Instructions::LambdaInstruction<double, double, double> iBinary(binary);

    // The same input can satisfy multiple operands.
    ASSERT_TRUE(iBinary.handleInputTypes({Data::DataType::scalar<double>()}));
    ASSERT_TRUE(iBinary.handleInputTypes({Data::DataType::scalar<double>(), Data::DataType::scalar<double>()}));
    ASSERT_TRUE(iBinary.handleInputTypes({Data::DataType::scalar<double>(), Data::DataType::scalar<float>()}));
    ASSERT_FALSE(iBinary.handleInputTypes({Data::DataType::scalar<float>(), Data::DataType::scalar<int>()}));

    auto mixed = [](double a, float b) -> double { return a + b; };
    Instructions::LambdaInstruction<double, double, float> iMixed(mixed);

    ASSERT_TRUE(iMixed.handleInputTypes({
        Data::DataType::scalar<double>(),
        Data::DataType::scalar<float>()
    }));
    ASSERT_FALSE(iMixed.handleInputTypes({Data::DataType::scalar<double>()}));
    ASSERT_FALSE(iMixed.handleInputTypes({Data::DataType::scalar<float>()}));
    ASSERT_FALSE(iMixed.handleInputTypes({Data::DataType::scalar<int>()}));
}

TEST(InstructionTest, handleInputTypesArrayShapes)
{
    Instructions::LambdaInstruction<
        double,
        const double[16][17],
        const double[16][16]
    > instruction([](const double a[16][17], const double b[16][16]) -> double {
        return 0.0;
    });

    ASSERT_TRUE(instruction.handleInputTypes({
        Data::DataType::array2d<double>(16, 17),
        Data::DataType::array2d<double>(16, 16)
    }));

    ASSERT_FALSE(instruction.handleInputTypes({
        Data::DataType::array2d<double>(16, 16)
    }));

    ASSERT_TRUE(instruction.handleInputTypes({
        Data::DataType::array2d<double>(16, 17)
    }));

    ASSERT_FALSE(instruction.handleInputTypes({
        Data::DataType::array2d<double>(3, 3),
        Data::DataType::array2d<double>(4, 4)
    }));

    ASSERT_FALSE(instruction.handleInputTypes({
        Data::DataType::array2d<float>(16, 17),
        Data::DataType::array2d<float>(16, 16)
    }));

    ASSERT_TRUE(instruction.handleInputTypes({
        Data::DataType::array2d<float>(3, 3),
        Data::DataType::array2d<double>(16, 17),
        Data::DataType::array2d<double>(16, 16)
    }));
}

TEST(InstructionTest, handleInputTypesRegression)
{
    Instructions::LambdaInstruction<
        double,
        const double[16][17],
        const double[16][16]
    > instruction([](const double a[16][17], const double b[16][16]) -> double {
        return 8.0;
    });

    // Only the second operand can be satisfied. The instruction must be rejected.
    ASSERT_FALSE(instruction.handleInputTypes({
        Data::DataType::array2d<double>(16, 16),
        Data::DataType::array2d<float>(16, 16),
        Data::DataType::array2d<int>(16, 16),
        Data::DataType::array1d<double>(16),
        Data::DataType::array1d<float>(16),
        Data::DataType::array1d<int>(16),
        Data::DataType::array1d<double>(0),
        Data::DataType::array1d<float>(0),
        Data::DataType::array1d<int>(0)
    }));

    // Both operands can now be satisfied.
    ASSERT_TRUE(instruction.handleInputTypes({
        Data::DataType::array2d<double>(16, 17),
        Data::DataType::array2d<double>(16, 16)
    }));
}

TEST(InstructionTest, handleOutputType)
{
    Instructions::LambdaInstruction<double, double> instruction(
        [](double a) -> double { return a; }
    );

    ASSERT_TRUE(instruction.handleOutputType(Data::DataType::scalar<double>()));
    ASSERT_TRUE(instruction.handleOutputType(Data::DataType::array1d<double>(4)));
    ASSERT_FALSE(instruction.handleOutputType(Data::DataType::scalar<float>()));
}

