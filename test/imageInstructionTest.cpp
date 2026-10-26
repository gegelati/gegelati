#include <gtest/gtest.h>

#ifdef GEGELATI_WITH_OPENCV

#include <cstdint>
#include <vector>

#include "data/constant.h"
#include "instructions/imageInstructionSet.h"

namespace {
    constexpr size_t SIZE = 5;

    /// Execute an instruction on images/params and return the output pixels.
    template <typename T>
    std::vector<T> run(const Instructions::ImageInstruction<T>& instruction,
                       const std::vector<std::vector<T>>& images,
                       const std::vector<double>& params)
    {
        std::vector<Data::DataView> views;
        for (const std::vector<T>& image : images) {
            views.emplace_back(image.data(),
                               Data::DataType::array2d<T>(SIZE, SIZE));
        }
        std::vector<Data::Constant> constants;
        for (double param : params) {
            constants.push_back(Data::Constant{param});
        }
        for (const Data::Constant& constant : constants) {
            views.emplace_back(constant);
        }

        const Data::DataValue result = instruction.execute(views);
        auto data = result.template getData<T>();
        return std::vector<T>(data.get(), data.get() + SIZE * SIZE);
    }

    template <typename T> std::vector<T> square3x3()
    {
        std::vector<T> image(SIZE * SIZE, T(0));
        for (size_t row = 1; row <= 3; row++) {
            for (size_t col = 1; col <= 3; col++) {
                image[row * SIZE + col] = T(255);
            }
        }
        return image;
    }

    template <typename T> std::vector<T> singlePixel()
    {
        std::vector<T> image(SIZE * SIZE, T(0));
        image[2 * SIZE + 2] = T(255);
        return image;
    }
} // namespace

template <typename T> class ImageInstructionTest : public ::testing::Test
{
};
using PixelTypes = ::testing::Types<uint8_t, float, double>;
TYPED_TEST_SUITE(ImageInstructionTest, PixelTypes);

TYPED_TEST(ImageInstructionTest, OperandAndOutputTypes)
{
    auto instruction =
        Instructions::ImageInstructions::create<TypeParam>("erode", SIZE, SIZE);

    ASSERT_EQ(instruction->getNbOperands(), 3);
    ASSERT_EQ(instruction->getOperandTypes()[0],
              Data::DataType::array2d<TypeParam>(SIZE, SIZE));
    ASSERT_EQ(instruction->getOperandTypes()[1],
              Data::DataType::type<Data::Constant>());
    ASSERT_EQ(instruction->getOperandTypes()[2],
              Data::DataType::type<Data::Constant>());
    ASSERT_EQ(instruction->getOutputType(),
              Data::DataType::array2d<TypeParam>(SIZE, SIZE));
}

TYPED_TEST(ImageInstructionTest, ErodeShrinksSquare)
{
    // params: rectangle kernel (0), ksize 3 (1).
    auto instruction =
        Instructions::ImageInstructions::create<TypeParam>("erode", SIZE, SIZE);
    const auto result =
        run<TypeParam>(*instruction, {square3x3<TypeParam>()}, {0, 1});
    ASSERT_EQ(result, singlePixel<TypeParam>());
}

TYPED_TEST(ImageInstructionTest, DilateGrowsPixel)
{
    auto instruction = Instructions::ImageInstructions::create<TypeParam>(
        "dilate", SIZE, SIZE);
    const auto result =
        run<TypeParam>(*instruction, {singlePixel<TypeParam>()}, {0, 1});
    ASSERT_EQ(result, square3x3<TypeParam>());
}

TYPED_TEST(ImageInstructionTest, BitwiseNot)
{
    auto instruction = Instructions::ImageInstructions::create<TypeParam>(
        "bitwise_not", SIZE, SIZE);
    const auto result =
        run<TypeParam>(*instruction, {singlePixel<TypeParam>()}, {});
    for (size_t i = 0; i < SIZE * SIZE; i++) {
        ASSERT_EQ(result[i], i == 2 * SIZE + 2 ? TypeParam(0) : TypeParam(255));
    }
}

