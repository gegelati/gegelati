#ifndef DATA_SET_EXAMPLES_H
#define DATA_SET_EXAMPLES_H

#include "evaluation/dataset.h"

static Evaluation::DataSet createSmallDataSet() {
    std::vector<Dimensions::Requirement> inputDim;
    inputDim.push_back(Dimensions::Requirement::array1d<int>(2));
    Evaluation::DataSet dataSet(inputDim, Dimensions::Requirement::scalar<int>());

    
    std::vector<Data::DataValue> inputs1;
    inputs1.push_back(Data::DataValue::array1d<int[2]>({1, 2}));
    dataSet.addData(std::move(inputs1), Data::DataValue::scalar<int>(0));
    
    std::vector<Data::DataValue> inputs2;
    inputs2.push_back(Data::DataValue::array1d<int[2]>({3, 4}));
    dataSet.addData(std::move(inputs2), Data::DataValue::scalar<int>(1));
    
    std::vector<Data::DataValue> inputs3;
    inputs3.push_back(Data::DataValue::array1d<int[2]>({5, 6}));
    dataSet.addData(std::move(inputs3), Data::DataValue::scalar<int>(2));

    return std::move(dataSet);
}

#endif