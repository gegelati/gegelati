#include <gtest/gtest.h>

#include <array>
#include <string>
#include <vector>

#include "data/dataType.h"
#include "individual.h"

TEST(DataTypeTest, DefaultConstructorCreatesInvalidDescriptor)
{
    Data::DataType descriptor;

    EXPECT_EQ(descriptor.rank, 0u) << "Default rank should be 0.";
    EXPECT_EQ(descriptor.dimensions.size(), 0u) << "Default dimension 0 should be 0.";
    EXPECT_EQ(descriptor.elementType, nullptr) << "Default element type should be null.";
    EXPECT_EQ(descriptor.elementSize, 0u) << "Default element size should be 0.";
    EXPECT_EQ(descriptor.strides.size(), 0u) << "Default strides should be empty.";
    EXPECT_EQ(descriptor.totalElements(), 1u) << "Default total elements should be 1.";
    EXPECT_EQ(descriptor.memorySpan(), 1u) << "Default memory span should be 1.";
    EXPECT_TRUE(descriptor.isContiguous()) << "A scalar is contiguous.";
    EXPECT_TRUE(descriptor.canFitIn(descriptor, 0)) << "Default descriptor should fit inside itself at index 0.";
}

TEST(DataTypeTest, ScalarFactoryCreatesScalarDescriptor)
{
    const Data::DataType descriptor = Data::DataType::type<double>();

    EXPECT_EQ(descriptor.rank, 0u) << "Scalar rank should be 0.";
    EXPECT_EQ(descriptor.dimensions.size(), 0u) << "Default dimension 0 should be 0.";
    ASSERT_NE(descriptor.elementType, nullptr) << "Scalar elementType should not be null.";
    EXPECT_TRUE(*descriptor.elementType == typeid(double)) << "Scalar type should be double.";
    EXPECT_EQ(descriptor.elementSize, sizeof(double)) << "Scalar size should be sizeof(double).";
    EXPECT_EQ(descriptor.strides.size(), 0u) << "A scalar has no strides.";
    EXPECT_EQ(descriptor.totalElements(), 1u) << "Scalar total elements should be 1.";
    EXPECT_TRUE(descriptor.canFitIn(descriptor, 0)) << "descriptor should always fit inside itself at index 0.";
    EXPECT_TRUE(descriptor.canFitIn(Data::DataType::type<double>(), 0)) << "Scalar should fit inside itself at offset 0.";
    EXPECT_FALSE(descriptor.canFitIn(Data::DataType::type<double>(), 1)) << "Scalar should not start at offset 1 inside a scalar.";
    EXPECT_FALSE(descriptor.canFitIn(Data::DataType::type<double>({1}), 0)) << "Scalar should not fit an array of the same type.";
    EXPECT_FALSE(descriptor.canFitIn(Data::DataType::type<double>({1, 1}), 0)) << "Scalar should not fit an array of the same type.";
}

TEST(DataTypeTest, Array1dFactoryCreates1DDescriptor)
{
    const Data::DataType descriptor = Data::DataType::type<int>({4});

    EXPECT_EQ(descriptor.rank, 1u) << "1D rank should be 1.";
    EXPECT_EQ(descriptor.dimensions.size(), 1u) << "Default dimension 0 should be 0.";
    EXPECT_EQ(descriptor.dimensions[0], 4u) << "1D size should be 4.";
    ASSERT_NE(descriptor.elementType, nullptr) << "1D elementType should not be null.";
    EXPECT_TRUE(*descriptor.elementType == typeid(int)) << "1D type should be int.";
    EXPECT_EQ(descriptor.elementSize, sizeof(int)) << "1D size should be sizeof(int).";
    ASSERT_EQ(descriptor.strides.size(), 1u) << "1D should have one stride.";
    EXPECT_EQ(descriptor.strides[0], 1u) << "The elements of a 1D array follow each other.";
    EXPECT_EQ(descriptor.totalElements(), 4u) << "1D total elements should be 4.";
    EXPECT_EQ(descriptor.memorySpan(), 4u) << "1D memory span should be 4.";

    EXPECT_TRUE(descriptor.canFitIn(descriptor, 0)) << "descriptor should always fit inside itself at index 0.";

    EXPECT_TRUE(descriptor.canFitIn(Data::DataType::type<int>({4}), 0)) << "An identical 1D descriptor should fit inside itself.";
    EXPECT_TRUE(descriptor.canFitIn(Data::DataType::type<int>({2}), 1)) << "A valid 1D window should fit at the end of the descriptor.";
    EXPECT_FALSE(descriptor.canFitIn(Data::DataType::type<int>({2}), 3)) << "A 1D window exceeding the end should fail.";

    EXPECT_TRUE(descriptor.canFitIn(Data::DataType::type<double>(), 2)) << "Scalar should fit inside itself at offset 2.";
    EXPECT_FALSE(descriptor.canFitIn(Data::DataType::type<double>(), 5)) << "Scalar should not fit inside itself at offset 5.";

    EXPECT_FALSE(descriptor.canFitIn(Data::DataType::type<double>({1, 1}), 0)) << "Scalar should not fit an array of the same type.";
}