TYPED_TEST(ImageInstructionTest, AddSaturates)
{
    auto instruction =
        Instructions::ImageInstructions::create<TypeParam>("add", SIZE, SIZE);
    const std::vector<TypeParam> a(SIZE * SIZE, TypeParam(200));
    const std::vector<TypeParam> b(SIZE * SIZE, TypeParam(100));
    const auto result = run<TypeParam>(*instruction, {a, b}, {});
    for (TypeParam pixel : result) {
        ASSERT_EQ(pixel, TypeParam(255));
    }
}

TYPED_TEST(ImageInstructionTest, ThresholdBinary)
{
    auto instruction = Instructions::ImageInstructions::create<TypeParam>(
        "threshold", SIZE, SIZE);
    std::vector<TypeParam> image(SIZE * SIZE, TypeParam(50));
    image[0] = TypeParam(150);
    // p0 < 128: binary; p1: threshold value.
    const auto result = run<TypeParam>(*instruction, {image}, {0, 100});
    ASSERT_EQ(result[0], TypeParam(255));
    ASSERT_EQ(result[1], TypeParam(0));
}

TYPED_TEST(ImageInstructionTest, ParametersAreClamped)
{
    auto instruction = Instructions::ImageInstructions::create<TypeParam>(
        "threshold", SIZE, SIZE);
    std::vector<TypeParam> image(SIZE * SIZE, TypeParam(200));
    // Threshold 1000 is clamped to 255: nothing is above it.
    const auto result = run<TypeParam>(*instruction, {image}, {0, 1000});
    ASSERT_EQ(result[0], TypeParam(0));
}

TYPED_TEST(ImageInstructionTest, ExecuteIntoDestination)
{
    auto instruction = Instructions::ImageInstructions::create<TypeParam>(
        "bitwise_not", SIZE, SIZE);
    const std::vector<TypeParam> image(SIZE * SIZE, TypeParam(10));
    Data::DataValue destination =
        Data::DataValue::zeros<TypeParam>(SIZE, SIZE);
    instruction->executeInto(
        {Data::DataView(image.data(),
                        Data::DataType::array2d<TypeParam>(SIZE, SIZE))},
        destination, 0);
    auto data = destination.template getData<TypeParam>();
    ASSERT_EQ(data[0], TypeParam(245));
}

TYPED_TEST(ImageInstructionTest, WrongOperandsThrow)
{
    auto instruction = Instructions::ImageInstructions::create<TypeParam>(
        "bitwise_not", SIZE, SIZE);
    const std::vector<TypeParam> wrongSize((SIZE + 1) * (SIZE + 1));
    ASSERT_THROW(instruction->execute({Data::DataView(
                     wrongSize.data(), Data::DataType::array2d<TypeParam>(
                                           SIZE + 1, SIZE + 1))}),
                 std::invalid_argument);
    ASSERT_THROW(instruction->execute({}), std::invalid_argument);
}

TEST(ImageInstructionSetTest, UnknownOperation)
{
    ASSERT_THROW(
        Instructions::ImageInstructions::create<double>("nope", SIZE, SIZE),
        std::invalid_argument);
}

// Every operation must run on a realistic image and keep the output shape.
TYPED_TEST(ImageInstructionTest, AllOperationsRun)
{
    constexpr size_t N = 28;
    std::vector<TypeParam> image(N * N);
    for (size_t i = 0; i < N * N; i++) {
        image[i] = TypeParam((i * 37) % 256);
    }
    auto all = Instructions::ImageInstructions::createAll<TypeParam>(N, N);
    ASSERT_FALSE(all.empty());
    for (const auto& instruction : all) {
        std::vector<Data::DataView> views;
        for (const Data::DataType& type : instruction->getOperandTypes()) {
            if (type.rank == 2) {
                views.emplace_back(image.data(), type);
            }
        }
        std::vector<Data::Constant> constants(
            instruction->getNbOperands() - views.size(), Data::Constant{64});
        for (const Data::Constant& constant : constants) {
            views.emplace_back(constant);
        }
        try {
            const Data::DataValue result = instruction->execute(views);
            ASSERT_EQ(result.getType(),
                      Data::DataType::array2d<TypeParam>(N, N))
                << instruction->getName();
        } catch (const std::exception& e) {
            FAIL() << instruction->getName() << ": " << e.what();
        }
    }
}

#else

TEST(ImageInstructionTest, OpenCVDisabled)
{
    GTEST_SKIP() << "Compile with GEGELATI_WITH_OPENCV=ON to run these tests.";
}

#endif
