#include <gtest/gtest.h>

#include <array>
#include <vector>

#include "newData/dataView.h"

TEST(DataViewTest, ConstructorAndAccessorsHandleScalar1DAnd2DViews)
{
    std::vector<int> values{10, 20, 30, 40};
    int scalarValue = 42;

    Data::DataView scalarView(&scalarValue, Data::DataType::scalar<int>());
    ASSERT_NO_THROW(Data::DataView(&scalarValue, Data::DataType::scalar<int>()));
    EXPECT_EQ(scalarView.getRank(), 0u);
    EXPECT_EQ(scalarView.getDimensions()[0], 1u);
    EXPECT_EQ(scalarView.getDimensions()[1], 0u);
    EXPECT_EQ(scalarView.getElementType(), typeid(int));
    EXPECT_EQ(scalarView.getElementSize(), sizeof(int));
    EXPECT_TRUE(scalarView);

    const int& scalarRead = scalarView.getScalar<int>();
    EXPECT_EQ(scalarRead, 42);

    Data::DataView oneDView(values.data(), Data::DataType::array1d<int>(values.size()));
    ASSERT_NO_THROW(Data::DataView(values.data(), Data::DataType::array1d<int>(values.size())));
    EXPECT_EQ(oneDView.getRank(), 1u);
    EXPECT_EQ(oneDView.getDimensions()[0], static_cast<size_t>(values.size()));
    EXPECT_EQ(oneDView.getDimensions()[1], 0u);
    EXPECT_EQ(oneDView.getSourceRank(), 1u);
    EXPECT_EQ(oneDView.getSourceDimensions()[0], static_cast<size_t>(values.size()));
    EXPECT_EQ(oneDView.getSourceOffset(), 0u);
    EXPECT_TRUE(oneDView);

    const int* oneDPtr = nullptr;
    ASSERT_NO_THROW(oneDPtr = oneDView.getArray<int>());
    ASSERT_NE(oneDPtr, nullptr);
    for (size_t i = 0; i < values.size(); ++i) {
        EXPECT_EQ(oneDPtr[i], values[i]);
    }

    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
    Data::DataView twoDView(matrix, Data::DataType::array2d<int>(2, 3));
    ASSERT_NO_THROW(Data::DataView(matrix, Data::DataType::array2d<int>(2, 3)));
    EXPECT_EQ(twoDView.getRank(), 2u);
    EXPECT_EQ(twoDView.getDimensions()[0], 2u);
    EXPECT_EQ(twoDView.getDimensions()[1], 3u);
    EXPECT_EQ(twoDView.getSourceRank(), 2u);
    EXPECT_EQ(twoDView.getSourceDimensions()[0], 2u);
    EXPECT_EQ(twoDView.getSourceDimensions()[1], 3u);

    const int* twoDPtr = nullptr;
    ASSERT_NO_THROW(twoDPtr = twoDView.getArray<int>());
    ASSERT_NE(twoDPtr, nullptr);
    EXPECT_EQ(twoDPtr[0], 1);
    EXPECT_EQ(twoDPtr[1], 2);
    EXPECT_EQ(twoDPtr[2], 3);
    EXPECT_EQ(twoDPtr[3], 4);
    EXPECT_EQ(twoDPtr[4], 5);
    EXPECT_EQ(twoDPtr[5], 6);

    Data::DataView nullScalar(nullptr, Data::DataType::scalar<int>());
    ASSERT_THROW(nullScalar.getScalar<int>(), std::runtime_error);

    ASSERT_THROW(scalarView.getScalar<double>(), std::runtime_error);
    ASSERT_THROW(oneDView.getArray<double>(), std::runtime_error);
    ASSERT_THROW(scalarView.getArray<int>(), std::runtime_error);
    ASSERT_NO_THROW(oneDView.getScalar<int>());
}

