
#ifndef OUTPUT_INFO_H
#define OUTPUT_INFO_H

#include <vector>
#include <limits>
#include <cmath>
#include <stdexcept>
#include <iostream>
#include <future>

namespace Output
{
    /// Enumeration of possible output types.
    enum class OutputType
    {
        DISCRETE,
        CONTINUOUS
    };

    /**
     * \brief Represents an output with a type and range.
     */
    class Output
    {
        protected: 
            /// Type of the output.
            OutputType type;

            /// For continuous outputs
            /// Minimum value of the output range.
            double rangeMin;
            /// Maximum value of the output range.
            double rangeMax;
            /// Value by default
            double defaultValue;

            /// For discrete outputs
            /// Number of values.
            size_t nbValues;

        public:

            /**
             * \brief Constructor for continuous Output.
             * 
             * \param rangeMin The minimum range value.
             * \param rangeMax The maximum range value.
             * \param defaultValue value by default.
             */
            Output(double rangeMin = -std::numeric_limits<double>::infinity(), double rangeMax = std::numeric_limits<double>::infinity(), double defaultValue = 0) : type{OutputType::CONTINUOUS}, rangeMin{rangeMin}, rangeMax{rangeMax}, defaultValue{defaultValue}, nbValues{0} {
                if (rangeMin >= rangeMax) {
                    throw std::runtime_error("Output::Output: rangeMin must be less than rangeMax.");
                }
                if (defaultValue <= rangeMin || defaultValue >= rangeMax) {
                    throw std::runtime_error("Output::Output: defaultValue out of bounds");
                }
            }

            /**
             * \brief Constructor for discrete Output.
             * 
             * \param nbValues number of discrete output
             */
            explicit Output(size_t nbValues) : type{OutputType::DISCRETE}, rangeMin{-1.0}, rangeMax{1.0}, defaultValue{0}, nbValues{nbValues} {}

            /// \brief Gets the output type.
            OutputType getType() const { return this->type; }
            
            /// \brief Gets the minimum range value.
            double getRangeMin() const { return this->rangeMin; }
            
            /// \brief Gets the maximum range value.
            double getRangeMax() const { return this->rangeMax; }

            /// @brief Gets the number of values of the output 
            size_t getNbValues() const { return this->nbValues; }
    };

    /**
     * \brief Manages a collection of outputs.
     */
    class OutputHandler
    {
        protected:
            /// Vector of outputs.
            std::vector<Output> outputs;

        public:
            /**
             * \brief Constructor for OutputHandler.
             * \param outputs Reference to a vector of outputs.
             */
            OutputHandler(std::vector<Output> outputs): outputs{outputs} {}

            /**
             * \brief Constructor for OutputHandler.
             * \param output output.
             */
            OutputHandler(Output output): outputs{output} {}

            /**
             * \brief Constructor for OutputHandler with a single discrete output.
             * 
             * \param discreteNumber discrete number of outputs.
             */
            OutputHandler(size_t discreteNumber): outputs{Output(discreteNumber)} {}

            /// \brief Adds an output to the collection.
            void addOutput(Output output);

            /// \brief Gets the vector of outputs.
            const std::vector<Output>& getOutputs() const;

            /// @brief get the number of outputs 
            size_t size() const {return this->outputs.size(); }


            /// \brief Gets the vector of discrite outputs.
            std::vector<std::reference_wrapper<const Output>> getDiscreteOutputs() const;

            /// \brief Gets the vector of continuous outputs.
            std::vector<std::reference_wrapper<const Output>> getContinuousOutputs() const;

            /// @brief get the number of discrete outputs 
            size_t sizeDiscrete() const;

            /// @brief get the number of continuous outputs 
            size_t sizeContinuous() const;

            /// @brief get the front output. 
            const Output& front() const;

    };
};

#endif