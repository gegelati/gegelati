
#include <gtest/gtest.h>

#include <vector>

#include "data/dataValue.h"
#include "dimensions/numericRange.h"

TEST(DataValueTest, factoryConstructionCreatesScalarAndArrayValues)
{
    ASSERT_NO_THROW(Data::DataValue::scalar(42.5));
    auto scalarValue = Data::DataValue::scalar(42.5);
    EXPECT_TRUE(scalarValue);
    EXPECT_EQ(scalarValue.getType().rank, 0u);
    EXPECT_EQ(scalarValue.getType().dimensions[0], 1u);
    EXPECT_DOUBLE_EQ(scalarValue.getScalar<double>(), 42.5);

    std::vector<int> oneD = {10, 20, 30, 40};
    ASSERT_NO_THROW(Data::DataValue::array1d(oneD));
    auto oneDValue = Data::DataValue::array1d(oneD);
    EXPECT_EQ(oneDValue.getType().rank, 1u);
    EXPECT_EQ(oneDValue.getType().dimensions[0], 4u);
    std::shared_ptr<const int[]> oneDPtr = oneDValue.getData<int>();
    ASSERT_NE(oneDPtr, nullptr);
    EXPECT_EQ(oneDPtr[0], 10);
    EXPECT_EQ(oneDPtr[1], 20);
    EXPECT_EQ(oneDPtr[2], 30);
    EXPECT_EQ(oneDPtr[3], 40);

    std::vector<std::vector<int>> rows = {{1, 2, 3}, {4, 5, 6}};
    ASSERT_NO_THROW(Data::DataValue::array2d(rows));
    auto twoDValue = Data::DataValue::array2d(rows);
    EXPECT_EQ(twoDValue.getType().rank, 2u);
    EXPECT_EQ(twoDValue.getType().dimensions[0], 2u);
    EXPECT_EQ(twoDValue.getType().dimensions[1], 3u);
    std::shared_ptr<const int[]> twoDPtr = twoDValue.getData<int>();
    ASSERT_NE(twoDPtr, nullptr);
    EXPECT_EQ(twoDPtr[0], 1);
    EXPECT_EQ(twoDPtr[1], 2);
    EXPECT_EQ(twoDPtr[2], 3);
    EXPECT_EQ(twoDPtr[3], 4);
    EXPECT_EQ(twoDPtr[4], 5);
    EXPECT_EQ(twoDPtr[5], 6);
}

TEST(DataValueTest, equality) {
    
    auto scalarValue1 = Data::DataValue::scalar(42.5);
    auto oneDValue1 = Data::DataValue::array1d<int[5]>({0, 1, 2, 3, 4});
    auto twoDValue1 = Data::DataValue::array2d<int[2][3]>({{1, 2, 3}, {4, 5, 6}});

    auto scalarValue2 = Data::DataValue::scalar(42.5);
    auto oneDValue2 = Data::DataValue::array1d<int[5]>({0, 1, 2, 3, 4});
    auto twoDValue2 = Data::DataValue::array2d<int[2][3]>({{1, 2, 3}, {4, 5, 6}});

    ASSERT_TRUE(scalarValue1 == scalarValue2) << "Values should be equal";
    ASSERT_TRUE(oneDValue1 == oneDValue2) << "Values should be equal";
    ASSERT_TRUE(twoDValue1 == twoDValue2) << "Values should be equal";


    auto scalarValue3 = Data::DataValue::scalar(42);
    auto scalarValue4 = Data::DataValue::scalar(42.6);
    ASSERT_TRUE(scalarValue1 != oneDValue1) << "Value should not be equal";
    ASSERT_TRUE(scalarValue1 != scalarValue3) << "Value should not be equal";
    ASSERT_TRUE(scalarValue1 != scalarValue3) << "Value should not be equal";

    auto oneDValue3 = Data::DataValue::array1d<size_t[5]>({0, 1, 2, 3, 4});
    auto oneDValue4 = Data::DataValue::array1d<int[4]>({0, 1, 2, 3});
    auto oneDValue5 = Data::DataValue::array1d<int[5]>({0, 1, 2, 3, 6});
    ASSERT_TRUE(oneDValue1 != scalarValue1) << "Value should not be equal";
    ASSERT_TRUE(oneDValue1 != oneDValue3) << "Value should not be equal";
    ASSERT_TRUE(oneDValue1 != oneDValue4) << "Value should not be equal";
    ASSERT_TRUE(oneDValue1 != oneDValue5) << "Value should not be equal";
    
    auto twoDValue3 = Data::DataValue::array2d<size_t[2][3]>({{1, 2, 3}, {4, 5, 6}});
    auto twoDValue4 = Data::DataValue::array2d<int[2][2]>({{1, 2}, {4, 5}});
    auto twoDValue5 = Data::DataValue::array2d<int[1][3]>({{1, 2, 3}});
    auto twoDValue6 = Data::DataValue::array2d<int[2][3]>({{1, 2, 3}, {4, 5, 7}});
    ASSERT_TRUE(twoDValue1 != scalarValue1) << "Value should not be equal";
    ASSERT_TRUE(twoDValue1 != twoDValue3) << "Value should not be equal";
    ASSERT_TRUE(twoDValue1 != twoDValue4) << "Value should not be equal";
    ASSERT_TRUE(twoDValue1 != twoDValue5) << "Value should not be equal";
    ASSERT_TRUE(twoDValue1 != twoDValue6) << "Value should not be equal";
}