TEST(DataViewTest, ConstructorFromScalarInfersType)
{
    int scalarValue = 42;
    Data::DataView scalarView(scalarValue);

    EXPECT_EQ(scalarView.getRank(), 0u);
    EXPECT_EQ(scalarView.getDimensions()[0], 1u);
    EXPECT_EQ(scalarView.getDimensions()[1], 0u);
    EXPECT_EQ(scalarView.getElementType(), typeid(int));

    EXPECT_EQ(scalarView.getScalar<int>(), 42);

    int oneDValues[4] = {10, 20, 30, 40};
    Data::DataView oneDView(oneDValues);

    EXPECT_EQ(oneDView.getRank(), 1u);
    EXPECT_EQ(oneDView.getDimensions()[0], 4u);
    EXPECT_EQ(oneDView.getDimensions()[1], 0u);
    EXPECT_EQ(oneDView.getElementType(), typeid(int));

    const int* oneDPtr = oneDView.getArray<int>();
    ASSERT_NE(oneDPtr, nullptr);
    EXPECT_EQ(oneDPtr[0], 10);
    EXPECT_EQ(oneDPtr[1], 20);
    EXPECT_EQ(oneDPtr[2], 30);
    EXPECT_EQ(oneDPtr[3], 40);

    int twoDValues[2][3] = {{1, 2, 3}, {4, 5, 6}};
    Data::DataView twoDView(twoDValues);

    EXPECT_EQ(twoDView.getRank(), 2u);
    EXPECT_EQ(twoDView.getDimensions()[0], 2u);
    EXPECT_EQ(twoDView.getDimensions()[1], 3u);
    EXPECT_EQ(twoDView.getElementType(), typeid(int));

    const int* twoDPtr = twoDView.getArray<int>();
    ASSERT_NE(twoDPtr, nullptr);
    EXPECT_EQ(twoDPtr[0], 1);
    EXPECT_EQ(twoDPtr[1], 2);
    EXPECT_EQ(twoDPtr[2], 3);
    EXPECT_EQ(twoDPtr[3], 4);
    EXPECT_EQ(twoDPtr[4], 5);
    EXPECT_EQ(twoDPtr[5], 6);
}

TEST(DataViewTest, GetScalarAtAndGetSubViewSupportValidAndInvalidAddresses)
{
    int source[3][4] = {{0, 1, 2, 3}, {4, 5, 6, 7}, {8, 9, 10, 11}};
    Data::DataView view(source, Data::DataType::array2d<int>(3, 4));

    EXPECT_EQ(view.getScalarAt<int>(0), 0);
    EXPECT_EQ(view.getScalarAt<int>(3), 3);
    EXPECT_EQ(view.getScalarAt<int>(4), 4);
    EXPECT_EQ(view.getScalarAt<int>(7), 7);
    EXPECT_EQ(view.getScalarAt<int>(11), 11);
    ASSERT_THROW(view.getScalarAt<int>(12), std::out_of_range);

    Data::DataView oneDView(source[0], Data::DataType::array1d<int>(4));
    Data::DataView validOneDSubView;
    ASSERT_NO_THROW(validOneDSubView = oneDView.getSubView(Data::DataType::array1d<int>(2), 1));
    const int* validOneDSubPtr = nullptr;
    ASSERT_NO_THROW(validOneDSubPtr = validOneDSubView.getArray<int>());
    ASSERT_NE(validOneDSubPtr, nullptr);
    EXPECT_EQ(validOneDSubPtr[0], 1);
    EXPECT_EQ(validOneDSubPtr[1], 2);

    ASSERT_THROW(oneDView.getSubView(Data::DataType::array1d<int>(4), 1), std::out_of_range);

    Data::DataView validTwoDSubView;
    ASSERT_NO_THROW(validTwoDSubView = view.getSubView(Data::DataType::array2d<int>(2, 2), 1));
    const int* validTwoDSubPtr = nullptr;
    ASSERT_NO_THROW(validTwoDSubPtr = validTwoDSubView.getArray<int>());
    ASSERT_NE(validTwoDSubPtr, nullptr);
    EXPECT_EQ(validTwoDSubPtr[0], 1);
    EXPECT_EQ(validTwoDSubPtr[1], 2);
    EXPECT_EQ(validTwoDSubPtr[2], 5);
    EXPECT_EQ(validTwoDSubPtr[3], 6);

    ASSERT_THROW(view.getSubView(Data::DataType::array2d<int>(2, 3), 2), std::out_of_range);
}

