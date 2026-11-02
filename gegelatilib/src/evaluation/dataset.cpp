#include "evaluation/dataset.h"


const std::vector<Dimensions::Requirement>& Evaluation::DataSet::getInputDimensions() const
{
    return this->inputDimensions;
}


const Dimensions::Requirement& Evaluation::DataSet::getOutputDimension() const
{
    return this->outputDimension;
}



void Evaluation::DataSet::addData(std::vector<Data::DataValue> inputs, Data::DataValue output, bool training)
{
    if(!Dimensions::Requirement::acceptListValues(this->inputDimensions, inputs)) {
        throw std::runtime_error("Representations::Representation::execute: Dimensions of the inputs are wrong");
    }
    if(!this->outputDimension.accepts(output)) {
        throw std::runtime_error("Evaluation::DataSet::addDataSample: output not valid for the dataset's requirements");
    } 

    if(training) {
      this->trainingData.push_back(std::make_pair(std::move(inputs), std::move(output)));
    } else {
      this->validationgData.push_back(std::make_pair(std::move(inputs), std::move(output)));
    }
}

void Evaluation::DataSet::removeData(size_t index)
{
  if(index >= this->size()) {
    throw std::runtime_error("Evaluation::DataSet::removeData: index out of dataset range");
  }
  if(index >= this->sizeTraining()) {
    this->validationgData.erase(this->validationgData.begin() + index - this->sizeTraining());
  }
  this->trainingData.erase(this->trainingData.begin() + index);
}

void Evaluation::DataSet::setOutputAt(size_t index, Data::DataValue output)
{
  if(index >= this->size()) {
    throw std::runtime_error("Evaluation::DataSet::removeData: index out of dataset range");
  }
  if(index >= this->sizeTraining()) {
    this->validationgData.at(index - this->sizeTraining()).second = std::move(output);
  }
  this->trainingData.at(index).second = std::move(output);
}

size_t Evaluation::DataSet::size() const
{
    return this->trainingData.size() + this->validationgData.size();
}
size_t Evaluation::DataSet::sizeTraining() const
{
    return this->trainingData.size();
}
size_t Evaluation::DataSet::sizeValidation() const
{
    return this->validationgData.size();
}

const std::vector<Data::DataValue>& Evaluation::DataSet::getInputsAt(size_t index) const
{
  if(index >= this->size()) {
    throw std::runtime_error("Evaluation::DataSet::getInputsAt: index out of dataset range");
  }
  if(index >= this->sizeTraining()) {
    return this->validationgData.at(index - this->sizeTraining()).first;
  }
  return this->trainingData.at(index).first;
}

const Data::DataValue& Evaluation::DataSet::getOutputAt(size_t index) const
{
  if(index >= this->size()) {
    throw std::runtime_error("Evaluation::DataSet::getOutputAt: index out of dataset range");
  }
  if(index >= this->sizeTraining()) {
    return this->validationgData.at(index - this->sizeTraining()).second;
  }
  return this->trainingData.at(index).second;
}

std::string Evaluation::DataSet::toString(size_t end, size_t start) const
{
    end = std::min(end, this->size());
    std::stringstream ss;

    ss << "DataSet with current " << this->sizeTraining() << " training values and " << this->sizeValidation() << " validation values. Displaying values " << start << " to " << end << ":\n";
    for(size_t idx = start; idx < end ; idx++) {
      ss << " - Value " << idx << ":\n";
      const std::pair<std::vector<Data::DataValue>, Data::DataValue>* pair = &this->trainingData.at(idx);
      if(idx > this->sizeTraining()) {
        pair = &this->validationgData.at(idx - this->sizeTraining());
      }
      for(size_t idxInput = 0; idxInput < pair->first.size(); idxInput++) {
        ss << "    Input " << idxInput << ": " << pair->first.at(idxInput).toShortString() << "\n";
      }
      ss << "     --> Target Output: " << pair->second.toShortString() << "\n";
    }
    return ss.str();
}