TEST(DataValueTest, clone)
{   
    auto scalarValue = Data::DataValue::scalar(42.5);
    auto oneDValue = Data::DataValue::array1d<int[5]>({0, 1, 2, 3, 4});
    auto twoDValue = Data::DataValue::array2d<int[2][3]>({{1, 2, 3}, {4, 5, 6}});

    auto scalarValueCopy = scalarValue.clone();
    auto oneDValueCopy = oneDValue.clone();
    auto twoDValueCopy = twoDValue.clone();
    ASSERT_TRUE(scalarValue == scalarValueCopy)<< "cloned value should be equal";
    ASSERT_TRUE(oneDValue == oneDValueCopy)<< "cloned value should be equal";
    ASSERT_TRUE(twoDValue == twoDValueCopy)<< "cloned value should be equal";
}

TEST(DataValueTest, zerosInitialisesEverySupportedRank)
{
    ASSERT_NO_THROW(Data::DataValue::zeros<double>(Data::DataType::scalar<double>()));
    auto scalarZero = Data::DataValue::zeros<double>(Data::DataType::scalar<double>());
    EXPECT_DOUBLE_EQ(scalarZero.getScalar<double>(), 0.0);

    auto oneDZero = Data::DataValue::zeros<int>(Data::DataType::array1d<int>(4));
    std::shared_ptr<const int[]> oneDZeroPtr = oneDZero.getData<int>();
    ASSERT_NE(oneDZeroPtr, nullptr);
    EXPECT_EQ(oneDZeroPtr[0], 0);
    EXPECT_EQ(oneDZeroPtr[1], 0);
    EXPECT_EQ(oneDZeroPtr[2], 0);
    EXPECT_EQ(oneDZeroPtr[3], 0);

    auto twoDZero = Data::DataValue::zeros<int>(Data::DataType::array2d<int>(2, 3));
    std::shared_ptr<const int[]> twoDZeroPtr = twoDZero.getData<int>();
    ASSERT_NE(twoDZeroPtr, nullptr);
    EXPECT_EQ(twoDZeroPtr[0], 0);
    EXPECT_EQ(twoDZeroPtr[1], 0);
    EXPECT_EQ(twoDZeroPtr[2], 0);
    EXPECT_EQ(twoDZeroPtr[3], 0);
    EXPECT_EQ(twoDZeroPtr[4], 0);
    EXPECT_EQ(twoDZeroPtr[5], 0);

    Data::DataType unsupported;
    unsupported.rank = 3;
    ASSERT_THROW(Data::DataValue::zeros<int>(unsupported), std::invalid_argument);
}