TEST(DataTypeTest, Array2dFactoryCreates2DDescriptor)
{
    const Data::DataType descriptor = Data::DataType::type<float>({2, 3});

    EXPECT_EQ(descriptor.rank, 2u) << "2D rank should be 2.";
    EXPECT_EQ(descriptor.dimensions.size(), 2u) << "Default dimension 0 should be 0.";
    EXPECT_EQ(descriptor.dimensions[0], 2u) << "2D row count should be 2.";
    EXPECT_EQ(descriptor.dimensions[1], 3u) << "2D column count should be 3.";
    ASSERT_NE(descriptor.elementType, nullptr) << "2D elementType should not be null.";
    EXPECT_TRUE(*descriptor.elementType == typeid(float)) << "2D type should be float.";
    EXPECT_EQ(descriptor.elementSize, sizeof(float)) << "2D size should be sizeof(float).";
    ASSERT_EQ(descriptor.strides.size(), 2u) << "2D should have two strides.";
    EXPECT_EQ(descriptor.strides[0], 3u) << "Moving to the next row skips a whole row.";
    EXPECT_EQ(descriptor.strides[1], 1u) << "Moving to the next column skips one element.";
    EXPECT_EQ(descriptor.totalElements(), 6u) << "2D total elements should be 6.";
    EXPECT_TRUE(descriptor.isContiguous()) << "A 2D array stored alone is contiguous.";
    EXPECT_TRUE(descriptor.canFitIn(descriptor, 0)) << "descriptor should always fit inside itself at index 0.";
    EXPECT_TRUE(descriptor.canFitIn(Data::DataType::type<float>({2, 3}), 0)) << "An identical 2D descriptor should fit inside itself.";
    EXPECT_TRUE(descriptor.canFitIn(Data::DataType::type<float>({1, 3}), 3)) << "A valid 2D window should fit when it starts on a row boundary.";
    EXPECT_FALSE(descriptor.canFitIn(Data::DataType::type<float>({2, 2}), 2)) << "A 2D window that overflows the right edge should fail.";

    EXPECT_TRUE(descriptor.canFitIn(Data::DataType::type<double>(), 5)) << "Scalar should fit inside  at offset 5.";
    EXPECT_FALSE(descriptor.canFitIn(Data::DataType::type<double>(), 8)) << "Scalar should not fit inside  at offset 8.";

    EXPECT_TRUE(descriptor.canFitIn(Data::DataType::type<double>({2}), 4)) << "array should fit inside  at offset 4.";
    EXPECT_FALSE(descriptor.canFitIn(Data::DataType::type<double>({2}), 7)) << "array should not fit inside at offset 7.";
    EXPECT_FALSE(descriptor.canFitIn(Data::DataType::type<double>({4}), 2)) << "array should not fit inside  at offset 2.";
}
TEST(DataTypeTest, SubViewKeepsTheStridesOfTheSource)
{
    // Window of 2x3 in a source of 3x4:    . . . .
    //                                      . X X X     (the window starts at the row 1, column 0)
    //                                      . X X X
    const Data::DataType source = Data::DataType::type<double>({3, 4});
    const Data::DataType sub = Data::DataType::subView(Data::DataType::type<double>({2, 3}), source);

    EXPECT_EQ(sub.rank, 2u) << "Sub-view rank should match requested rank.";
    EXPECT_EQ(sub.dimensions[0], 2u) << "Sub-view row count should be 2.";
    EXPECT_EQ(sub.dimensions[1], 3u) << "Sub-view col count should be 3.";
    EXPECT_TRUE(*sub.elementType == typeid(double)) << "Sub-view element type should stay double.";
    ASSERT_EQ(sub.strides.size(), 2u);
    EXPECT_EQ(sub.strides[0], 4u) << "The next row of the window is a whole row of the source further.";
    EXPECT_EQ(sub.strides[1], 1u);
    EXPECT_EQ(sub.totalElements(), 6u) << "Sub-view total elements should be rows * cols.";
    EXPECT_FALSE(sub.isContiguous()) << "A window narrower than the source has a gap between its rows.";
    EXPECT_EQ(sub.memorySpan(), 7u) << "From the first to the last element: 1 * 4 + 2 * 1 + 1.";
    EXPECT_EQ(sub.offsetOf(0), 0u);
    EXPECT_EQ(sub.offsetOf(2), 2u) << "Last element of the first row.";
    EXPECT_EQ(sub.offsetOf(3), 4u) << "First element of the second row.";
    EXPECT_EQ(sub.offsetOf(5), 6u);

    // A window of lower rank follows the last axes of the source.
    const Data::DataType cube = Data::DataType::type<int>({2, 3, 4});
    EXPECT_EQ(cube.strides, (Data::Extents{12, 4, 1}));
    const Data::DataType slice = Data::DataType::subView(Data::DataType::type<int>({3, 4}), cube);
    EXPECT_EQ(slice.strides, (Data::Extents{4, 1}));
    const Data::DataType line = Data::DataType::subView(Data::DataType::type<int>({2}), cube);
    EXPECT_EQ(line.strides, (Data::Extents{1}));
    const Data::DataType element = Data::DataType::subView(Data::DataType::type<int>(), cube);
    EXPECT_EQ(element.rank, 0u);

    EXPECT_THROW(Data::DataType::subView(Data::DataType::type<int>({1, 1, 1}), slice), std::invalid_argument) << "A window cannot have more axes than its source.";
}

