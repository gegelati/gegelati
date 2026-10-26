#include <gtest/gtest.h>

#include <numeric>
#include <stdexcept>
#include <vector>

#include "data/dataType.h"
#include "data/dataValue.h"
#include "data/dataView.h"

namespace {
    // 2x3x4 tensor filled with 0..23.
    Data::DataValue makeTensor()
    {
        std::vector<int> values(24);
        std::iota(values.begin(), values.end(), 0);
        return Data::DataValue::array(values, {2, 3, 4});
    }
}

TEST(DataNdTest, TypeRankAndSize)
{
    Data::DataType type = Data::DataType::type<int>({2, 3, 4});
    EXPECT_EQ(3, type.rank);
    EXPECT_EQ(24u, type.totalElements());
    EXPECT_EQ(24u, type.sourceTotalElements());
    EXPECT_NO_THROW(Data::DataType::type<int>({1, 1, 1, 1, 1, 1, 1}));
}

TEST(DataNdTest, AddressSpaceAndFit)
{
    Data::DataType type = Data::DataType::type<int>({2, 3, 4});
    EXPECT_EQ(24u, type.getAddressSpace(Data::DataType::type<int>()));
    EXPECT_EQ(2u * 3u * 2u, type.getAddressSpace(Data::DataType::type<int>({3})));
    EXPECT_EQ(2u * 2u * 3u, type.getAddressSpace(Data::DataType::type<int>({1, 2, 2})));
    EXPECT_EQ(1u * 2u * 3u, type.getAddressSpace(Data::DataType::type<int>({2, 2, 2})));
    EXPECT_EQ(0u, type.getAddressSpace(Data::DataType::type<int>({3, 1, 1})));
    EXPECT_EQ(0u, type.getAddressSpace(Data::DataType::type<double>({1, 1, 1})));
    EXPECT_FALSE(type.canFitIn(Data::DataType::type<int>({2}), 3));
    EXPECT_TRUE(type.canFitIn(Data::DataType::type<int>({2}), 2));
}

TEST(DataNdTest, ValidAddressStaysInBounds)
{
    Data::DataType type = Data::DataType::type<int>({2, 3, 4});
    Data::DataType request = Data::DataType::type<int>({1, 2, 2});
    const size_t space = type.getAddressSpace(request);
    for (size_t address = 0; address < space; ++address) {
        EXPECT_TRUE(type.canFitIn(request, type.getValidAddress(request, address)))
            << "address " << address;
    }
}

TEST(DataNdTest, SubViewOnTensor)
{
    Data::DataValue tensor = makeTensor();
    Data::DataView whole = tensor.view();
    Data::DataView sub = whole.getSubView(Data::DataType::type<int>({1, 2, 2}), 1 * 12 + 1 * 4 + 1);
    EXPECT_EQ(17u, sub.getSourceOffset());
    auto data = sub.getData<int>();
    EXPECT_EQ(17, data[0]);
    EXPECT_EQ(18, data[1]);
    EXPECT_EQ(21, data[2]);
    EXPECT_EQ(22, data[3]);

    // A sub-view of a sub-view keeps the original strides.
    Data::DataView subSub = sub.getSubView(Data::DataType::type<int>({2}), 2);
    auto subData = subSub.getData<int>();
    EXPECT_EQ(21, subData[0]);
    EXPECT_EQ(22, subData[1]);
}

TEST(DataNdTest, SubViewNonSquareMatrix)
{
    std::vector<int> values(3 * 5);
    std::iota(values.begin(), values.end(), 0);
    Data::DataValue matrix = Data::DataValue::array(values, {3, 5});
    Data::DataView view = matrix.view();
    Data::DataView sub = view.getSubView(Data::DataType::type<int>({2, 2}), 1 * 5 + 2);
    EXPECT_EQ(7u, sub.getSourceOffset());
    auto data = sub.getData<int>();
    EXPECT_EQ(7, data[0]);
    EXPECT_EQ(8, data[1]);
    EXPECT_EQ(12, data[2]);
    EXPECT_EQ(13, data[3]);
}

TEST(DataNdTest, GetAndSetSubValue)
{
    Data::DataValue tensor = makeTensor();
    ASSERT_NO_THROW(tensor.getSubValue<int>(Data::DataType::type<int>({2, 2, 2}), 1));
    Data::DataValue slice = tensor.getSubValue<int>(Data::DataType::type<int>({2, 2, 2}), 1);
    EXPECT_EQ(3, slice.getType().rank);
    EXPECT_EQ(Data::DataValue::array(std::vector<int>{1, 2, 5, 6, 13, 14, 17, 18}, {2, 2, 2}), slice);

    Data::DataValue patch = Data::DataValue::zeros<int>({1, 2, 2});
    std::cout<<patch<<std::endl;
    ASSERT_NO_THROW(patch.setScalarAt<int>(9, 3));
    ASSERT_NO_THROW(tensor.setSubValue(patch, 12 + 4 + 1));
    std::cout<<patch<<std::endl;
    EXPECT_EQ(0, tensor.getScalarAt<int>(17));
    std::cout<<patch<<std::endl;
    EXPECT_EQ(9, tensor.getScalarAt<int>(22));
    EXPECT_EQ(16, tensor.getScalarAt<int>(16));
}

TEST(DataNdTest, ZerosCloneAndConvert)
{
    Data::DataValue zeros = Data::DataValue::zeros<float>({2, 2, 2, 2});
    EXPECT_EQ(4, zeros.getType().rank);
    EXPECT_EQ(16u, zeros.getType().totalElements());
    EXPECT_EQ(zeros, zeros.clone());

    Data::DataValue converted = Data::DataValue::convertNumericValue<int, double>(makeTensor());
    EXPECT_EQ(Data::DataType::type<double>({2, 3, 4}), converted.getType());
    EXPECT_EQ(23.0, converted.getSubValue<double>(Data::DataType::type<double>(), 23).getScalar<double>());
}
