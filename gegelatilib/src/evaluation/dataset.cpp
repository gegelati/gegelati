#include "evaluation/dataset.h"


const std::vector<Dimensions::Requirement>& Evaluation::DataSet::getInputDimensions() const
{
    return this->inputDimensions;
}


const Dimensions::Requirement& Evaluation::DataSet::getOutputDimension() const
{
    return this->outputDimension;
}

void Evaluation::DataSet::addData(std::vector<Data::DataValue> inputs, Data::DataValue output)
{
    if(inputs.size() != this->inputDimensions.size()) {
        throw std::runtime_error("Evaluation::DataSet::addDataSample: size of input vector is wrong.");
    } 
    for(size_t idx = 0; idx < inputs.size(); idx++) {
        if(!this->inputDimensions.at(idx).accepts(inputs.at(idx))) {
            throw std::runtime_error("Evaluation::DataSet::addDataSample: input not valid for the dataset's requirements");
        }
    }
    if(!this->outputDimension.accepts(output)) {
        throw std::runtime_error("Evaluation::DataSet::addDataSample: output not valid for the dataset's requirements");
    } 

    this->data.push_back(std::make_pair(std::move(inputs), std::move(output)));
}

void Evaluation::DataSet::removeData(size_t index)
{
  if(index >= this->data.size()) {
    throw std::runtime_error("Evaluation::DataSet::removeData: index out of dataset range");
  }
  this->data.erase(this->data.begin() + index);
}

size_t Evaluation::DataSet::size() const
{
    return this->data.size();
}

const std::vector<Data::DataValue>& Evaluation::DataSet::getInputsAt(size_t index) const
{
  if(index >= this->data.size()) {
    throw std::runtime_error("Evaluation::DataSet::getInputsAt: index out of dataset range");
  }
  return this->data.at(index).first;
}

const Data::DataValue& Evaluation::DataSet::getOutputAt(size_t index) const
{
  if(index >= this->data.size()) {
    throw std::runtime_error("Evaluation::DataSet::getOutputAt: index out of dataset range");
  }
  return this->data.at(index).second;
}