TEST(DataTypeTest, IsContiguousAndMemorySpan)
{
    EXPECT_TRUE(Data::DataType::type<int>({4}).isContiguous());
    EXPECT_TRUE(Data::DataType::type<int>({2, 3, 4}).isContiguous());

    // Full lines of a bigger source follow each other, but only part of the lines does not.
    const Data::DataType source = Data::DataType::type<int>({4, 5});
    EXPECT_TRUE(Data::DataType::subView(Data::DataType::type<int>({2, 5}), source).isContiguous());
    EXPECT_FALSE(Data::DataType::subView(Data::DataType::type<int>({2, 4}), source).isContiguous());
    EXPECT_TRUE(Data::DataType::subView(Data::DataType::type<int>({1, 4}), source).isContiguous()) << "One line is contiguous.";
    EXPECT_EQ(Data::DataType::subView(Data::DataType::type<int>({2, 5}), source).memorySpan(), 10u);
    EXPECT_EQ(Data::DataType::subView(Data::DataType::type<int>({2, 4}), source).memorySpan(), 9u) << "1 * 5 + 3 + 1.";
    EXPECT_EQ(Data::DataType::type<int>({0, 3}).memorySpan(), 0u) << "An empty view has no memory.";
}

TEST(DataTypeTest, ForEachRunVisitsTheRunsOfAnyLayout)
{
    using Run = std::array<size_t, 3>; // memory offset, block offset, length
    auto runsOf = [](const Data::DataType& layout, const Data::DataType& block, size_t base) {
        std::vector<Run> runs;
        layout.forEachRun(block, base, [&](size_t memory, size_t blockOffset, size_t length) { runs.push_back({memory, blockOffset, length}); });
        return runs;
    };

    // A block of a contiguous array is one run.
    const Data::DataType matrix = Data::DataType::type<int>({4, 5});
    EXPECT_EQ(runsOf(matrix, matrix, 0), (std::vector<Run>{{0, 0, 20}}));
    EXPECT_EQ(runsOf(matrix, Data::DataType::type<int>({2, 5}), 5), (std::vector<Run>{{5, 0, 10}})) << "Full lines are one run.";

    // A narrower block is one run per line.
    EXPECT_EQ(runsOf(matrix, Data::DataType::type<int>({2, 3}), 6), (std::vector<Run>{{6, 0, 3}, {11, 3, 3}}));

    // A scalar block is one element.
    EXPECT_EQ(runsOf(matrix, Data::DataType::type<int>(), 7), (std::vector<Run>{{7, 0, 1}}));

    // 3D: a block of 2 planes of 2 lines of 2 elements in a 3x3x4 array has 4 runs.
    const Data::DataType cube = Data::DataType::type<int>({3, 3, 4});
    EXPECT_EQ(runsOf(cube, Data::DataType::type<int>({2, 2, 2}), 0),
              (std::vector<Run>{{0, 0, 2}, {4, 2, 2}, {12, 4, 2}, {16, 6, 2}}));

    // 3D: when the last two axes are complete, each plane is one run.
    EXPECT_EQ(runsOf(cube, Data::DataType::type<int>({2, 3, 4}), 12), (std::vector<Run>{{12, 0, 24}}));
    EXPECT_EQ(runsOf(cube, Data::DataType::type<int>({2, 2, 4}), 0), (std::vector<Run>{{0, 0, 8}, {12, 8, 8}}));

    // The layout of a window is used: lines of a window of a window.
    const Data::DataType window = Data::DataType::subView(Data::DataType::type<int>({3, 3}), matrix);
    EXPECT_EQ(runsOf(window, Data::DataType::type<int>({2, 2}), 0), (std::vector<Run>{{0, 0, 2}, {5, 2, 2}}));

    // An empty block has no run, and a block cannot have more axes than the layout.
    EXPECT_TRUE(runsOf(matrix, Data::DataType::type<int>({0, 3}), 0).empty());
    EXPECT_THROW(runsOf(matrix, Data::DataType::type<int>({1, 1, 1}), 0), std::invalid_argument);
}