TEST(DataViewTest, CanFitCoversEveryRankAndTypeCombination)
{
    int scalarValue = 42;
    std::vector<int> oneD{10, 20, 30, 40, 50};
    int twoD[2][3] = {{1, 2, 3}, {4, 5, 6}};

    Data::DataView scalarView(&scalarValue, Data::DataType::scalar<int>());
    Data::DataView oneDView(oneD.data(), Data::DataType::array1d<int>(oneD.size()));
    Data::DataView twoDView(twoD, Data::DataType::array2d<int>(2, 3));

    EXPECT_TRUE(scalarView.canFit(Data::DataType::scalar<int>(), 0));
    EXPECT_FALSE(scalarView.canFit(Data::DataType::array1d<int>(1), 0));
    EXPECT_FALSE(scalarView.canFit(Data::DataType::array2d<int>(1, 1), 0));
    EXPECT_FALSE(scalarView.canFit(Data::DataType::array1d<double>(1), 0));

    EXPECT_TRUE(oneDView.canFit(Data::DataType::scalar<int>(), 0));
    EXPECT_TRUE(oneDView.canFit(Data::DataType::scalar<int>(), 2));
    EXPECT_FALSE(oneDView.canFit(Data::DataType::scalar<int>(), 5));
    EXPECT_TRUE(oneDView.canFit(Data::DataType::array1d<int>(2), 1));
    EXPECT_TRUE(oneDView.canFit(Data::DataType::array1d<int>(3), 2));
    EXPECT_FALSE(oneDView.canFit(Data::DataType::array1d<int>(4), 2));
    EXPECT_FALSE(oneDView.canFit(Data::DataType::array2d<int>(1, 2), 0));
    EXPECT_FALSE(oneDView.canFit(Data::DataType::array1d<double>(2), 0));

    EXPECT_TRUE(twoDView.canFit(Data::DataType::scalar<int>(), 0));
    EXPECT_TRUE(twoDView.canFit(Data::DataType::scalar<int>(), 5));
    EXPECT_FALSE(twoDView.canFit(Data::DataType::scalar<int>(), 6));
    EXPECT_TRUE(twoDView.canFit(Data::DataType::array1d<int>(2), 1));
    EXPECT_TRUE(twoDView.canFit(Data::DataType::array1d<int>(3), 3));
    EXPECT_FALSE(twoDView.canFit(Data::DataType::array1d<int>(3), 1));
    EXPECT_TRUE(twoDView.canFit(Data::DataType::array2d<int>(1, 2), 1));
    EXPECT_TRUE(twoDView.canFit(Data::DataType::array2d<int>(2, 2), 0));
    EXPECT_FALSE(twoDView.canFit(Data::DataType::array2d<int>(2, 2), 2));
    EXPECT_FALSE(twoDView.canFit(Data::DataType::array2d<double>(1, 2), 0));
}