TEST(DataValueTest, convertNumericValue)
{
    Data::DataValue scalar = Data::DataValue::scalar<int>(42);
    auto convertedScalar = Data::DataValue::convertNumericValue<int, double>(scalar);
    EXPECT_EQ(convertedScalar.getType().rank, 0u);
    EXPECT_DOUBLE_EQ(convertedScalar.getScalar<double>(), 42.0);

    auto oneD = Data::DataValue::array1d(std::vector<int>{1, 2, 3});
    auto convertedOneD = Data::DataValue::convertNumericValue<int, float>(oneD);
    EXPECT_EQ(convertedOneD.getType().rank, 1u);
    EXPECT_EQ(convertedOneD.getType().dimensions[0], 3u);
    std::shared_ptr<const float[]> oneDValues = convertedOneD.getData<float>();
    EXPECT_FLOAT_EQ(oneDValues[0], 1.0f);
    EXPECT_FLOAT_EQ(oneDValues[1], 2.0f);
    EXPECT_FLOAT_EQ(oneDValues[2], 3.0f);

    Data::DataValue twoD = Data::DataValue::array2d(std::vector<std::vector<double>>{{1.5, 2.5}, {3.5, 4.5}});
    auto convertedTwoD = Data::DataValue::convertNumericValue<double, int>(twoD);
    EXPECT_EQ(convertedTwoD.getType().rank, 2u);
    EXPECT_EQ(convertedTwoD.getType().dimensions[0], 2u);
    EXPECT_EQ(convertedTwoD.getType().dimensions[1], 2u);
    std::shared_ptr<const int[]> twoDValues = convertedTwoD.getData<int>();
    EXPECT_EQ(twoDValues[0], 1);
    EXPECT_EQ(twoDValues[1], 2);
    EXPECT_EQ(twoDValues[2], 3);
    EXPECT_EQ(twoDValues[3], 4);

    auto text = Data::DataValue::scalar(std::string("42"));
    ASSERT_THROW((Data::DataValue::convertNumericValue<double, int>(text)), std::runtime_error);
    ASSERT_THROW((Data::DataValue::convertNumericValue<int, double>(twoD)), std::runtime_error);
}

TEST(DataValueTest, getSubValueCoversAllRankCombinations)
{
    auto scalarTarget = Data::DataValue::scalar(10.0);
    ASSERT_NO_THROW(scalarTarget.getSubValue<double>(Data::DataType::scalar<double>(), 0));
    auto scalarCopy = scalarTarget.getSubValue<double>(Data::DataType::scalar<double>(), 0);
    EXPECT_DOUBLE_EQ(scalarCopy.getScalar<double>(), 10.0);

    auto oneDTarget = Data::DataValue::array1d(std::vector<int>{10, 20, 30, 40, 50});
    ASSERT_NO_THROW(oneDTarget.getSubValue<int>(Data::DataType::scalar<int>(), 2));
    auto scalarFrom1D = oneDTarget.getSubValue<int>(Data::DataType::scalar<int>(), 2);
    EXPECT_EQ(scalarFrom1D.getScalar<int>(), 30);

    ASSERT_NO_THROW(oneDTarget.getSubValue<int>(Data::DataType::array1d<int>(3), 1));
    auto arrayFrom1D = oneDTarget.getSubValue<int>(Data::DataType::array1d<int>(3), 1);
    std::shared_ptr<const int[]> fromOneDPtr = arrayFrom1D.getData<int>();
    ASSERT_NE(fromOneDPtr, nullptr);
    EXPECT_EQ(fromOneDPtr[0], 20);
    EXPECT_EQ(fromOneDPtr[1], 30);
    EXPECT_EQ(fromOneDPtr[2], 40);

    auto twoDTarget = Data::DataValue::array2d(std::vector<std::vector<int>>{{0, 1, 2}, {3, 4, 5}, {6, 7, 8}});
    ASSERT_NO_THROW(twoDTarget.getSubValue<int>(Data::DataType::scalar<int>(), 4));
    auto scalarFrom2D = twoDTarget.getSubValue<int>(Data::DataType::scalar<int>(), 4);
    EXPECT_EQ(scalarFrom2D.getScalar<int>(), 4);

    ASSERT_NO_THROW(twoDTarget.getSubValue<int>(Data::DataType::array1d<int>(3), 3));
    auto array1DFrom2D = twoDTarget.getSubValue<int>(Data::DataType::array1d<int>(3), 3);
    std::shared_ptr<const int[]> from2D1D = array1DFrom2D.getData<int>();
    ASSERT_NE(from2D1D, nullptr);
    EXPECT_EQ(from2D1D[0], 3);
    EXPECT_EQ(from2D1D[1], 4);
    EXPECT_EQ(from2D1D[2], 5);

    ASSERT_NO_THROW(twoDTarget.getSubValue<int>(Data::DataType::array2d<int>(2, 2), 1));
    auto array2DFrom2D = twoDTarget.getSubValue<int>(Data::DataType::array2d<int>(2, 2), 1);
    std::shared_ptr<const int[]> from2D2D = array2DFrom2D.getData<int>();
    ASSERT_NE(from2D2D, nullptr);
    EXPECT_EQ(from2D2D[0], 1);
    EXPECT_EQ(from2D2D[1], 2);
    EXPECT_EQ(from2D2D[2], 4);
    EXPECT_EQ(from2D2D[3], 5);

    ASSERT_THROW(scalarTarget.getSubValue<double>(Data::DataType::array1d<double>(2), 0), std::out_of_range);
    ASSERT_THROW(oneDTarget.getSubValue<int>(Data::DataType::array1d<int>(3), 3), std::out_of_range);
    ASSERT_THROW(twoDTarget.getSubValue<int>(Data::DataType::array2d<int>(2, 2), 7), std::out_of_range);
    ASSERT_THROW(twoDTarget.getSubValue<int>(Data::DataType::array1d<int>(3), 1), std::out_of_range);
    ASSERT_THROW(twoDTarget.getSubValue<float>(Data::DataType::array2d<float>(2, 2), 1), std::runtime_error);
}