TEST(DataTypeTest, ExtentsBehaveLikeASmallVector)
{
    Data::Extents extents{2, 3};
    EXPECT_EQ(extents.size(), 2u);
    EXPECT_FALSE(extents.empty());
    EXPECT_EQ(extents[1], 3u);
    EXPECT_EQ(extents, Data::Extents(std::vector<size_t>{2, 3}));
    EXPECT_NE(extents, (Data::Extents{2, 3, 1}));
    EXPECT_NE(extents, (Data::Extents{2, 4}));
    EXPECT_EQ(extents.toVector(), (std::vector<size_t>{2, 3}));

    extents.resize(3);
    EXPECT_EQ(extents, (Data::Extents{2, 3, 0})) << "New values are zero.";
    extents.resize(1);
    EXPECT_EQ(extents, (Data::Extents{2}));

    size_t sum = 0;
    for(size_t value: Data::Extents{1, 2, 3}) {
        sum += value;
    }
    EXPECT_EQ(sum, 6u);

    EXPECT_THROW(Data::Extents(std::vector<size_t>(Data::Extents::Capacity + 1, 1)), std::length_error) << "At most 8 axes.";
    EXPECT_THROW(Data::DataType::type<int>(std::vector<size_t>(Data::DataType::MaxRank + 1, 1)), std::length_error);
    EXPECT_NO_THROW(Data::DataType::type<int>(std::vector<size_t>(Data::DataType::MaxRank, 1)));
}

TEST(DataTypeTest, CanFitInHandlesScalar1DAnd2DCases)
{
    const Data::DataType scalar = Data::DataType::type<int>();
    const Data::DataType oneD = Data::DataType::type<int>({5});
    const Data::DataType twoD = Data::DataType::type<int>({3, 4});
    Data::DataType wrong;
    wrong.rank = 3;

    EXPECT_TRUE(scalar.canFitIn(Data::DataType::type<int>(), 0)) << "Scalar into scalar should fit.";
    EXPECT_FALSE(scalar.canFitIn(Data::DataType::type<int>({1}), 0)) << "Scalar should not accept a 1D shape.";
    EXPECT_FALSE(scalar.canFitIn(Data::DataType::type<int>({1, 1}), 0)) << "Scalar should not accept a 2D shape.";

    EXPECT_TRUE(oneD.canFitIn(Data::DataType::type<int>(), 0)) << "A 1D view can contain a scalar at its first element.";
    EXPECT_TRUE(oneD.canFitIn(Data::DataType::type<int>({2}), 1)) << "A valid 1D sub-view should fit.";
    EXPECT_FALSE(oneD.canFitIn(Data::DataType::type<int>({3}), 3)) << "Out-of-range 1D sub-view should fail.";
    EXPECT_FALSE(oneD.canFitIn(Data::DataType::type<int>({1, 2}), 1)) << "1D descriptor should reject a 2D requested shape.";

    EXPECT_TRUE(twoD.canFitIn(Data::DataType::type<int>(), 0)) << "A 2D view can contain a scalar at the origin.";
    EXPECT_TRUE(twoD.canFitIn(Data::DataType::type<int>({2}), 1)) << "A valid 1D window in a 2D descriptor should fit.";
    EXPECT_TRUE(twoD.canFitIn(Data::DataType::type<int>({1, 2}), 1)) << "A valid 2D window should fit.";
    EXPECT_FALSE(twoD.canFitIn(Data::DataType::type<int>({2, 3}), 2)) << "An out-of-bounds 2D window should fail.";

    EXPECT_FALSE(twoD.canFitIn(wrong)) << "Should be false with a rank 3 type";
}