TEST(DataViewTest, GetArrayAndSubViewPreserveExactElementValuesAcrossRanks)
{
    std::vector<int> oneD{11, 22, 33, 44, 55};
    Data::DataView oneDView(oneD.data(), Data::DataType::array1d<int>(oneD.size()));

    const int* oneDPtr = nullptr;
    ASSERT_NO_THROW(oneDPtr = oneDView.getArray<int>());
    ASSERT_NE(oneDPtr, nullptr);
    EXPECT_EQ(oneDPtr[0], 11);
    EXPECT_EQ(oneDPtr[2], 33);
    EXPECT_EQ(oneDPtr[4], 55);

    Data::DataView oneDSubView;
    ASSERT_NO_THROW(oneDSubView = oneDView.getSubView(Data::DataType::array1d<int>(3), 1));
    const int* oneDSubPtr = nullptr;
    ASSERT_NO_THROW(oneDSubPtr = oneDSubView.getArray<int>());
    ASSERT_NE(oneDSubPtr, nullptr);
    EXPECT_EQ(oneDSubPtr[0], 22);
    EXPECT_EQ(oneDSubPtr[1], 33);
    EXPECT_EQ(oneDSubPtr[2], 44);

    int matrix[3][4] = {{0, 1, 2, 3}, {4, 5, 6, 7}, {8, 9, 10, 11}};
    Data::DataView twoDView(matrix, Data::DataType::array2d<int>(3, 4));

    const int* twoDPtr = nullptr;
    ASSERT_NO_THROW(twoDPtr = twoDView.getArray<int>());
    ASSERT_NE(twoDPtr, nullptr);
    EXPECT_EQ(twoDPtr[0], 0);
    EXPECT_EQ(twoDPtr[1], 1);
    EXPECT_EQ(twoDPtr[4], 4);
    EXPECT_EQ(twoDPtr[7], 7);
    EXPECT_EQ(twoDPtr[11], 11);

    Data::DataView twoDSubView;
    ASSERT_NO_THROW(twoDSubView = twoDView.getSubView(Data::DataType::array2d<int>(2, 3), 5));
    const int* twoDSubPtr = nullptr;
    ASSERT_NO_THROW(twoDSubPtr = twoDSubView.getArray<int>());
    ASSERT_NE(twoDSubPtr, nullptr);
    EXPECT_EQ(twoDSubPtr[0], 5);
    EXPECT_EQ(twoDSubPtr[1], 6);
    EXPECT_EQ(twoDSubPtr[2], 7);
    EXPECT_EQ(twoDSubPtr[3], 9);
    EXPECT_EQ(twoDSubPtr[4], 10);
    EXPECT_EQ(twoDSubPtr[5], 11);

    ASSERT_THROW(twoDView.getSubView(Data::DataType::array2d<int>(2, 3), 10), std::out_of_range);
}

TEST(DataViewTest, BinaryCompatibilityAndTypeMismatchAreRejected)
{
    std::vector<int> ints{10, 20, 30, 40};
    std::vector<double> doubles{1.0, 2.0, 3.0, 4.0};
    Data::DataView intView(ints.data(), Data::DataType::array1d<int>(ints.size()));
    Data::DataView doubleView(doubles.data(), Data::DataType::array1d<double>(doubles.size()));

    EXPECT_TRUE(intView.canFit(Data::DataType::array1d<int>(2), 0));
    EXPECT_TRUE(intView.canFit(Data::DataType::array1d<int>(2), 2));
    EXPECT_FALSE(intView.canFit(Data::DataType::array1d<double>(2), 0));
    EXPECT_FALSE(intView.canFit(Data::DataType::array2d<int>(1, 2), 0));
    EXPECT_FALSE(doubleView.canFit(Data::DataType::array1d<int>(2), 0));

    ASSERT_THROW(intView.getSubView(Data::DataType::array1d<double>(2), 0), std::out_of_range);
    ASSERT_THROW(intView.getSubView(Data::DataType::array2d<int>(1, 2), 0), std::out_of_range);
}


TEST(DataViewTest, ToStringContainsViewAndTypeInformation)
{
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
    Data::DataView view(matrix, Data::DataType::array2d<int>(2, 3));

    const std::string text = view.toString();
    EXPECT_NE(text.find("DataView{"), std::string::npos);
    EXPECT_NE(text.find("DataType{"), std::string::npos);
    EXPECT_NE(text.find("rank=2"), std::string::npos);
    EXPECT_NE(text.find("dimensions=[2, 3]"), std::string::npos);
    EXPECT_NE(text.find("sourceRank=2"), std::string::npos);
    ASSERT_NO_THROW(std::cout<<view<<std::endl);
}

