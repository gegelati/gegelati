#include <gtest/gtest.h>

#include "evaluation/dataset.h"

TEST(DataSetTest, Constructor)
{
    Evaluation::DataSet* dataset1;
    Evaluation::DataSet* dataset2;

    std::vector<Dimensions::Requirement> inputDim;
    inputDim.push_back(Dimensions::Requirement::array1d<int>(1));
    inputDim.push_back(Dimensions::Requirement::array1d<double>(1));
    Dimensions::Requirement outputDim = Dimensions::Requirement::scalar<int>();

    ASSERT_NO_THROW(dataset1 = new Evaluation::DataSet(inputDim, outputDim)) << "Constructor without data failed";

    std::vector<Data::DataValue> inputs;
    inputs.push_back(Data::DataValue::array1d<int[1]>({1}));
    inputs.push_back(Data::DataValue::array1d<double[1]>({1.0}));
    Data::DataValue output = Data::DataValue::scalar<int>(0);

    std::vector<std::pair<std::vector<Data::DataValue>, Data::DataValue>> data;
    data.push_back(std::make_pair(std::move(inputs), std::move(output)));

    ASSERT_NO_THROW(dataset2 = new Evaluation::DataSet(inputDim, outputDim, std::move(data))) << "Constructor without data failed";


    ASSERT_NO_THROW(delete dataset1) << "Destruction failed";
    ASSERT_NO_THROW(delete dataset2) << "Destruction failed";
}

TEST(DataSetTest, addRemoveDataSample)
{
    std::vector<Dimensions::Requirement> inputDim;
    inputDim.push_back(Dimensions::Requirement::array1d<int>(1));
    inputDim.push_back(Dimensions::Requirement::array1d<double>(1));
    Dimensions::Requirement outputDim = Dimensions::Requirement::scalar<int>();

    Evaluation::DataSet dataset(inputDim, outputDim);
    ASSERT_EQ(dataset.size(), 0) << "Size should be 0";

    
    std::vector<Data::DataValue> inputs1;
    inputs1.push_back(Data::DataValue::array1d<int[1]>({1}));
    inputs1.push_back(Data::DataValue::array1d<double[1]>({1.0}));
    Data::DataValue output1 = Data::DataValue::scalar<int>(0);

    ASSERT_NO_THROW(dataset.addData(std::move(inputs1), std::move(output1))) << "Adding correct data should not fail";
    ASSERT_EQ(dataset.size(), 1) << "Size should be 1";

    std::vector<Data::DataValue> inputs2;
    inputs2.push_back(Data::DataValue::array1d<int[1]>({1}));
    Data::DataValue output2 = Data::DataValue::scalar<int>(0);

    ASSERT_THROW(dataset.addData(std::move(inputs2), std::move(output2)), std::runtime_error) << "Adding wrong data should have fail";

    std::vector<Data::DataValue> inputs3;
    inputs3.push_back(Data::DataValue::array1d<double[1]>({1}));
    inputs3.push_back(Data::DataValue::array1d<double[1]>({1.0}));
    Data::DataValue output3 = Data::DataValue::scalar<int>(0);

    ASSERT_THROW(dataset.addData(std::move(inputs3), std::move(output3)), std::runtime_error) << "Adding wrong data should have fail";

    std::vector<Data::DataValue> inputs4;
    inputs4.push_back(Data::DataValue::array1d<int[1]>({1}));
    inputs4.push_back(Data::DataValue::array1d<double[1]>({1.0}));
    Data::DataValue output4 = Data::DataValue::scalar<double>(0.0);

    ASSERT_THROW(dataset.addData(std::move(inputs4), std::move(output4)), std::runtime_error) << "Adding wrong data should have fail";

    ASSERT_THROW(dataset.removeData(3), std::runtime_error) << "Should fail to remove data out of bound";
    ASSERT_NO_THROW(dataset.removeData(0)) << "Removing correct value should not fail";
    ASSERT_EQ(dataset.size(), 0) << "Size should be 0";
}


TEST(DataSetTest, getInputsOutput)
{
    std::vector<Dimensions::Requirement> inputDim;
    inputDim.push_back(Dimensions::Requirement::array1d<int>(2));
    inputDim.push_back(Dimensions::Requirement::array1d<double>(3));
    Dimensions::Requirement outputDim = Dimensions::Requirement::scalar<int>();

    Evaluation::DataSet dataset(inputDim, outputDim);

    std::vector<Data::DataValue> inputs1;
    inputs1.push_back(Data::DataValue::array1d<int[2]>({1, 2}));
    inputs1.push_back(Data::DataValue::array1d<double[3]>({1.0, 2.0, 3.0}));
    dataset.addData(std::move(inputs1), Data::DataValue::scalar<int>(0));
    
    std::vector<Data::DataValue> inputs2;
    inputs2.push_back(Data::DataValue::array1d<int[2]>({3, 4}));
    inputs2.push_back(Data::DataValue::array1d<double[3]>({4.0, 5.0, 6.0}));
    dataset.addData(std::move(inputs2), Data::DataValue::scalar<int>(1));
    
    std::vector<Data::DataValue> inputs3;
    inputs3.push_back(Data::DataValue::array1d<int[2]>({5, 6}));
    inputs3.push_back(Data::DataValue::array1d<double[3]>({7.0, 8.0, 9.0}));
    dataset.addData(std::move(inputs3), Data::DataValue::scalar<int>(2));

    ASSERT_EQ(dataset.size(), 3) << "Size should be 3";

    ASSERT_NO_THROW(dataset.getInputsAt(0)) << "Getting value should not fail with good index";
    ASSERT_NO_THROW(dataset.getInputsAt(1)) << "Getting value should not fail with good index";
    ASSERT_NO_THROW(dataset.getInputsAt(2)) << "Getting value should not fail with good index";
    ASSERT_THROW(dataset.getInputsAt(3), std::runtime_error) << "Getting value should fail with wrong index";

    ASSERT_NO_THROW(dataset.getOutputAt(0)) << "Getting value should not fail with good index";
    ASSERT_NO_THROW(dataset.getOutputAt(1)) << "Getting value should not fail with good index";
    ASSERT_NO_THROW(dataset.getOutputAt(2)) << "Getting value should not fail with good index";
    ASSERT_THROW(dataset.getOutputAt(3), std::runtime_error) << "Getting value should fail with wrong index";

    const std::vector<Data::DataValue>& values = dataset.getInputsAt(1);
    std::shared_ptr<const int[]> values0 = values.at(0).getData<int>();
    std::shared_ptr<const double[]> values1 = values.at(1).getData<double>();
    ASSERT_EQ(values0[1], 4) << "Value should be equal";
    ASSERT_EQ(values1[1], 5.0) << "Value should be equal";

    const Data::DataValue& output = dataset.getOutputAt(2);
    ASSERT_EQ(2, output.getScalar<int>());
}