TEST(DataTypeTest, CanFitInUsesLinearOffsetRelativeToCurrentDescriptor)
{
    const Data::DataType source = Data::DataType::type<int>({3, 4});
    const Data::DataType window = Data::DataType::subView(Data::DataType::type<int>({2, 2}), source);

    EXPECT_TRUE(window.canFitIn(Data::DataType::type<int>({2, 2}), 0)) << "The exact same 2x2 window should fit inside itself.";
    EXPECT_FALSE(window.canFitIn(Data::DataType::type<int>({2, 2}), 1)) << "A shifted 2x2 window should not fit if it leaves its own descriptor.";
    EXPECT_TRUE(source.canFitIn(Data::DataType::type<int>({2, 2}), 5)) << "A source descriptor can fit a valid 2D window at offset 5.";
    EXPECT_FALSE(source.canFitIn(Data::DataType::type<int>({2, 2}), 10)) << "An offset past the valid bounds should fail.";
}

TEST(DataTypeTest, getAddressSpace) 
{
    const Data::DataType scalar0 = Data::DataType::type<int>();
    const Data::DataType scalar1 = Data::DataType::type<int>(); 

    ASSERT_EQ(scalar0.getAddressSpace(scalar1), 1) << "AddressSpace should be 1";
    ASSERT_EQ(scalar0.getAddressSpace(scalar0), 1) << "AddressSpace should be 1";

    const Data::DataType array1d0 = Data::DataType::type<int>({8});
    const Data::DataType array1d1 = Data::DataType::type<int>({4}); 
    
    ASSERT_EQ(array1d1.getAddressSpace(array1d1), 1) << "AddressSpace should be 1";
    ASSERT_EQ(array1d0.getAddressSpace(array1d0), 1) << "AddressSpace should be 1";
    ASSERT_EQ(array1d0.getAddressSpace(scalar0), 8) << "AddressSpace should be 8";
    ASSERT_EQ(array1d0.getAddressSpace(array1d1), 5) << "AddressSpace should be 5";
    ASSERT_EQ(scalar0.getAddressSpace(array1d0), 0) << "AddressSpace should be 0";
    ASSERT_EQ(array1d1.getAddressSpace(array1d0), 0) << "AddressSpace should be 0";

    
    const Data::DataType array2d0 = Data::DataType::type<int>({8, 8});
    const Data::DataType array2d1 = Data::DataType::type<int>({4, 4});
    
    ASSERT_EQ(array2d0.getAddressSpace(array2d0), 1) << "AddressSpace should be 1";
    ASSERT_EQ(array2d1.getAddressSpace(array2d1), 1) << "AddressSpace should be 1";
    ASSERT_EQ(array2d0.getAddressSpace(array2d1), 25) << "AddressSpace should be 25 (5*5)";
    ASSERT_EQ(array2d0.getAddressSpace(array1d0), 8) << "AddressSpace should be 8";
    ASSERT_EQ(array2d0.getAddressSpace(array1d1), 40) << "AddressSpace should be 40 (8*5)";
    ASSERT_EQ(array2d0.getAddressSpace(scalar1), 64) << "AddressSpace should be 64 (8*8)";
    ASSERT_EQ(array2d1.getAddressSpace(array1d1), 4) << "AddressSpace should be 4";
    
    
    ASSERT_EQ(array2d1.getAddressSpace(array2d0), 0) << "AddressSpace should be 0";
    ASSERT_EQ(scalar0.getAddressSpace(array2d0), 0) << "AddressSpace should be 0";
    ASSERT_EQ(array1d0.getAddressSpace(array2d0), 0) << "AddressSpace should be 0";

    // getValidAddress
    ASSERT_EQ(array2d0.getValidAddress(scalar0, 0), 0);
    ASSERT_EQ(array2d0.getValidAddress(scalar0, 10), 10);
    ASSERT_EQ(array2d0.getValidAddress(scalar0, 63), 63);

    // 8x8 source, 1D request of length 4.
    // Compact addresses: 0..4 per row.
    ASSERT_EQ(array2d0.getValidAddress(array1d1, 0), 0);
    ASSERT_EQ(array2d0.getValidAddress(array1d1, 1), 1);
    ASSERT_EQ(array2d0.getValidAddress(array1d1, 4), 4);
    ASSERT_EQ(array2d0.getValidAddress(array1d1, 5), 8);
    ASSERT_EQ(array2d0.getValidAddress(array1d1, 6), 9);
    ASSERT_EQ(array2d0.getValidAddress(array1d1, 9), 12);
    ASSERT_EQ(array2d0.getValidAddress(array1d1, 10), 16);
    ASSERT_EQ(array2d0.getValidAddress(array1d1, 39), 60);

    // 8x8 source, 4x4 request.
    // Compact addresses form a 5x5 grid.
    ASSERT_EQ(array2d0.getValidAddress(array2d1, 0), 0);
    ASSERT_EQ(array2d0.getValidAddress(array2d1, 1), 1);
    ASSERT_EQ(array2d0.getValidAddress(array2d1, 4), 4);
    ASSERT_EQ(array2d0.getValidAddress(array2d1, 5), 8);
    ASSERT_EQ(array2d0.getValidAddress(array2d1, 6), 9);
    ASSERT_EQ(array2d0.getValidAddress(array2d1, 9), 12);
    ASSERT_EQ(array2d0.getValidAddress(array2d1, 10), 16);
    ASSERT_EQ(array2d0.getValidAddress(array2d1, 24), 36);

}

