#ifndef DATASET_H
#define DATASET_H


#include "data/dataValue.h"
#include "dimensions/requirement.h"

namespace Evaluation {
    
    /**
     * Class holding the data notably used by the predictionProblem class.
     * 
     * The dataSet stores a vector of DataValue as input, and a DataValue as output.
     */
    class DataSet {

        protected:
            /// @brief Storage of the data
            std::vector<std::pair<std::vector<Data::DataValue>, Data::DataValue>> data;

            /// Input dimensions
            std::vector<Dimensions::Requirement> inputDimensions;

            /// Output dimension
            Dimensions::Requirement outputDimension;

        public: 
            /**
             * \brief Constructor for empty DataSet.
             *
             * \param[in] inputDimensions the dimensions of the input sources.
             * \param[in] outputDimension the dimensions of the output source.
             */
            DataSet(const std::vector<Dimensions::Requirement>& inputDimensions, const Dimensions::Requirement& outputDimension)
                : inputDimensions(inputDimensions), outputDimension(outputDimension) {};

            /**
             * \brief Constructor for DataSet with defined data.
             *
             * \param[in] inputDimensions the dimensions of the input sources.
             * \param[in] outputDimension the dimensions of the output source.
             * \param[in] data predifined data
             */
            DataSet(const std::vector<Dimensions::Requirement>& inputDimensions, const Dimensions::Requirement& outputDimension,
                    std::vector<std::pair<std::vector<Data::DataValue>, Data::DataValue>> data)
                : inputDimensions(inputDimensions), outputDimension(outputDimension) {
                    // Add data
                    for(auto& [inputs, output]: data) {
                        this->addData(std::move(inputs), std::move(output));
                    }
                };

            /**
             * \brief get the input dimensions of the Problem.
             */
            virtual const std::vector<Dimensions::Requirement>& getInputDimensions() const;

            /**
             * \brief get the output dimension of the Problem.
             */
            virtual const Dimensions::Requirement& getOutputDimension() const;

            /**
             * \brief Add a sample to the dataset.
             * 
             * \param[in] inputs the inputs added.
             * \param[in] output the corresponding output
             * 
             * \throw if either the inputs or output are not compatible with the dimensions.
             */
            virtual void addData(std::vector<Data::DataValue> inputs, Data::DataValue output);

            /**
             * \brief Remove the data at specified index.
             */
            virtual void removeData(size_t index);

            /**
             * \brief return the size of the dataset
             */
            virtual size_t size() const;

            /**
             * \brief Return the inputs at the specified index of the dataset.
             */
            virtual const std::vector<Data::DataValue>& getInputsAt(size_t index) const;

            /**
             * \brief Return the output at the specified index of the dataset.
             */
            virtual const Data::DataValue& getOutputAt(size_t index) const;
    };
};

#endif