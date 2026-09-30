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
    if(!Dimensions::Requirement::acceptListValues(this->inputDimensions, inputs)) {
        throw std::runtime_error("Representations::Representation::execute: Dimensions of the inputs are wrong");
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

std::string Evaluation::DataSet::toString(size_t end, size_t start) const
{
    end = std::min(end, this->data.size());
    std::stringstream ss;

    ss << "DataSet with current " << this->data.size() << " values. Displaying values " << start << " to " << end << ":\n";
    for(size_t idx = start; idx < end ; idx++) {
      ss << " - Value " << idx << ":\n";
      const std::pair<std::vector<Data::DataValue>, Data::DataValue>& pair = this->data.at(idx);
      for(size_t idxInput = 0; idxInput < pair.first.size(); idxInput++) {
        ss << "    Input " << idxInput << ": " << pair.first.at(idxInput).toShortString() << "\n";
      }
      ss << "     --> Target Output: " << pair.second.toShortString() << "\n";
    }
    return ss.str();
}