TEST(DataTypeTest, EqualityAndLayoutAwareComparisonBehaveAsExpected)
{
    const Data::DataType a = Data::DataType::type<int>({2, 3});
    const Data::DataType b = Data::DataType::type<int>({2, 3});
    const Data::DataType c = Data::DataType::subView(Data::DataType::type<int>({2, 3}), Data::DataType::type<int>({4, 5}));

    EXPECT_TRUE(a == b) << "Same logical shape and type should compare equal.";
    EXPECT_FALSE(a != b) << "Inequality should be false for identical descriptors.";
    EXPECT_TRUE(a.equalsWithLayout(a)) << "A descriptor should compare equal to itself including its layout.";
    EXPECT_FALSE(a.equalsWithLayout(c)) << "Different strides should make the layout-aware comparison fail.";
    EXPECT_TRUE(a == c) << "Equality intentionally ignores the layout.";
}

TEST(DataTypeTest, ToString)
{
    const Data::DataType descriptor = Data::DataType::subView(
        Data::DataType::type<double>({2, 2}),
        Data::DataType::type<double>({3, 4}));

    const std::string text = descriptor.toString();
    const std::string expect = "DataType{rank=2, dimensions=[2, 2], strides=[4, 1], elementType=double, elementSize=8}";

    ASSERT_EQ(text, expect) << "Text should be equal";

    std::ostringstream oss;
    oss << descriptor;
    ASSERT_EQ(expect, oss.str()) << "Text should be equal";

    Data::DataType scalarType = Data::DataType::type<double>();
    Data::DataType wrongType;
    ASSERT_NO_THROW(scalarType.summary());
    ASSERT_NO_THROW(wrongType.summary());
    ASSERT_NO_THROW(descriptor.summary());

    auto typeD =  Data::DataType::type<std::shared_ptr<const Individual>>();
    typeD.elementType = nullptr;
    ASSERT_NO_THROW(typeD.summary()) << "For coverage";
    ASSERT_NO_THROW(typeD.toString()) << "For coverage";

}