TEST(DataValueTest, setSubValue)
{
    auto scalarTarget = Data::DataValue::scalar(0.0);
    auto scalarSource = Data::DataValue::scalar(7.5);
    ASSERT_NO_THROW(scalarTarget.setSubValue(scalarSource, 0));
    EXPECT_DOUBLE_EQ(scalarTarget.getScalar<double>(), 7.5);

    auto oneDTarget = Data::DataValue::zeros<int>(Data::DataType::array1d<int>(4));
    auto oneDSource = Data::DataValue::array1d(std::vector<int>{1, 2, 3});
    auto scalarSourceInt = Data::DataValue::scalar<int>(7);
    ASSERT_NO_THROW(oneDTarget.setSubValue(oneDSource, 1));
    std::shared_ptr<const int[]> oneDPtr = oneDTarget.getData<int>();
    EXPECT_EQ(oneDPtr[0], 0);
    EXPECT_EQ(oneDPtr[1], 1);
    EXPECT_EQ(oneDPtr[2], 2);
    EXPECT_EQ(oneDPtr[3], 3);


    ASSERT_NO_THROW(oneDTarget.setSubValue(scalarSourceInt, 2));
    ASSERT_EQ(oneDTarget.getScalarAt<int>(2), 7);

    auto twoDTarget = Data::DataValue::zeros<int>(3, 3);
    auto twoDSource = Data::DataValue::array2d(std::vector<std::vector<int>>{{9, 8}, {7, 6}});
    ASSERT_NO_THROW(twoDTarget.setSubValue(twoDSource, 1));
    std::shared_ptr<const int[]> twoDPtr = twoDTarget.getData<int>();
    EXPECT_EQ(twoDPtr[0], 0);
    EXPECT_EQ(twoDPtr[1], 9);
    EXPECT_EQ(twoDPtr[2], 8);
    EXPECT_EQ(twoDPtr[3], 0);
    EXPECT_EQ(twoDPtr[4], 7);
    EXPECT_EQ(twoDPtr[5], 6);
    EXPECT_EQ(twoDPtr[6], 0);
    EXPECT_EQ(twoDPtr[7], 0);
    EXPECT_EQ(twoDPtr[8], 0);

    ASSERT_NO_THROW(twoDTarget.setSubValue(oneDSource, 3));
    std::shared_ptr<const int[]> twoDPtr2 = twoDTarget.getData<int>();
    EXPECT_EQ(twoDPtr2[0], 0);
    EXPECT_EQ(twoDPtr2[1], 9);
    EXPECT_EQ(twoDPtr2[2], 8);
    EXPECT_EQ(twoDPtr2[3], 1);
    EXPECT_EQ(twoDPtr2[4], 2);
    EXPECT_EQ(twoDPtr2[5], 3);
    EXPECT_EQ(twoDPtr2[6], 0);
    EXPECT_EQ(twoDPtr2[7], 0);
    EXPECT_EQ(twoDPtr2[8], 0);



    auto target2D = Data::DataValue::zeros<double>(3, 3);
    auto scalarInsert = Data::DataValue::scalar(5.5);
    ASSERT_NO_THROW(target2D.setSubValue(scalarInsert, 4));
    EXPECT_DOUBLE_EQ(target2D.getScalarAt<double>(4), 5.5);

    ASSERT_THROW(oneDTarget.setSubValue(oneDSource, 2), std::out_of_range);
    ASSERT_THROW(twoDTarget.setSubValue(twoDSource, 7), std::out_of_range);
    ASSERT_THROW(twoDTarget.setSubValue(oneDSource, 1), std::out_of_range);
    ASSERT_THROW(target2D.setSubValue(Data::DataValue::scalar(1.0), 9), std::out_of_range);
    ASSERT_THROW(target2D.setSubValue(Data::DataValue::scalar(1), 0), std::runtime_error);
}

TEST(DataValueTest, setScalarAt) 
{
    auto scalarTarget = Data::DataValue::zeros<int>(0);
    auto oneDTarget = Data::DataValue::zeros<int>(4);
    auto twoDTarget = Data::DataValue::zeros<int>(3, 3);
    
    ASSERT_NO_THROW(scalarTarget.setScalarAt<int>(8, 0)) << "Setting value failed";
    ASSERT_NO_THROW(oneDTarget.setScalarAt<int>(8, 2)) << "Setting value failed";
    ASSERT_NO_THROW(twoDTarget.setScalarAt<int>(8, 5)) << "Setting value failed";
    
    ASSERT_EQ(scalarTarget.getScalarAt<int>(0), 8) << "Value is not rightfully set";
    ASSERT_EQ(oneDTarget.getScalarAt<int>(2), 8) << "Value is not rightfully set";
    ASSERT_EQ(twoDTarget.getScalarAt<int>(5), 8) << "Value is not rightfully set";

    ASSERT_THROW(twoDTarget.setScalarAt<int>(1, 10), std::out_of_range) << "Should have throw with out of bounds";
    ASSERT_THROW(twoDTarget.setScalarAt<double>(1, 1), std::out_of_range) << "Should have throw with wrong type";
    
}

TEST(DataValueTest, arrayRangeFactoriesAndStringOutputRemainReliable)
{
    std::vector<int> flat = {1, 2, 3, 4, 5};
    ASSERT_NO_THROW(Data::DataValue::array2d(flat, 1, 5));
    auto validFlat2D = Data::DataValue::array2d(flat, 1, 5);
    EXPECT_EQ(validFlat2D.getType().dimensions[0], 1u);
    EXPECT_EQ(validFlat2D.getType().dimensions[1], 5u);

    std::vector<std::vector<int>> invalidRows = {{1, 2}, {3}};
    ASSERT_THROW(Data::DataValue::array2d(invalidRows), std::invalid_argument);
    ASSERT_THROW(Data::DataValue::array2d(flat, 2, 6), std::invalid_argument);
    ASSERT_THROW(Data::DataValue::array2d(std::vector<std::vector<int>>{}), std::invalid_argument);

    auto value = Data::DataValue::array2d(std::vector<int>{1, 2, 3, 4}, 2, 2);
    ASSERT_NO_THROW(value.toString());
    const std::string text = value.toString();
    EXPECT_NE(text.find("DataValue{"), std::string::npos);
    EXPECT_NE(text.find("DataType{"), std::string::npos);
    EXPECT_NE(text.find("rank=2"), std::string::npos);
    EXPECT_NE(text.find("[1, 2"), std::string::npos);

    ASSERT_NO_THROW(std::cout<<value<<std::endl);
    
}



TEST(DataValueTest, DataViewCreation)
{
    Data::DataValue value = Data::DataValue::array1d<int[3]>({1, 2, 3});
    Data::DataView view = value.view();
    ASSERT_TRUE(view.getType() == value.getType());

    std::shared_ptr<const int[]> data = view.getData<int>();
    ASSERT_EQ(data[0], 1);
    ASSERT_EQ(data[1], 2);
    ASSERT_EQ(data[2], 3);

    value.setSubValue(Data::DataValue::scalar<int>(4), 0);
    ASSERT_EQ(data[0], 4);
    ASSERT_EQ(data[1], 2);
    ASSERT_EQ(data[2], 3);
}